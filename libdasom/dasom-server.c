/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-server.c
 * This file is part of Dasom.
 *
 * Copyright (C) 2015 Hodong Kim <hodong@cogno.org>
 *
 * Dasom is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Dasom is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program;  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "dasom-server.h"
#include "dasom-private.h"
#include <string.h>
#include <gio/gunixsocketaddress.h>
#include "dasom-module.h"
#include "IMdkit/Xi18n.h"
#include "dasom-english.h"

enum
{
  PROP_0,
  PROP_ADDRESS,
};

static gboolean
on_incoming_message_dasom (GSocket         *socket,
                           GIOCondition     condition,
                           DasomConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomMessage *message;
  gboolean      retval;
  dasom_message_unref (connection->result->reply);

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    g_debug (G_STRLOC ": condition & (G_IO_HUP | G_IO_ERR)");

    g_socket_close (socket, NULL);

    connection->result->reply   = NULL;

    if (G_UNLIKELY (connection->type == DASOM_CONNECTION_DASOM_AGENT))
      connection->server->agents_list =
        g_list_remove (connection->server->agents_list, connection);

    g_hash_table_remove (connection->server->connections,
                         GUINT_TO_POINTER (dasom_connection_get_id (connection)));

    return G_SOURCE_REMOVE;
  }

  if (connection->type == DASOM_CONNECTION_DASOM_IM)
    dasom_engine_set_english_mode (connection->engine,
                                   connection->is_english_mode);

  message = dasom_recv_message (socket);
  connection->result->reply = message;
  connection->result->is_dispatched = TRUE;

  if (G_UNLIKELY (message == NULL))
  {
    g_critical (G_STRLOC ": NULL message");
    return G_SOURCE_CONTINUE;
  }

  switch (message->header->type)
  {
    case DASOM_MESSAGE_FILTER_EVENT:
      dasom_message_ref (message);
      retval = dasom_connection_filter_event (connection,
                                              (DasomEvent *) message->data);
      dasom_message_unref (message);
      dasom_send_message (socket, DASOM_MESSAGE_FILTER_EVENT_REPLY, &retval,
                          sizeof (gboolean), NULL);
      break;
    case DASOM_MESSAGE_GET_PREEDIT_STRING:
      {
        gchar *data = NULL;
        gint   cursor_pos;
        gint   str_len = 0;

        dasom_connection_get_preedit_string (connection, &data, &cursor_pos);

        str_len = strlen (data);
        data = g_realloc (data, str_len + 1 + sizeof (gint));
        *(gint *) (data + str_len + 1) = cursor_pos;

        dasom_send_message (socket, DASOM_MESSAGE_GET_PREEDIT_STRING_REPLY,
                            data,
                            str_len + 1 + sizeof (gint),
                            NULL);
        g_free (data);
      }
      break;
    case DASOM_MESSAGE_RESET:
      dasom_connection_reset (connection);
      dasom_send_message (socket, DASOM_MESSAGE_RESET_REPLY, NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_FOCUS_IN:
      dasom_connection_focus_in (connection);
      dasom_send_message (socket, DASOM_MESSAGE_FOCUS_IN_REPLY, NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_FOCUS_OUT:
      dasom_connection_focus_out (connection);
      dasom_send_message (socket, DASOM_MESSAGE_FOCUS_OUT_REPLY, NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_SET_SURROUNDING:
      {
        dasom_message_ref (message);
        gchar   *data     = message->data;
        guint16  data_len = message->header->data_len;

        gint   str_len      = data_len - 1 - 2 * sizeof (gint);
        gint   cursor_index = *(gint *) (data + data_len - sizeof (gint));

        dasom_connection_set_surrounding (connection, data, str_len,
                                          cursor_index);
        dasom_message_unref (message);
        dasom_send_message (socket, DASOM_MESSAGE_SET_SURROUNDING_REPLY, NULL, 0, NULL);
      }
      break;
    case DASOM_MESSAGE_GET_SURROUNDING:
      {
        gchar *data;
        gint   cursor_index;
        gint   str_len = 0;

        retval = dasom_connection_get_surrounding (connection, &data,
                                                   &cursor_index);
        str_len = strlen (data);
        data = g_realloc (data, str_len + 1 + sizeof (gint) + sizeof (gboolean));
        *(gint *) (data + str_len + 1) = cursor_index;
        *(gboolean *) (data + str_len + 1 + sizeof (gint)) = retval;

        dasom_send_message (socket, DASOM_MESSAGE_GET_SURROUNDING_REPLY,
                            data,
                            str_len + 1 + sizeof (gint) + sizeof (gboolean),
                            NULL);
        g_free (data);
      }
      break;
    case DASOM_MESSAGE_SET_CURSOR_LOCATION:
      dasom_message_ref (message);
      dasom_connection_set_cursor_location (connection,
                                            (DasomRectangle *) message->data);
      dasom_message_unref (message);
      dasom_send_message (socket, DASOM_MESSAGE_SET_CURSOR_LOCATION_REPLY,
                          NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_SET_USE_PREEDIT:
      dasom_message_ref (message);
      dasom_connection_set_use_preedit (connection, *(gboolean *) message->data);
      dasom_message_unref (message);
      dasom_send_message (socket, DASOM_MESSAGE_SET_USE_PREEDIT_REPLY,
                          NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_PREEDIT_START_REPLY:
    case DASOM_MESSAGE_PREEDIT_CHANGED_REPLY:
    case DASOM_MESSAGE_PREEDIT_END_REPLY:
    case DASOM_MESSAGE_COMMIT_REPLY:
    case DASOM_MESSAGE_RETRIEVE_SURROUNDING_REPLY:
    case DASOM_MESSAGE_DELETE_SURROUNDING_REPLY:
      break;
    default:
      g_warning ("Unknown message type: %d", message->header->type);
      break;
  }

  if (connection->type == DASOM_CONNECTION_DASOM_IM)
    connection->is_english_mode =
      dasom_engine_get_english_mode (connection->engine);

  return G_SOURCE_CONTINUE;
}

static guint16
dasom_server_add_connection (DasomServer     *server,
                             DasomConnection *connection)
{
  guint16 id;

  do
    id = server->next_id++;
  while (id == 0 || g_hash_table_contains (server->connections,
                                           GUINT_TO_POINTER (id)));
  connection->id = id;
  connection->server = server;
  g_hash_table_insert (server->connections, GUINT_TO_POINTER (id), connection);

  return id;
}

static gboolean
on_new_connection (GSocketService    *service,
                   GSocketConnection *socket_connection,
                   GObject           *source_object,
                   DasomServer       *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSocket *socket = g_socket_connection_get_socket (socket_connection);

  DasomMessage *message;
  message = dasom_recv_message (socket);

  if (G_UNLIKELY (message == NULL ||
                  message->header->type != DASOM_MESSAGE_CONNECT))
  {
    g_critical (G_STRLOC ": Couldn't connect");
    dasom_message_unref (message);
    dasom_send_message (socket, DASOM_MESSAGE_ERROR, NULL, 0, NULL);
    return FALSE;
  }

  dasom_send_message (socket, DASOM_MESSAGE_CONNECT_REPLY, NULL, 0, NULL);

  DasomConnection *connection;
  connection = dasom_connection_new (*(DasomConnectionType *) message->data,
                                     dasom_server_get_default_engine (server), NULL);
  dasom_message_unref (message);
  connection->socket = socket;
  dasom_server_add_connection (server, connection);

  if (connection->type == DASOM_CONNECTION_DASOM_AGENT)
    server->agents_list = g_list_prepend (server->agents_list, connection);

  connection->source = g_socket_create_source (socket, G_IO_IN, NULL);
  connection->socket_connection = g_object_ref (socket_connection);
  g_source_set_can_recurse (connection->source, TRUE);
  g_source_set_callback (connection->source,
                         (GSourceFunc) on_incoming_message_dasom,
                         connection, NULL);
  g_source_attach (connection->source, server->main_context);

  return TRUE;
}

static gboolean
dasom_server_initable_init (GInitable     *initable,
                            GCancellable  *cancellable,
                            GError       **error)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomServer    *server = DASOM_SERVER (initable);
  GSocketAddress *address;
  GError         *local_error = NULL;

  server->listener = G_SOCKET_LISTENER (g_socket_service_new ());
  /* server->listener = G_SOCKET_LISTENER (g_threaded_socket_service_new (-1)); */

  if (g_unix_socket_address_abstract_names_supported ())
    address = g_unix_socket_address_new_with_type (server->address, -1,
                                                   G_UNIX_SOCKET_ADDRESS_ABSTRACT);
  else
  {
    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                         "Abstract UNIX domain socket names are not supported.");
    return FALSE;
  }

  g_socket_listener_add_address (server->listener, address,
                                 G_SOCKET_TYPE_STREAM,
                                 G_SOCKET_PROTOCOL_DEFAULT,
                                 NULL, NULL, &local_error);
  g_object_unref (address);

  if (local_error)
  {
    g_propagate_error (error, local_error);
    return FALSE;
  }

  server->is_using_listener = TRUE;
  server->run_signal_handler_id =
    g_signal_connect (G_SOCKET_SERVICE (server->listener), "incoming",
                      (GCallback) on_new_connection, server);
/*
  server->run_signal_handler_id = g_signal_connect (G_SOCKET_SERVICE (server->listener),
                                                    "run",
                                                    G_CALLBACK (on_run),
                                                    server);
*/

  return TRUE;
}

static void
dasom_server_initable_iface_init (GInitableIface *initable_iface)
{
  initable_iface->init = dasom_server_initable_init;
}

G_DEFINE_TYPE_WITH_CODE (DasomServer, dasom_server, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                dasom_server_initable_iface_init));

static gint
on_comparing_engine_with_id (DasomEngine *engine, const gchar *id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_strcmp0 (dasom_engine_get_id (engine), id);
}

DasomEngine *
dasom_server_get_instance (DasomServer *server,
                           const gchar *id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  list = g_list_find_custom (g_list_first (server->instances),
                             id,
                             (GCompareFunc) on_comparing_engine_with_id);
  if (list)
    return list->data;

  return NULL;
}

DasomEngine *
dasom_server_get_next_instance (DasomServer *server, DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  server->instances = g_list_first (server->instances);
  server->instances = g_list_find  (server->instances, engine);

  list = g_list_next (server->instances);

  if (list == NULL)
    list = g_list_first (server->instances);

  if (list)
  {
    server->instances = list;
    return list->data;
  }

  g_assert (list != NULL);

  return engine;
}

DasomEngine *
dasom_server_get_default_engine (DasomServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings *settings = g_settings_new ("org.freedesktop.Dasom");
  gchar *engine_id = g_settings_get_string (settings, "default-engine");
  DasomEngine *engine = dasom_server_get_instance (server, engine_id);

  g_free (engine_id);
  g_object_unref (settings);

  if (engine == NULL)
    engine = dasom_server_get_instance (server, "dasom-english");

  g_assert (engine != NULL);

  return engine;
}

static GList *dasom_server_create_module_instances (DasomServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *instances = NULL;

  GHashTableIter iter;
  gpointer value;

  g_hash_table_iter_init (&iter, server->module_manager->modules);
  while (g_hash_table_iter_next (&iter, NULL, &value))
  {
    DasomModule *module = value;
    instances = g_list_prepend (instances,
                                g_object_new (module->type,
                                              "server", server,
                                              NULL));
  }

  /* add english engine */
  instances = g_list_prepend (instances,
                              g_object_new (DASOM_TYPE_ENGLISH,
                                            "server", server, NULL));
  return instances;
}

static void
dasom_server_init (DasomServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings *settings = g_settings_new ("org.freedesktop.Dasom");
  gchar **hotkeys = g_settings_get_strv (settings, "hotkeys");
  server->hotkeys = dasom_key_newv ((const gchar **) hotkeys);
  g_object_unref (settings);
  g_strfreev (hotkeys);

  server->candidate = dasom_candidate_new ();
  server->module_manager = dasom_module_manager_get_default ();
  server->instances = dasom_server_create_module_instances (server);

  server->main_context = g_main_context_ref_thread_default ();
  server->connections = g_hash_table_new_full (g_direct_hash,
                                               g_direct_equal,
                                               NULL,
                                               (GDestroyNotify) g_object_unref);
  server->agents_list = NULL;
}

void
dasom_server_stop (DasomServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_SERVER (server));

  if (!server->active)
    return;

  g_assert (server->is_using_listener);
  g_assert (server->run_signal_handler_id > 0);

  g_signal_handler_disconnect (server->listener, server->run_signal_handler_id);
  server->run_signal_handler_id = 0;

  g_socket_service_stop (G_SOCKET_SERVICE (server->listener));
  server->active = FALSE;
}

