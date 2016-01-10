/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-agent.c
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

#include "nimf-agent.h"
#include "nimf-private.h"
#include <gio/gunixsocketaddress.h>
#include "nimf-marshalers.h"
#include <string.h>

enum {
  ENGINE_CHANGED,
  DISCONNECTED,
  LAST_SIGNAL
};

static guint agent_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (NimfAgent, nimf_agent, G_TYPE_OBJECT);

static gboolean
on_incoming_message (GSocket      *socket,
                     GIOCondition  condition,
                     NimfAgent    *agent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMessage *message;
  nimf_message_unref (agent->result->reply);
  agent->result->is_dispatched = TRUE;

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    if (!g_socket_is_closed (socket))
      g_socket_close (socket, NULL);

    agent->result->reply = NULL;
    g_signal_emit_by_name (agent, "disconnected", NULL);

    g_warning (G_STRLOC ": %s: G_IO_HUP | G_IO_ERR", G_STRFUNC);

    return G_SOURCE_REMOVE;
  }

  message = nimf_recv_message (socket);
  agent->result->reply = message;

  if (G_UNLIKELY (message == NULL))
  {
    g_critical (G_STRLOC ": NULL message");
    return G_SOURCE_CONTINUE;
  }

  switch (message->header->type)
  {
    case NIMF_MESSAGE_ENGINE_CHANGED:
      nimf_message_ref (agent->result->reply);
      g_signal_emit_by_name (agent, "engine-changed", (gchar *) agent->result->reply->data);
      nimf_message_unref (agent->result->reply);
      break;
    /* reply */
    case NIMF_MESSAGE_GET_LOADED_ENGINE_IDS_REPLY:
    case NIMF_MESSAGE_SET_ENGINE_BY_ID_REPLY:
      break;
    default:
      g_warning (G_STRLOC ": %s: Unknown message type: %d", G_STRFUNC, message->header->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

gboolean
nimf_agent_connect_to_server (NimfAgent *agent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSocketAddress *address;
  GSocketClient  *client;
  GSocket        *socket;
  NimfMessage    *message;
  GError         *error = NULL;
  gint            retry_limit = 5;
  gint            retry_count = 0;

  address = g_unix_socket_address_new_with_type (NIMF_ADDRESS, -1,
                                                 G_UNIX_SOCKET_ADDRESS_ABSTRACT);
  client = g_socket_client_new ();

  for (retry_count = 0; retry_count < retry_limit; retry_count++)
  {
    g_clear_error (&error);
    agent->connection = g_socket_client_connect (client,
                                                 G_SOCKET_CONNECTABLE (address),
                                                 NULL, &error);
    if (agent->connection)
      break;
    else
      g_usleep (G_USEC_PER_SEC);;
  }

  g_object_unref (address);
  g_object_unref (client);

  if (agent->connection == NULL)
  {
    g_critical (G_STRLOC ": %s", error->message);
    g_clear_error (&error);
    g_signal_emit_by_name (agent, "disconnected", NULL);
    return FALSE;
  }

  socket = g_socket_connection_get_socket (agent->connection);

  if (!socket)
  {
    g_critical (G_STRLOC ": Can't get socket");
    g_signal_emit_by_name (agent, "disconnected", NULL);
    return FALSE;
  }

  NimfConnectionType type = NIMF_CONNECTION_NIMF_AGENT;

  nimf_send_message (socket, NIMF_MESSAGE_CONNECT, &type,
                     sizeof (NimfConnectionType), NULL);
  g_socket_condition_wait (socket, G_IO_IN, NULL, NULL);
  message = nimf_recv_message (socket);

  if (G_UNLIKELY (message == NULL ||
                  message->header->type != NIMF_MESSAGE_CONNECT_REPLY))
  {
    nimf_message_unref (message);
    g_critical ("Couldn't connect to nimf daemon");
    g_signal_emit_by_name (agent, "disconnected", NULL);
    return FALSE;
  }

  nimf_message_unref (message);

  agent->source = g_socket_create_source (socket, G_IO_IN, NULL);
  g_source_attach (agent->source, NULL);
  g_source_set_callback (agent->source, (GSourceFunc) on_incoming_message,
                         agent, NULL);
  return TRUE;
}

static void
nimf_agent_init (NimfAgent *agent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  agent->result = g_slice_new0 (NimfResult);
}

static void
nimf_agent_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfAgent *agent = NIMF_AGENT (object);

  if (agent->source)
  {
    g_source_destroy (agent->source);
    g_source_unref   (agent->source);
  }

  if (agent->connection)
    g_object_unref (agent->connection);

  nimf_message_unref (agent->result->reply);

  if (agent->result)
    g_slice_free (NimfResult, agent->result);

  G_OBJECT_CLASS (nimf_agent_parent_class)->finalize (object);
}

static void
nimf_agent_class_init (NimfAgentClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->finalize = nimf_agent_finalize;

  agent_signals[ENGINE_CHANGED] =
    g_signal_new (g_intern_static_string ("engine-changed"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfAgentClass, engine_changed),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
  agent_signals[DISCONNECTED] =
    g_signal_new (g_intern_static_string ("disconnected"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfAgentClass, disconnected),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

NimfAgent *
nimf_agent_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (NIMF_TYPE_AGENT, NULL);
}

void
nimf_agent_set_engine_by_id (NimfAgent   *agent,
                             const gchar *id,
                             gboolean     is_english_mode)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  /* TODO: 요 코드 정상 작동 여부 반드시 확인할 것 */
  if (agent->connection == NULL ||
      !g_socket_connection_is_connected (agent->connection))
    return;

  GSocket *socket = g_socket_connection_get_socket (agent->connection);

  gchar *data     = NULL;
  gint   str_len  = strlen (id);
  gint   data_len = str_len + 1 + sizeof (gboolean);

  data = g_strndup (id, data_len - 1);
  *(gboolean *) (data + str_len + 1) = is_english_mode;

  nimf_send_message (socket, NIMF_MESSAGE_SET_ENGINE_BY_ID, data,
                     data_len, g_free);
  nimf_result_iteration_until (agent->result, NULL,
                               NIMF_MESSAGE_SET_ENGINE_BY_ID_REPLY);
}

NimfEngineInfo *
nimf_agent_get_engine_info (gchar *name)
{
  /* TODO */
  return NULL;
}

/**
 * nimf_agent_get_loaded_engine_ids:
 * @agent: a #NimfAgent.
 *
 * Returns: (transfer full): gchar **
 */
gchar **
nimf_agent_get_loaded_engine_ids (NimfAgent *agent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  /* TODO: 요 코드 정상 작동 여부 반드시 확인할 것 */
  if (agent->connection == NULL ||
      !g_socket_connection_is_connected (agent->connection))
    return NULL;

  GSocket *socket = g_socket_connection_get_socket (agent->connection);

  nimf_send_message (socket, NIMF_MESSAGE_GET_LOADED_ENGINE_IDS, NULL, 0, NULL);
  nimf_result_iteration_until (agent->result, NULL,
                               NIMF_MESSAGE_GET_LOADED_ENGINE_IDS_REPLY);
  if (agent->result->reply == NULL)
    return NULL;
  /* 0x1e is RS (record separator) */
  return g_strsplit (agent->result->reply->data, "\x1e", -1);
}
