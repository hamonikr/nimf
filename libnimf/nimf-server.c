/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-server.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2017 Hodong Kim <cogniti@gmail.com>
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

#include "config.h"
#include "nimf-server.h"
#include "nimf-private.h"
#include "nimf-marshalers.h"
#include "nimf-module.h"
#include "nimf-service.h"
#include "nimf-key-syms.h"
#include "nimf-candidate.h"
#include "nimf-types.h"
#include "nimf-service-im.h"
#include "nimf-server-im.h"
#include <gio/gunixsocketaddress.h>
#include <string.h>

enum
{
  PROP_0,
  PROP_ADDRESS,
};

enum {
  ENGINE_CHANGED,
  LAST_SIGNAL
};

static guint nimf_server_signals[LAST_SIGNAL] = { 0 };

static gboolean
on_incoming_message_nimf (GSocket        *socket,
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

    GList *l;
    for (l = connection->server->instances; l != NULL; l = l->next)
      nimf_engine_reset (l->data, NULL);

    connection->result->reply = NULL;
    g_hash_table_remove (connection->server->connections,
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

  NimfServerIM *im;
  guint16       icid = message->header->icid;

  im = g_hash_table_lookup (connection->ims, GUINT_TO_POINTER (icid));

  switch (message->header->type)
  {
    case NIMF_MESSAGE_CREATE_CONTEXT:
      im = nimf_server_im_new (connection, connection->server, NULL);
      NIMF_SERVICE_IM (im)->icid = icid;
      g_hash_table_insert (connection->ims, GUINT_TO_POINTER (icid), im);

      nimf_send_message (socket, icid, NIMF_MESSAGE_CREATE_CONTEXT_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_DESTROY_CONTEXT:
      g_hash_table_remove (connection->ims, GUINT_TO_POINTER (icid));
      nimf_send_message (socket, icid, NIMF_MESSAGE_DESTROY_CONTEXT_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_FILTER_EVENT:
      nimf_message_ref (message);
      retval = nimf_service_im_filter_event (NIMF_SERVICE_IM (im), (NimfEvent *) message->data);
      nimf_message_unref (message);
      nimf_send_message (socket, icid, NIMF_MESSAGE_FILTER_EVENT_REPLY,
                         &retval, sizeof (gboolean), NULL);
      break;
    case NIMF_MESSAGE_RESET:
      nimf_service_im_reset (NIMF_SERVICE_IM (im));
      nimf_send_message (socket, icid, NIMF_MESSAGE_RESET_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_FOCUS_IN:
      nimf_service_im_focus_in (NIMF_SERVICE_IM (im));
      nimf_send_message (socket, icid, NIMF_MESSAGE_FOCUS_IN_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_FOCUS_OUT:
      nimf_service_im_focus_out (NIMF_SERVICE_IM (im));
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

        nimf_service_im_set_surrounding (NIMF_SERVICE_IM (im), data, str_len, cursor_index);
        nimf_message_unref (message);
        nimf_send_message (socket, icid,
                           NIMF_MESSAGE_SET_SURROUNDING_REPLY, NULL, 0, NULL);
      }
      break;
    case NIMF_MESSAGE_GET_SURROUNDING:
      {
        gchar *data;
        gint   cursor_index;
        gint   str_len = 0;

        retval = nimf_service_im_get_surrounding (NIMF_SERVICE_IM (im), &data, &cursor_index);
        str_len = strlen (data);
        data = g_realloc (data, str_len + 1 + sizeof (gint) + sizeof (gboolean));
        *(gint *) (data + str_len + 1) = cursor_index;
        *(gboolean *) (data + str_len + 1 + sizeof (gint)) = retval;

        nimf_send_message (socket, icid,
                           NIMF_MESSAGE_GET_SURROUNDING_REPLY, data,
                           str_len + 1 + sizeof (gint) + sizeof (gboolean),
                           NULL);
        g_free (data);
      }
      break;
    case NIMF_MESSAGE_SET_CURSOR_LOCATION:
      nimf_message_ref (message);
      nimf_service_im_set_cursor_location (NIMF_SERVICE_IM (im), (NimfRectangle *) message->data);
      nimf_message_unref (message);
      nimf_send_message (socket, icid, NIMF_MESSAGE_SET_CURSOR_LOCATION_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_SET_USE_PREEDIT:
      nimf_message_ref (message);
      nimf_service_im_set_use_preedit (NIMF_SERVICE_IM (im), *(gboolean *) message->data);
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
      break;
    default:
      g_warning ("Unknown message type: %d", message->header->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

static guint16
nimf_server_add_connection (NimfServer     *server,
                            NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

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
                   NimfServer        *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfConnection *connection;
  connection = nimf_connection_new ();
  connection->socket = g_socket_connection_get_socket (socket_connection);
  nimf_server_add_connection (server, connection);

  connection->source = g_socket_create_source (connection->socket, G_IO_IN, NULL);
  connection->socket_connection = g_object_ref (socket_connection);
  g_source_set_can_recurse (connection->source, TRUE);
  g_source_set_callback (connection->source,
                         (GSourceFunc) on_incoming_message_nimf,
                         connection, NULL);
  g_source_attach (connection->source, server->main_context);

  return TRUE;
}

static gboolean
nimf_server_initable_init (GInitable     *initable,
                           GCancellable  *cancellable,
                           GError       **error)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServer     *server = NIMF_SERVER (initable);
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
  return TRUE;
}

static void
nimf_server_initable_iface_init (GInitableIface *initable_iface)
{
  initable_iface->init = nimf_server_initable_init;
}

G_DEFINE_TYPE_WITH_CODE (NimfServer, nimf_server, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                nimf_server_initable_iface_init));

static gint
on_comparing_engine_with_id (NimfEngine *engine, const gchar *id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_strcmp0 (nimf_engine_get_id (engine), id);
}

NimfEngine *
nimf_server_get_instance (NimfServer  *server,
                          const gchar *id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  list = g_list_find_custom (g_list_first (server->instances), id,
                             (GCompareFunc) on_comparing_engine_with_id);
  if (list)
    return list->data;

  return NULL;
}

NimfEngine *
nimf_server_get_next_instance (NimfServer *server, NimfEngine *engine)
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

NimfEngine *
nimf_server_get_default_engine (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings  *settings;
  gchar      *engine_id;
  NimfEngine *engine;

  settings  = g_settings_new ("org.nimf.engines");
  engine_id = g_settings_get_string (settings, "default-engine");
  engine    = nimf_server_get_instance (server, engine_id);

  if (G_UNLIKELY (engine == NULL))
  {
    g_settings_reset (settings, "default-engine");
    g_free (engine_id);
    engine_id = g_settings_get_string (settings, "default-engine");
    engine = nimf_server_get_instance (server, engine_id);
  }

  g_free (engine_id);
  g_object_unref (settings);

  return engine;
}

static void
on_changed_trigger_keys (GSettings  *settings,
                         gchar      *key,
                         NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GHashTableIter iter;
  gpointer       engine_id;
  gpointer       gsettings;

  g_hash_table_remove_all (server->trigger_keys);

  g_hash_table_iter_init (&iter, server->trigger_gsettings);

  while (g_hash_table_iter_next (&iter, &engine_id, &gsettings))
  {
    NimfKey **trigger_keys;
    gchar   **strv;

    strv = g_settings_get_strv (gsettings, "trigger-keys");
    trigger_keys = nimf_key_newv ((const gchar **) strv);
    g_hash_table_insert (server->trigger_keys,
                         trigger_keys, g_strdup (engine_id));
    g_strfreev (strv);
  }
}

static void
on_changed_hotkeys (GSettings  *settings,
                    gchar      *key,
                    NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **keys = g_settings_get_strv (settings, key);

  nimf_key_freev (server->hotkeys);
  server->hotkeys = nimf_key_newv ((const gchar **) keys);

  g_strfreev (keys);
}

static void
on_use_singleton (GSettings  *settings,
                  gchar      *key,
                  NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  server->use_singleton = g_settings_get_boolean (server->settings,
                                                  "use-singleton");
}

 static void

nimf_server_load_service (NimfServer  *server,
                          const gchar *path)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfModule  *module;
  NimfService *service;

  module = nimf_module_new (path);

  if (!g_type_module_use (G_TYPE_MODULE (module)))
  {
    g_warning (G_STRLOC ":" "Failed to load module: %s", path);
    g_object_unref (module);
    return;
  }

  service = g_object_new (module->type, "server", server, NULL);
  g_hash_table_insert (server->services, g_strdup (path), service);

  g_type_module_unuse (G_TYPE_MODULE (module));
}

static void
nimf_server_load_services (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GDir        *dir;
  GError      *error = NULL;
  const gchar *filename;
  gchar       *path;

  dir = g_dir_open (NIMF_SERVICE_MODULE_DIR, 0, &error);

  if (error)
  {
    g_warning (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
    g_clear_error (&error);
    return;
  }

  while ((filename = g_dir_read_name (dir)))
  {
    path = g_build_path (G_DIR_SEPARATOR_S, NIMF_SERVICE_MODULE_DIR, filename, NULL);
    nimf_server_load_service (server, path);
    g_free (path);
  }

  g_dir_close (dir);
}

static void
nimf_server_load_engines (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettingsSchemaSource  *source; /* do not free */
  gchar                 **schema_ids;
  gint                    i;

  source = g_settings_schema_source_get_default ();
  g_settings_schema_source_list_schemas (source, TRUE, &schema_ids, NULL);

  for (i = 0; schema_ids[i] != NULL; i++)
  {
    if (g_str_has_prefix (schema_ids[i], "org.nimf.engines."))
    {
      GSettingsSchema *schema;
      GSettings       *settings;
      const gchar     *engine_id;
      gboolean         active = TRUE;

      engine_id = schema_ids[i] + strlen ("org.nimf.engines.");
      schema = g_settings_schema_source_lookup (source, schema_ids[i], TRUE);
      settings = g_settings_new (schema_ids[i]);

      if (g_settings_schema_has_key (schema, "active"))
        active = g_settings_get_boolean (settings, "active");

      if (active)
      {
        NimfModule *module;
        NimfEngine *engine;
        gchar      *path;

        path = g_module_build_path (NIMF_MODULE_DIR, engine_id);
        module = nimf_module_new (path);

        if (!g_type_module_use (G_TYPE_MODULE (module)))
        {
          g_warning (G_STRLOC ": Failed to load module: %s", path);

          g_object_unref (module);
          g_free (path);
          g_object_unref (settings);
          g_settings_schema_unref (schema);

          continue;
        }

        g_hash_table_insert (server->modules, g_strdup (path), module);
        engine = g_object_new (module->type, "server", server, NULL);
        server->instances = g_list_prepend (server->instances, engine);
        g_type_module_unuse (G_TYPE_MODULE (module));

        if (g_settings_schema_has_key (schema, "trigger-keys"))
        {
          NimfKey **trigger_keys;
          gchar   **strv;

          strv = g_settings_get_strv (settings, "trigger-keys");
          trigger_keys = nimf_key_newv ((const gchar **) strv);
          g_hash_table_insert (server->trigger_keys,
                               trigger_keys, g_strdup (engine_id));
          g_hash_table_insert (server->trigger_gsettings,
                               g_strdup (engine_id), settings);
          g_signal_connect (settings, "changed::trigger-keys",
                            G_CALLBACK (on_changed_trigger_keys), server);
          g_strfreev (strv);
        }

        g_free (path);
      }

      g_settings_schema_unref (schema);
    }
  }

  g_strfreev (schema_ids);
}

static void
nimf_server_init (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  server->settings = g_settings_new ("org.nimf");
  server->use_singleton = g_settings_get_boolean (server->settings,
                                                  "use-singleton");
  server->trigger_gsettings = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                     g_free, g_object_unref);
  server->trigger_keys = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                                (GDestroyNotify) nimf_key_freev,
                                                g_free);
  gchar **hotkeys = g_settings_get_strv (server->settings, "hotkeys");
  server->hotkeys = nimf_key_newv ((const gchar **) hotkeys);
  g_strfreev (hotkeys);

  g_signal_connect (server->settings, "changed::hotkeys",
                    G_CALLBACK (on_changed_hotkeys), server);
  g_signal_connect (server->settings, "changed::use-singleton",
                    G_CALLBACK (on_use_singleton), server);

  server->candidate = nimf_candidate_new ();
  server->modules   = g_hash_table_new_full (g_str_hash, g_str_equal,
                                             g_free, NULL);
  server->services  = g_hash_table_new_full (g_str_hash, g_str_equal,
                                             g_free, g_object_unref);
  nimf_server_load_engines  (server);
  nimf_server_load_services (server);
  server->main_context = g_main_context_ref_thread_default ();
  server->connections = g_hash_table_new_full (g_direct_hash,
                                               g_direct_equal,
                                               NULL,
                                               (GDestroyNotify) g_object_unref);
}

void
nimf_server_stop (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_SERVER (server));

  if (!server->active)
    return;

  g_assert (server->is_using_listener);
  g_assert (server->run_signal_handler_id > 0);

  g_signal_handler_disconnect (server->listener, server->run_signal_handler_id);
  server->run_signal_handler_id = 0;

  GHashTableIter iter;
  gpointer       service;

  g_hash_table_iter_init (&iter, server->services);

  while (g_hash_table_iter_next (&iter, NULL, &service))
    nimf_service_stop (NIMF_SERVICE (service));

  g_socket_service_stop (G_SOCKET_SERVICE (server->listener));
  server->active = FALSE;
}

static void
nimf_server_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServer *server = NIMF_SERVER (object);

  if (server->run_signal_handler_id > 0)
    g_signal_handler_disconnect (server->listener, server->run_signal_handler_id);

  if (server->listener != NULL)
    g_object_unref (server->listener);

  g_hash_table_unref (server->modules);
  g_hash_table_unref (server->services);

  if (server->instances)
  {
    g_list_free_full (server->instances, g_object_unref);
    server->instances = NULL;
  }

  g_object_unref (server->candidate);
  g_hash_table_unref (server->connections);
  g_object_unref (server->settings);
  g_hash_table_unref (server->trigger_gsettings);
  g_hash_table_unref (server->trigger_keys);
  nimf_key_freev (server->hotkeys);
  g_free (server->address);

  g_main_context_unref (server->main_context);

  G_OBJECT_CLASS (nimf_server_parent_class)->finalize (object);
}

static void
nimf_server_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  NimfServer *server = NIMF_SERVER (object);

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
nimf_server_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  NimfServer *server = NIMF_SERVER (object);

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
nimf_server_class_init (NimfServerClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize     = nimf_server_finalize;
  object_class->set_property = nimf_server_set_property;
  object_class->get_property = nimf_server_get_property;

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
  nimf_server_signals[ENGINE_CHANGED] =
    g_signal_new (g_intern_static_string ("engine-changed"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfServerClass, engine_changed),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

NimfServer *
nimf_server_new (const gchar  *address,
                 GError      **error)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (address != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return g_initable_new (NIMF_TYPE_SERVER, NULL, error,
                         "address", address, NULL);
}

void
nimf_server_start (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_SERVER (server));

  if (server->active)
    return;

  g_assert (server->is_using_listener);
  g_socket_service_start (G_SOCKET_SERVICE (server->listener));

  GHashTableIter iter;
  gpointer       service;

  g_hash_table_iter_init (&iter, server->services);

  while (g_hash_table_iter_next (&iter, NULL, &service))
    nimf_service_start (NIMF_SERVICE (service));

  server->active = TRUE;
}

void nimf_server_set_engine_by_id (NimfServer  *server,
                                   const gchar *id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GHashTableIter iter;
  gpointer       conn;
  gpointer       service;

  g_hash_table_iter_init (&iter, server->connections);

  while (g_hash_table_iter_next (&iter, NULL, &conn))
    nimf_connection_set_engine_by_id (NIMF_CONNECTION (conn), id);

  g_hash_table_iter_init (&iter, server->services);

  while (g_hash_table_iter_next (&iter, NULL, &service))
    nimf_service_set_engine_by_id (service, id);
}

gchar **nimf_server_get_loaded_engine_ids (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **engine_ids;
  gint    i;
  GList  *list;
  const gchar *id;

  engine_ids = g_malloc0_n (1, sizeof (gchar *));

  for (list = g_list_first (server->instances), i = 0;
       list != NULL;
       list = list->next, i++)
  {
    id = nimf_engine_get_id (list->data);
    engine_ids[i] = g_strdup (id);
    engine_ids = g_realloc_n (engine_ids, sizeof (gchar *), i + 2);
    engine_ids[i + 1] = NULL;
  }

  return engine_ids;
}