static void
dasom_server_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomServer *server = DASOM_SERVER (object);

  if (server->run_signal_handler_id > 0)
    g_signal_handler_disconnect (server->listener, server->run_signal_handler_id);

  if (server->listener != NULL)
    g_object_unref (server->listener);

  g_object_unref (server->module_manager);

  if (server->instances)
  {
    g_list_free_full (server->instances, g_object_unref);
    server->instances = NULL;
  }

  g_object_unref (server->candidate);
  g_hash_table_unref (server->connections);
  g_list_free (server->agents_list);
  dasom_key_freev (server->hotkeys);
  g_free (server->address);

  if (server->xevent_source)
  {
    g_source_destroy (server->xevent_source);
    g_source_unref   (server->xevent_source);
  }

  g_main_context_unref (server->main_context);

  G_OBJECT_CLASS (dasom_server_parent_class)->finalize (object);
}

static void
dasom_server_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  DasomServer *server = DASOM_SERVER (object);

  switch (prop_id)
  {
    case PROP_ADDRESS:
      g_value_set_string (value, server->address);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
dasom_server_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  DasomServer *server = DASOM_SERVER (object);

  switch (prop_id)
  {
    case PROP_ADDRESS:
      server->address = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
dasom_server_class_init (DasomServerClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize     = dasom_server_finalize;
  object_class->set_property = dasom_server_set_property;
  object_class->get_property = dasom_server_get_property;

  g_object_class_install_property (object_class,
                                   PROP_ADDRESS,
                                   g_param_spec_string ("address",
                                                        "Address",
                                                        "The address to listen on",
                                                        NULL,
                                                        G_PARAM_READABLE |
                                                        G_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB |
                                                        G_PARAM_STATIC_NICK));
}

DasomServer *
dasom_server_new (const gchar  *address,
                  GError      **error)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (address != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return g_initable_new (DASOM_TYPE_SERVER, NULL, error,
                         "address", address, NULL);
}

typedef struct
{
  GSource  source;
  Display *display;
  GPollFD  poll_fd;
} DasomXEventSource;

static gboolean dasom_xevent_source_prepare (GSource *source,
                                             gint    *timeout)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  Display *display = ((DasomXEventSource *) source)->display;
  *timeout = -1;
  return XPending (display) > 0;
}

static gboolean dasom_xevent_source_check (GSource *source)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomXEventSource *display_source = (DasomXEventSource *) source;

  if (display_source->poll_fd.revents & G_IO_IN)
    return XPending (display_source->display) > 0;
  else
    return FALSE;
}

