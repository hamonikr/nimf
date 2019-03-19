/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-nim.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2019 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-nim.h"
#include "nimf-nim-ic.h"
#include "nimf-message-private.h"
#include "nimf-connection.h"
#include <gio/gunixsocketaddress.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "nimf-utils.h"

G_DEFINE_DYNAMIC_TYPE (NimfNim, nimf_nim, NIMF_TYPE_SERVICE);

static const gchar *
nimf_nim_get_id (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_SERVICE (service), NULL);

  return NIMF_NIM (service)->id;
}

static gboolean
on_incoming (GSocket        *socket,
             GIOCondition    condition,
             NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMessage *message;
  gboolean     retval;
  nimf_message_unref (connection->result->reply);
  connection->result->is_dispatched = TRUE;

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    g_debug (G_STRLOC ": condition & (G_IO_HUP | G_IO_ERR)");

    g_socket_close (socket, NULL);

    GHashTableIter  iter;
    gpointer        ic;

    g_hash_table_iter_init (&iter, connection->ics);

    while (g_hash_table_iter_next (&iter, NULL, &ic))
      nimf_service_ic_reset (ic);

    connection->result->reply = NULL;
    g_hash_table_remove (connection->nim->connections,
                         GUINT_TO_POINTER (nimf_connection_get_id (connection)));

    return G_SOURCE_REMOVE;
  }

  message = nimf_recv_message (socket);
  connection->result->reply = message;

  if (G_UNLIKELY (message == NULL))
  {
    g_critical (G_STRLOC ": NULL message");
    return G_SOURCE_CONTINUE;
  }

  NimfNimIC *nic;
  guint16    icid = message->header->icid;

  nic = g_hash_table_lookup (connection->ics, GUINT_TO_POINTER (icid));

  switch (message->header->type)
  {
    case NIMF_MESSAGE_CREATE_CONTEXT:
      nic = nimf_nim_ic_new (icid, connection);
      g_hash_table_insert (connection->ics, GUINT_TO_POINTER (icid), nic);
      nimf_send_message (socket, icid, NIMF_MESSAGE_CREATE_CONTEXT_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_DESTROY_CONTEXT:
      g_hash_table_remove (connection->ics, GUINT_TO_POINTER (icid));
      nimf_send_message (socket, icid, NIMF_MESSAGE_DESTROY_CONTEXT_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_FILTER_EVENT:
      nimf_message_ref (message);
      retval = nimf_service_ic_filter_event (NIMF_SERVICE_IC (nic), (NimfEvent *) message->data);
      nimf_message_unref (message);
      nimf_send_message (socket, icid, NIMF_MESSAGE_FILTER_EVENT_REPLY,
                         &retval, sizeof (gboolean), NULL);
      break;
    case NIMF_MESSAGE_RESET:
      nimf_service_ic_reset (NIMF_SERVICE_IC (nic));
      nimf_send_message (socket, icid, NIMF_MESSAGE_RESET_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_FOCUS_IN:
      nimf_service_ic_focus_in (NIMF_SERVICE_IC (nic));
      nimf_send_message (socket, icid, NIMF_MESSAGE_FOCUS_IN_REPLY,
                         NULL, 0, NULL);
      connection->nim->last_focused_conn_id = connection->id;
      connection->nim->last_focused_icid    = icid;
      break;
    case NIMF_MESSAGE_FOCUS_OUT:
      nimf_service_ic_focus_out (NIMF_SERVICE_IC (nic));
      nimf_send_message (socket, icid, NIMF_MESSAGE_FOCUS_OUT_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_SET_SURROUNDING:
      {
        nimf_message_ref (message);
        gchar   *data     = message->data;
        guint16  data_len = message->header->data_len;

        gint   str_len      = data_len - 1 - 2 * sizeof (gint);
        gint   cursor_index = *(gint *) (data + data_len - sizeof (gint));

        nimf_service_ic_set_surrounding (NIMF_SERVICE_IC (nic), data, str_len, cursor_index);
        nimf_message_unref (message);
        nimf_send_message (socket, icid,
                           NIMF_MESSAGE_SET_SURROUNDING_REPLY, NULL, 0, NULL);
      }
      break;
    case NIMF_MESSAGE_SET_CURSOR_LOCATION:
      nimf_message_ref (message);
      nimf_service_ic_set_cursor_location (NIMF_SERVICE_IC (nic), (NimfRectangle *) message->data);
      nimf_message_unref (message);
      nimf_send_message (socket, icid, NIMF_MESSAGE_SET_CURSOR_LOCATION_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_SET_USE_PREEDIT:
      nimf_message_ref (message);
      nimf_service_ic_set_use_preedit (NIMF_SERVICE_IC (nic), *(gboolean *) message->data);
      nimf_message_unref (message);
      nimf_send_message (socket, icid, NIMF_MESSAGE_SET_USE_PREEDIT_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_PREEDIT_START_REPLY:
    case NIMF_MESSAGE_PREEDIT_CHANGED_REPLY:
    case NIMF_MESSAGE_PREEDIT_END_REPLY:
    case NIMF_MESSAGE_COMMIT_REPLY:
    case NIMF_MESSAGE_RETRIEVE_SURROUNDING_REPLY:
    case NIMF_MESSAGE_DELETE_SURROUNDING_REPLY:
    case NIMF_MESSAGE_BEEP_REPLY:
      break;
    default:
      g_warning ("Unknown message type: %d", message->header->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

static guint16
nimf_nim_add_connection (NimfNim        *nim,
                         NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  guint16 id;

  do
    id = nim->next_id++;
  while (id == 0 || g_hash_table_contains (nim->connections,
                                           GUINT_TO_POINTER (id)));
  connection->id  = id;
  connection->nim = nim;
  g_hash_table_insert (nim->connections, GUINT_TO_POINTER (id), connection);

  return id;
}

static gboolean
on_new_connection (GSocketService    *service,
                   GSocketConnection *socket_connection,
                   GObject           *source_object,
                   NimfNim           *nim)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfConnection *connection;
  connection = nimf_connection_new ();
  connection->socket = g_socket_connection_get_socket (socket_connection);
  nimf_nim_add_connection (nim, connection);

  connection->source = g_socket_create_source (connection->socket, G_IO_IN, NULL);
  connection->socket_connection = g_object_ref (socket_connection);
  g_source_set_can_recurse (connection->source, TRUE);
  g_source_set_callback (connection->source,
                         (GSourceFunc) on_incoming,
                         connection, NULL);
  g_source_attach (connection->source, NULL);

  return TRUE;
}

static void nimf_nim_change_engine (NimfService *service,
                                    const gchar *engine_id,
                                    const gchar *method_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNim *nim = NIMF_NIM (service);

  if (nim->last_focused_conn_id > 0)
  {
    NimfConnection *connection;
    connection = g_hash_table_lookup (nim->connections,
                                      GUINT_TO_POINTER (nim->last_focused_conn_id));
    if (connection)
      nimf_connection_change_engine (connection, engine_id, method_id);
  }
}

static void nimf_nim_change_engine_by_id (NimfService *service,
                                          const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNim *nim = NIMF_NIM (service);

  if (nim->last_focused_conn_id > 0)
  {
    NimfConnection *connection;
    connection = g_hash_table_lookup (nim->connections,
                                      GUINT_TO_POINTER (nim->last_focused_conn_id));
    if (connection)
      nimf_connection_change_engine_by_id (connection, engine_id);
  }
}

static gboolean nimf_nim_is_active (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_NIM (service)->active;
}

static gboolean
nimf_nim_start (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNim *nim = NIMF_NIM (service);

  if (nim->active)
    return TRUE;

  GSocketAddress *address;
  gchar          *path;
  GError         *error = NULL;

  nim->service = g_socket_service_new ();
  path = nimf_get_socket_path ();
  address = g_unix_socket_address_new_with_type (path, -1,
                                                 G_UNIX_SOCKET_ADDRESS_PATH);
  g_socket_listener_add_address (G_SOCKET_LISTENER (nim->service), address,
                                 G_SOCKET_TYPE_STREAM,
                                 G_SOCKET_PROTOCOL_DEFAULT,
                                 NULL, NULL, &error);
  g_object_unref (address);
  g_chmod (path, 0600);
  g_free  (path);

  if (error)
  {
    g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
    g_clear_error (&error);

    return FALSE;
  }

  g_signal_connect (nim->service, "incoming",
                    G_CALLBACK (on_new_connection), nim);
  g_socket_service_start (nim->service);

  return nim->active = TRUE;
}

static void nimf_nim_stop (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNim *nim = NIMF_NIM (service);

  if (!nim->active)
    return;

  g_socket_service_stop (nim->service);

  nim->active = FALSE;
}

static void
nimf_nim_init (NimfNim *nim)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nim->id  = g_strdup ("nimf-nim");
  nim->connections = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
                                            (GDestroyNotify) g_object_unref);
}

static void nimf_nim_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfService *service = NIMF_SERVICE (object);
  NimfNim     *nim     = NIMF_NIM (object);

  if (nimf_nim_is_active (service))
    nimf_nim_stop (service);

  if (nim->service != NULL)
    g_object_unref (nim->service);

  g_hash_table_unref (nim->connections);
  g_free (nim->id);

  G_OBJECT_CLASS (nimf_nim_parent_class)->finalize (object);
}

static void
nimf_nim_class_init (NimfNimClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass     *object_class  = G_OBJECT_CLASS (class);
  NimfServiceClass *service_class = NIMF_SERVICE_CLASS (class);

  service_class->get_id              = nimf_nim_get_id;
  service_class->start               = nimf_nim_start;
  service_class->stop                = nimf_nim_stop;
  service_class->is_active           = nimf_nim_is_active;
  service_class->change_engine       = nimf_nim_change_engine;
  service_class->change_engine_by_id = nimf_nim_change_engine_by_id;

  object_class->finalize = nimf_nim_finalize;
}

static void
nimf_nim_class_finalize (NimfNimClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_nim_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_nim_get_type ();
}
