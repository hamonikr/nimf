/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-client.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015,2016 Hodong Kim <cogniti@gmail.com>
 *
 * Nimf is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nimf is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program;  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nimf-client.h"
#include "nimf-im.h"
#include "nimf-agent.h"
#include "nimf-marshalers.h"
#include "nimf-enum-types.h"
#include <gio/gunixsocketaddress.h>

enum
{
  PROP_0,
  PROP_CONNECTION_TYPE,
};

enum {
  ENGINE_CHANGED,
  DISCONNECTED,
  LAST_SIGNAL
};

static guint  nimf_client_signals[LAST_SIGNAL] = { 0 };
GMainContext *nimf_client_sockets_context = NULL;
static guint  nimf_client_sockets_context_ref_count = 0;

G_DEFINE_ABSTRACT_TYPE (NimfClient, nimf_client, G_TYPE_OBJECT);

static gboolean
on_incoming_message (GSocket      *socket,
                     GIOCondition  condition,
                     gpointer      user_data)
{
  g_debug (G_STRLOC ": %s: socket fd:%d", G_STRFUNC, g_socket_get_fd (socket));

  NimfClient *client = NIMF_CLIENT (user_data);
  nimf_message_unref (client->result->reply);
  client->result->is_dispatched = TRUE;

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    /* Because two GSource is created over one socket,
     * when G_IO_HUP | G_IO_ERR, callback can run two times.
     * the following code avoid that callback runs two times. */
    GSource *source = g_main_current_source ();

    if (source == client->default_context_source)
      g_source_destroy (client->sockets_context_source);
    else if (source == client->sockets_context_source)
      g_source_destroy (client->default_context_source);

    if (!g_socket_is_closed (socket))
      g_socket_close (socket, NULL);

    client->result->reply    = NULL;
    g_signal_emit_by_name (client, "disconnected", NULL);

    g_critical (G_STRLOC ": %s: G_IO_HUP | G_IO_ERR", G_STRFUNC);

    return G_SOURCE_REMOVE;
  }

  NimfMessage *message;
  message = nimf_recv_message (socket);
  client->result->reply = message;
  gboolean retval;

  if (G_UNLIKELY (message == NULL))
  {
    g_critical (G_STRLOC ": NULL message");
    return G_SOURCE_CONTINUE;
  }

  switch (message->header->type)
  {
    /* reply */
    case NIMF_MESSAGE_FILTER_EVENT_REPLY:
    case NIMF_MESSAGE_RESET_REPLY:
    case NIMF_MESSAGE_FOCUS_IN_REPLY:
    case NIMF_MESSAGE_FOCUS_OUT_REPLY:
    case NIMF_MESSAGE_SET_SURROUNDING_REPLY:
    case NIMF_MESSAGE_GET_SURROUNDING_REPLY:
    case NIMF_MESSAGE_SET_CURSOR_LOCATION_REPLY:
    case NIMF_MESSAGE_SET_USE_PREEDIT_REPLY:
      break;
    /* signals */
    case NIMF_MESSAGE_PREEDIT_START:
      g_signal_emit_by_name (NIMF_IM (client), "preedit-start");
      nimf_send_message (socket, NIMF_MESSAGE_PREEDIT_START_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_PREEDIT_END:
      g_signal_emit_by_name (NIMF_IM (client), "preedit-end");
      nimf_send_message (socket, NIMF_MESSAGE_PREEDIT_END_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_PREEDIT_CHANGED:
      {
        NimfIM *im = NIMF_IM (client);
        g_free (im->preedit_string);
        im->preedit_string = g_strndup (message->data,
                                        message->header->data_len - 1 - sizeof (gint));
        im->cursor_pos = *(gint *) (message->data +
                                    message->header->data_len - sizeof (gint));
        g_signal_emit_by_name (im, "preedit-changed");
        nimf_send_message (socket, NIMF_MESSAGE_PREEDIT_CHANGED_REPLY,
                           NULL, 0, NULL);
      }
      break;
    case NIMF_MESSAGE_COMMIT:
      nimf_message_ref (message);
      g_signal_emit_by_name (NIMF_IM (client), "commit", (const gchar *) message->data);
      nimf_message_unref (message);
      nimf_send_message (socket, NIMF_MESSAGE_COMMIT_REPLY, NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_RETRIEVE_SURROUNDING:
      g_signal_emit_by_name (NIMF_IM (client), "retrieve-surrounding", &retval);
      nimf_send_message (socket, NIMF_MESSAGE_RETRIEVE_SURROUNDING_REPLY,
                         &retval, sizeof (gboolean), NULL);
      break;
    case NIMF_MESSAGE_DELETE_SURROUNDING:
      nimf_message_ref (message);
      g_signal_emit_by_name (NIMF_IM (client), "delete-surrounding",
                             ((gint *) message->data)[0],
                             ((gint *) message->data)[1], &retval);
      nimf_message_unref (message);
      nimf_send_message (socket, NIMF_MESSAGE_DELETE_SURROUNDING_REPLY,
                         &retval, sizeof (gboolean), NULL);
      break;
    /* for agent */
    case NIMF_MESSAGE_ENGINE_CHANGED:
      nimf_message_ref (client->result->reply);
      g_signal_emit_by_name (client, "engine-changed", (gchar *) client->result->reply->data);
      nimf_message_unref (client->result->reply);
      break;
    case NIMF_MESSAGE_GET_LOADED_ENGINE_IDS_REPLY:
    case NIMF_MESSAGE_SET_ENGINE_BY_ID_REPLY:
      break;
    default:
      g_warning (G_STRLOC ": %s: Unknown message type: %d", G_STRFUNC, message->header->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

static void
nimf_client_init (NimfClient *client)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_client_constructed (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfClient *client = NIMF_CLIENT (object);

  client->result = g_slice_new0 (NimfResult);

  GSocketClient  *socket_client;
  GSocketAddress *address;
  GSocket        *socket;
  GError         *error = NULL;
  gint            retry_limit = 5;
  gint            retry_count = 0;

  address = g_unix_socket_address_new_with_type (NIMF_ADDRESS, -1,
                                                 G_UNIX_SOCKET_ADDRESS_ABSTRACT);
  socket_client = g_socket_client_new ();

  for (retry_count = 0; retry_count < retry_limit; retry_count++)
  {
    g_clear_error (&error);
    client->connection =
      g_socket_client_connect (socket_client, G_SOCKET_CONNECTABLE (address),
                               NULL, &error);
    if (client->connection)
      break;
    else
      g_usleep (G_USEC_PER_SEC);;
  }

  g_object_unref (address);
  g_object_unref (socket_client);

  if (client->connection == NULL)
  {
    g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
    g_clear_error (&error);
    g_signal_emit_by_name (client, "disconnected", NULL);
    return;
  }

  socket = g_socket_connection_get_socket (client->connection);

  if (!socket)
  {
    g_critical (G_STRLOC ": %s: %s", G_STRFUNC, "Can't get socket");
    g_signal_emit_by_name (client, "disconnected", NULL);
    return;
  }

  NimfMessage *message;

  nimf_send_message (socket, NIMF_MESSAGE_CONNECT, &client->type,
                     sizeof (NimfConnectionType), NULL);
  g_socket_condition_wait (socket, G_IO_IN, NULL, NULL);
  message = nimf_recv_message (socket);

  if (G_UNLIKELY (message == NULL ||
                  message->header->type != NIMF_MESSAGE_CONNECT_REPLY))
  {
    nimf_message_unref (message);
    g_critical ("Couldn't connect to nimf-daemon");
    g_signal_emit_by_name (client, "disconnected", NULL);

    if (message)
      nimf_message_unref (message);

    return;
  }

  nimf_message_unref (message);

  GMutex mutex;

  g_mutex_init (&mutex);
  g_mutex_lock (&mutex);

  if (G_UNLIKELY (nimf_client_sockets_context == NULL))
  {
    nimf_client_sockets_context = g_main_context_new ();
    nimf_client_sockets_context_ref_count++;
  }
  else
  {
    nimf_client_sockets_context = g_main_context_ref (nimf_client_sockets_context);
    nimf_client_sockets_context_ref_count++;
  }

  g_mutex_unlock (&mutex);

  /* when g_main_context_iteration(), iterate only sockets */
  client->sockets_context_source = g_socket_create_source (socket, G_IO_IN, NULL);
  g_source_set_can_recurse (client->sockets_context_source, TRUE);
  g_source_attach (client->sockets_context_source, nimf_client_sockets_context);
  g_source_set_callback (client->sockets_context_source,
                         (GSourceFunc) on_incoming_message, client, NULL);

  client->default_context_source = g_socket_create_source (socket, G_IO_IN, NULL);
  g_source_set_can_recurse (client->default_context_source, TRUE);
  g_source_set_callback (client->default_context_source,
                         (GSourceFunc) on_incoming_message, client, NULL);
  g_source_attach (client->default_context_source, NULL);

  return;
}

static void
nimf_client_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfClient *client = NIMF_CLIENT (object);

  if (client->sockets_context_source)
  {
    g_source_destroy (client->sockets_context_source);
    g_source_unref   (client->sockets_context_source);
  }

  if (client->default_context_source)
  {
    g_source_destroy (client->default_context_source);
    g_source_unref   (client->default_context_source);
  }

  if (client->connection)
    g_object_unref (client->connection);

  GMutex mutex;

  g_mutex_init (&mutex);
  g_mutex_lock (&mutex);

  if (nimf_client_sockets_context)
  {
    g_main_context_unref (nimf_client_sockets_context);
    nimf_client_sockets_context_ref_count--;

    if (nimf_client_sockets_context_ref_count == 0)
      nimf_client_sockets_context = NULL;
  }

  g_mutex_unlock (&mutex);

  if (client->result)
    g_slice_free (NimfResult, client->result);

  G_OBJECT_CLASS (nimf_client_parent_class)->finalize (object);
}

static void
nimf_client_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfClient *client = NIMF_CLIENT (object);

  switch (prop_id)
  {
    case PROP_CONNECTION_TYPE:
      client->type = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
nimf_client_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfClient *client = NIMF_CLIENT (object);

  switch (prop_id)
  {
    case PROP_CONNECTION_TYPE:
      g_value_set_enum (value, client->type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
nimf_client_class_init (NimfClientClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = nimf_client_finalize;
  object_class->set_property = nimf_client_set_property;
  object_class->get_property = nimf_client_get_property;
  object_class->constructed  = nimf_client_constructed;

  g_object_class_install_property (object_class,
                                   PROP_CONNECTION_TYPE,
                                   g_param_spec_enum ("connection-type",
                                                      "connection type",
                                                      "connection type",
                                                      NIMF_TYPE_CONNECTION_TYPE,
                                                      NIMF_CONNECTION_NIMF_IM,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  nimf_client_signals[ENGINE_CHANGED] =
    g_signal_new (g_intern_static_string ("engine-changed"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfClientClass, engine_changed),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
  nimf_client_signals[DISCONNECTED] =
    g_signal_new (g_intern_static_string ("disconnected"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfClientClass, disconnected),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}