int dasom_server_xim_set_ic_values (DasomServer      *server,
                                    XIMS              xims,
                                    IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));
  CARD16 i;

  for (i = 0; i < data->ic_attr_num; i++)
  {
    if (g_strcmp0 (XNInputStyle, data->ic_attr[i].name) == 0)
      g_message ("XNInputStyle is ignored");
    else if (g_strcmp0 (XNClientWindow, data->ic_attr[i].name) == 0)
      connection->client_window = *(Window *) data->ic_attr[i].value;
    else if (g_strcmp0 (XNFocusWindow, data->ic_attr[i].name) == 0)
      connection->focus_window = *(Window *) data->ic_attr[i].value;
    else
      g_warning (G_STRLOC ": %s %s", G_STRFUNC, data->ic_attr[i].name);
  }

  for (i = 0; i < data->preedit_attr_num; i++)
  {
    if (g_strcmp0 (XNPreeditState, data->preedit_attr[i].name) == 0)
    {
      XIMPreeditState state = *(XIMPreeditState *) data->preedit_attr[i].value;
      switch (state)
      {
        case XIMPreeditEnable:
          dasom_connection_set_use_preedit (connection, TRUE);
          break;
        case XIMPreeditDisable:
          dasom_connection_set_use_preedit (connection, FALSE);
          break;
        default:
          g_message ("XIMPreeditState: %ld is ignored", state);
          break;
      }
    }
    else
      g_critical (G_STRLOC ": %s: %s is ignored",
                  G_STRFUNC, data->preedit_attr[i].name);
  }

  for (i = 0; i < data->status_attr_num; i++)
  {
    g_critical (G_STRLOC ": %s: %s is ignored",
                G_STRFUNC, data->status_attr[i].name);
  }

  dasom_connection_xim_set_cursor_location (connection, xims);

  return 1;
}

int dasom_server_xim_create_ic (DasomServer      *server,
                                XIMS              xims,
                                IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s, data->connect_id: %d", G_STRFUNC, data->connect_id);

  DasomConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER ((gint) data->icid));

  if (!connection)
  {
    connection = dasom_connection_new (DASOM_CONNECTION_XIM,
                                       dasom_server_get_default_engine (server),
                                       xims);
    connection->xim_connect_id = data->connect_id;
    data->icid = dasom_server_add_connection (server, connection);
    g_debug (G_STRLOC ": icid = %d", data->icid);
  }

  dasom_server_xim_set_ic_values (server, xims, data);

  return 1;
}

int dasom_server_xim_destroy_ic (DasomServer       *server,
                                 XIMS               xims,
                                 IMDestroyICStruct *data)
{
  g_debug (G_STRLOC ": %s, data->icid = %d", G_STRFUNC, data->icid);

  return g_hash_table_remove (server->connections,
                              GUINT_TO_POINTER (data->icid));
}

int dasom_server_xim_get_ic_values (DasomServer      *server,
                                    XIMS              xims,
                                    IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));
  CARD16 i;

  for (i = 0; i < data->ic_attr_num; i++)
  {
    if (g_strcmp0 (XNFilterEvents, data->ic_attr[i].name) == 0)
    {
      data->ic_attr[i].value_length = sizeof (CARD32);
      data->ic_attr[i].value = g_malloc (sizeof (CARD32));
      *(CARD32 *) data->ic_attr[i].value = KeyPressMask | KeyReleaseMask;
    }
    else if (g_strcmp0 (XNSeparatorofNestedList, data->ic_attr[i].name) == 0)
    {
      data->ic_attr[i].value_length = sizeof (CARD16);
      data->ic_attr[i].value = g_malloc (sizeof (CARD16));
      *(CARD16 *) data->ic_attr[i].value = 0;
    }
    else
      g_critical (G_STRLOC ": %s: %s is ignored",
                  G_STRFUNC, data->ic_attr[i].name);
  }

  for (i = 0; i < data->preedit_attr_num; i++)
  {
    if (g_strcmp0 (XNPreeditState, data->preedit_attr[i].name) == 0)
    {
      data->preedit_attr[i].value_length = sizeof (XIMPreeditState);
      data->preedit_attr[i].value = g_malloc (sizeof (XIMPreeditState));

      if (connection->use_preedit)
        *(XIMPreeditState *) data->preedit_attr[i].value = XIMPreeditEnable;
      else
        *(XIMPreeditState *) data->preedit_attr[i].value = XIMPreeditDisable;
    }
    else
      g_critical (G_STRLOC ": %s: %s is ignored",
                  G_STRFUNC, data->preedit_attr[i].name);
  }

  for (i = 0; i < data->status_attr_num; i++)
    g_critical (G_STRLOC ": %s: %s is ignored",
                G_STRFUNC, data->status_attr[i].name);

  return 1;
}

int dasom_server_xim_set_ic_focus (DasomServer         *server,
                                   XIMS                 xims,
                                   IMChangeFocusStruct *data)
{
  DasomConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));

  g_debug (G_STRLOC ": %s, icid = %d, connection id = %d",
           G_STRFUNC, data->icid, connection->id);

  dasom_connection_focus_in (connection);

  return 1;
}

int dasom_server_xim_unset_ic_focus (DasomServer         *server,
                                     XIMS                 xims,
                                     IMChangeFocusStruct *data)
{
  DasomConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));

  g_debug (G_STRLOC ": %s, icid = %d, connection id = %d",
           G_STRFUNC, data->icid, connection->id);

  dasom_connection_focus_out (connection);

  return 1;
}

int dasom_server_xim_forward_event (DasomServer          *server,
                                    XIMS                  xims,
                                    IMForwardEventStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  XKeyEvent            *xevent;
  DasomEvent           *event;
  DasomEventType        type;
  IMForwardEventStruct *fw_event;
  gboolean              retval;

  xevent = (XKeyEvent*) &(data->event);

  type = (xevent->type == KeyPress) ? DASOM_EVENT_KEY_PRESS : DASOM_EVENT_KEY_RELEASE;

  event = dasom_event_new (type);
  event->key.state = (DasomModifierType) xevent->state;
  event->key.keyval = XLookupKeysym (xevent,
                                     (!(xevent->state & ShiftMask) !=
                                      !(xevent->state & LockMask)) ? 1 : 0);
  event->key.hardware_keycode = xevent->keycode;

  DasomConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));
  retval = dasom_connection_filter_event (connection, event);
  dasom_event_free (event);

  if (retval)
    return 1;

  /* forward event */
  fw_event = g_slice_new0 (IMForwardEventStruct);
  *fw_event = *data;
  fw_event->sync_bit = 0;

  IMForwardEvent (xims, (XPointer) fw_event);

  g_slice_free (IMForwardEventStruct, fw_event);

  return 1;
}

int dasom_server_xim_reset_ic (DasomServer     *server,
                               XIMS             xims,
                               IMResetICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));
  dasom_connection_reset (connection);

  return 1;
}

static int
on_incoming_message_xim (XIMS         xims,
                         IMProtocol  *data,
                         DasomServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (xims != NULL, True);
  g_return_val_if_fail (data != NULL, True);

  if (!DASOM_IS_SERVER (server))
    g_error ("ERROR: IMUserData");

  int retval;

  DasomConnection *connection = NULL;

  if (data->major_code == XIM_CREATE_IC      ||
      data->major_code == XIM_DESTROY_IC     ||
      data->major_code == XIM_SET_IC_VALUES  ||
      data->major_code == XIM_GET_IC_VALUES  ||
      data->major_code == XIM_FORWARD_EVENT  ||
      data->major_code == XIM_SET_IC_FOCUS   ||
      data->major_code == XIM_UNSET_IC_FOCUS ||
      data->major_code == XIM_RESET_IC)
  {
    connection = g_hash_table_lookup (server->connections,
                                      GUINT_TO_POINTER (data->changeic.icid));
    if (connection)
      dasom_engine_set_english_mode (connection->engine,
                                     connection->is_english_mode);
  }

  switch (data->major_code)
  {
    case XIM_OPEN:
      g_debug (G_STRLOC ": XIM_OPEN: connect_id: %u", data->imopen.connect_id);
      retval = 1;
      break;
    case XIM_CLOSE:
      g_debug (G_STRLOC ": XIM_CLOSE: connect_id: %u",
               data->imclose.connect_id);
      retval = 1;
      break;
    case XIM_PREEDIT_START_REPLY:
      g_debug (G_STRLOC ": XIM_PREEDIT_START_REPLY");
      retval = 1;
      break;
    case XIM_CREATE_IC:
      retval = dasom_server_xim_create_ic (server, xims, &data->changeic);
      break;
    case XIM_DESTROY_IC:
      retval = dasom_server_xim_destroy_ic (server, xims, &data->destroyic);
      break;
    case XIM_SET_IC_VALUES:
      retval = dasom_server_xim_set_ic_values (server, xims, &data->changeic);
      break;
    case XIM_GET_IC_VALUES:
      retval = dasom_server_xim_get_ic_values (server, xims, &data->changeic);
      break;
    case XIM_FORWARD_EVENT:
      retval = dasom_server_xim_forward_event (server, xims, &data->forwardevent);
      break;
    case XIM_SET_IC_FOCUS:
      retval = dasom_server_xim_set_ic_focus (server, xims, &data->changefocus);
      break;
    case XIM_UNSET_IC_FOCUS:
      retval = dasom_server_xim_unset_ic_focus (server, xims, &data->changefocus);
      break;
    case XIM_RESET_IC:
      retval = dasom_server_xim_reset_ic (server, xims, &data->resetic);
      break;
    default:
      g_warning (G_STRLOC ": %s: major op code %d not handled", G_STRFUNC,
                 data->major_code);
      retval = 0;
      break;
  }

  if (connection)
    connection->is_english_mode =
      dasom_engine_get_english_mode (connection->engine);

  return retval;
}

static gboolean dasom_xevent_source_dispatch (GSource     *source,
                                              GSourceFunc  callback,
                                              gpointer     user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  Display *display = ((DasomXEventSource*) source)->display;
  XEvent   event;

  while (XPending (display))
  {
    XNextEvent (display, &event);
    if (XFilterEvent (&event, None))
      continue;
  }

  return TRUE;
}

static void dasom_xevent_source_finalize (GSource *source)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static GSourceFuncs event_funcs = {
  dasom_xevent_source_prepare,
  dasom_xevent_source_check,
  dasom_xevent_source_dispatch,
  dasom_xevent_source_finalize
};

GSource *
dasom_xevent_source_new (Display *display)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSource *source;
  DasomXEventSource *xevent_source;
  int connection_number;

  source = g_source_new (&event_funcs, sizeof (DasomXEventSource));
  xevent_source = (DasomXEventSource *) source;
  xevent_source->display = display;

  connection_number = ConnectionNumber (xevent_source->display);

  xevent_source->poll_fd.fd = connection_number;
  xevent_source->poll_fd.events = G_IO_IN;
  g_source_add_poll (source, &xevent_source->poll_fd);

  g_source_set_priority (source, G_PRIORITY_DEFAULT);
  g_source_set_can_recurse (source, FALSE);

  return source;
}

static int
on_xerror (Display *display, XErrorEvent *error)
{
  gchar err_msg[64];

  XGetErrorText (display, error->error_code, err_msg, 63);
  g_warning (G_STRLOC ": %s: XError: %s "
    "serial=%lu, error_code=%d request_code=%d minor_code=%d resourceid=%lu",
    G_STRFUNC, err_msg, error->serial, error->error_code, error->request_code,
    error->minor_code, error->resourceid);

  return 1;
}

static gboolean
dasom_server_init_xims (DasomServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  Display *display;
  Window   window;

  display = XOpenDisplay (NULL);

  if (display == NULL)
    return FALSE;

  XIMStyle ims_styles_on_spot [] = {
    XIMPreeditPosition  | XIMStatusNothing,
    XIMPreeditCallbacks | XIMStatusNothing,
    XIMPreeditNothing   | XIMStatusNothing,
    XIMPreeditPosition  | XIMStatusCallbacks,
    XIMPreeditCallbacks | XIMStatusCallbacks,
    XIMPreeditNothing   | XIMStatusCallbacks,
    0
  };

  XIMEncoding ims_encodings[] = {
      "COMPOUND_TEXT",
      NULL
  };

  XIMStyles styles;
  XIMEncodings encodings;

  styles.count_styles = sizeof (ims_styles_on_spot) / sizeof (XIMStyle) - 1;
  styles.supported_styles = ims_styles_on_spot;

  encodings.count_encodings = sizeof (ims_encodings) / sizeof (XIMEncoding) - 1;
  encodings.supported_encodings = ims_encodings;

  XSetWindowAttributes attrs;

  attrs.event_mask = KeyPressMask | KeyReleaseMask;
  attrs.override_redirect = True;

  window = XCreateWindow (display,      /* Display *display */
                          DefaultRootWindow (display),  /* Window parent */
                          0, 0,         /* int x, y */
                          1, 1,         /* unsigned int width, height */
                          0,            /* unsigned int border_width */
                          0,            /* int depth */
                          InputOutput,  /* unsigned int class */
                          CopyFromParent, /* Visual *visual */
                          CWOverrideRedirect | CWEventMask, /* unsigned long valuemask */
                          &attrs);      /* XSetWindowAttributes *attributes */

  IMOpenIM (display,
            IMModifiers,        "Xi18n",
            IMServerWindow,     window,
            IMServerName,       PACKAGE,
            IMLocale,           "C,en,ko",
            IMServerTransport,  "X/",
            IMInputStyles,      &styles,
            IMEncodingList,     &encodings,
            IMProtocolHandler,  on_incoming_message_xim,
            IMUserData,         server,
            IMFilterEventMask,  KeyPressMask | KeyReleaseMask,
            NULL);

  server->xevent_source = dasom_xevent_source_new (display);
  g_source_attach (server->xevent_source, server->main_context);
  XSetErrorHandler (on_xerror);

  return TRUE;
}

void
dasom_server_start (DasomServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_SERVER (server));

  if (server->active)
    return;

  g_assert (server->is_using_listener);
  g_socket_service_start (G_SOCKET_SERVICE (server->listener));

  if (dasom_server_init_xims (server) == FALSE)
    g_warning ("XIM server is not starded");

  server->active = TRUE;
}
