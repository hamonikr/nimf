/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2018 Hodong Kim <cogniti@gmail.com>
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

#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>
#include "nimf-message.h"
#include "nimf-private.h"
#include <glib/gstdio.h>
#include <fcntl.h>
#include <string.h>
#include "nimf-server-im.h"
#include "nimf-service.h"
#include "nimf-module.h"
#include <glib/gi18n.h>
#include "config.h"
#include <stdlib.h>
#include <syslog.h>
#include <sys/file.h>
#include <glib-unix.h>

static gboolean
start_indicator_service (gchar *addr)
{
  GSocketAddress *address;
  GSocket        *socket;
  gboolean        retval = FALSE;

  address = g_unix_socket_address_new_with_type (addr, -1,
                                                 G_UNIX_SOCKET_ADDRESS_PATH);

  socket = g_socket_new (G_SOCKET_FAMILY_UNIX,
                         G_SOCKET_TYPE_STREAM,
                         G_SOCKET_PROTOCOL_DEFAULT,
                         NULL);

  if (g_socket_connect (socket, address, NULL, NULL))
  {
    NimfMessage *message;

    if (socket && !g_socket_is_closed (socket))
      nimf_send_message (socket, 0, NIMF_MESSAGE_START_INDICATOR,
                         NULL, 0, NULL);

    if ((message = nimf_recv_message (socket)))
    {
      retval = *(gboolean *) message->data;
      nimf_message_unref (message);
    }
  }

  g_object_unref (socket);
  g_object_unref (address);

  return retval;
}

static gboolean
create_nimf_runtime_dir ()
{
  gchar   *runtime_dir;
  gboolean retval = TRUE;

  runtime_dir = g_strconcat (g_get_user_runtime_dir (), "/nimf", NULL);

  if (g_mkdir_with_parents (runtime_dir, 0700))
  {
    g_critical (G_STRLOC": Can't create directory: %s", runtime_dir);
    retval = FALSE;
  }

  g_free (runtime_dir);

  return retval;
}

static int
open_lock_file ()
{
  gchar *path;
  int    fd;

  path = g_strconcat (g_get_user_runtime_dir (), "/nimf/lock", NULL);
  fd = g_open (path, O_RDWR | O_CREAT, 0600);

  if (fd == -1)
    g_critical ("Failed to open lock file: %s", path);

  g_free (path);

  return fd;
}

static gboolean
write_pid (int fd)
{
  gchar  *pid;
  ssize_t len;
  ssize_t written;

  if (ftruncate (fd, 0))
    return FALSE;

  pid = g_strdup_printf ("%ld", (long int) getpid ());

  len = strlen (pid) + 1;
  written = write (fd, pid, len);

  g_free (pid);

  if (written != len)
    return FALSE;

  return TRUE;
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
      im = nimf_server_im_new (connection, connection->server);
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
    case NIMF_MESSAGE_START_INDICATOR:
      {
        NimfService *service;
        gboolean     retval = FALSE;

        service = g_hash_table_lookup (connection->server->services,
                                       "nimf-indicator");
        if (!nimf_service_is_active (service))
          retval = nimf_service_start (service);

        nimf_send_message (socket, icid, NIMF_MESSAGE_START_INDICATOR_REPLY,
                           &retval, sizeof (gboolean), NULL);
      }
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
  g_source_attach (connection->source, NULL);

  return TRUE;
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
  g_hash_table_insert (server->services,
                       g_strdup (nimf_service_get_id (service)), service);

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

static gboolean
nimf_server_start (NimfServer *server,
                   gboolean    start_indicator)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_SERVER (server), FALSE);

  if (server->active)
    return TRUE;

  nimf_server_load_services (server);

  server->candidatable = g_hash_table_lookup (server->services, "nimf-candidate");
  server->preeditable  = g_hash_table_lookup (server->services, "nimf-preedit-window");
  nimf_service_start (NIMF_SERVICE (server->candidatable));
  nimf_service_start (NIMF_SERVICE (server->preeditable));

  nimf_server_load_engines  (server);

  GHashTableIter iter;
  gpointer       service;

  g_hash_table_iter_init (&iter, server->services);

  while (g_hash_table_iter_next (&iter, NULL, &service))
  {
    if (!g_strcmp0 (nimf_service_get_id (NIMF_SERVICE (service)), "nimf-indicator") && !start_indicator)
      continue;
    else if (!g_strcmp0 (nimf_service_get_id (NIMF_SERVICE (service)), "nimf-candidate"))
      continue;
    else if (!g_strcmp0 (nimf_service_get_id (NIMF_SERVICE (service)), "nimf-preedit-window"))
      continue;

    if (!nimf_service_start (NIMF_SERVICE (service)))
      g_hash_table_iter_remove (&iter);
  }

  GSocketAddress *address;
  GError         *error = NULL;

  server->path = nimf_get_socket_path ();
  server->service = g_socket_service_new ();

  if (g_unix_socket_address_abstract_names_supported ())
    address = g_unix_socket_address_new_with_type (server->path, -1,
                                                   G_UNIX_SOCKET_ADDRESS_PATH);
  else
  {
    g_critical ("Abstract UNIX domain socket names are not supported.");
    return FALSE;
  }

  g_socket_listener_add_address (G_SOCKET_LISTENER (server->service), address,
                                 G_SOCKET_TYPE_STREAM,
                                 G_SOCKET_PROTOCOL_DEFAULT,
                                 NULL, NULL, &error);
  g_object_unref (address);

  if (error)
  {
    g_critical ("%s", error->message);
    g_clear_error (&error);

    return FALSE;
  }

  g_chmod (server->path, 0700);

  g_signal_connect (server->service, "incoming",
                    G_CALLBACK (on_new_connection), server);

  g_socket_service_start (server->service);

  server->active = TRUE;

  return TRUE;
}

int
main (int argc, char **argv)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServer *server = NULL;
  GMainLoop  *loop;
  int         fd;
  GError     *error  = NULL;
  gboolean    retval = FALSE;

  gboolean is_debug   = FALSE;
  gboolean is_version = FALSE;
  gboolean start_indicator = FALSE;

  GOptionContext *context;
  GOptionEntry    entries[] = {
    {"debug", 0, 0, G_OPTION_ARG_NONE, &is_debug, N_("Log debugging message"), NULL},
    {"version", 0, 0, G_OPTION_ARG_NONE, &is_version, N_("Version"), NULL},
    {"start-indicator", 0, 0, G_OPTION_ARG_NONE, &start_indicator, N_("Start indicator"), NULL},
    {NULL}
  };

  context = g_option_context_new ("- Nimf Input Method Server");
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);

  if (error != NULL)
  {
    g_warning ("%s", error->message);
    g_error_free (error);
    return EXIT_FAILURE;
  }

  g_setenv ("GTK_IM_MODULE", "gtk-im-context-simple", TRUE);
  g_setenv ("GDK_BACKEND", "x11", TRUE);

#if ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, NIMF_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  if (is_debug)
    g_setenv ("G_MESSAGES_DEBUG", "nimf", TRUE);

  if (is_version)
  {
    g_print ("%s %s\n", argv[0], VERSION);
    return EXIT_SUCCESS;
  }

  openlog (g_get_prgname (), LOG_PID | LOG_PERROR, LOG_DAEMON);
  g_log_set_default_handler ((GLogFunc) nimf_log_default_handler, &is_debug);

  if (daemon (0, 0) != 0)
  {
    g_critical ("Couldn't daemonize.");
    return EXIT_FAILURE;
  }

  if (!create_nimf_runtime_dir ())
    return EXIT_FAILURE;

  if ((fd = open_lock_file ()) == -1)
    return EXIT_FAILURE;

  if (flock (fd, LOCK_EX | LOCK_NB))
  {
    if (start_indicator)
    {
      gchar   *sock_path;
      gboolean retval;

      sock_path = nimf_get_socket_path ();
      retval = start_indicator_service (sock_path);

      g_free (sock_path);

      if (retval)
        return EXIT_SUCCESS;
    }

    g_message ("Another instance appears to be running.");
    return EXIT_FAILURE;
  }

  if (!write_pid (fd))
  {
    g_critical ("Can't write pid");
    goto finally;
  }

  server = g_object_new (NIMF_TYPE_SERVER, NULL);

  if (!nimf_server_start (server, start_indicator))
  {
    retval = EXIT_FAILURE;
    goto finally;
  }

  loop = g_main_loop_new (NULL, FALSE);

  g_unix_signal_add (SIGINT,  (GSourceFunc) g_main_loop_quit, loop);
  g_unix_signal_add (SIGTERM, (GSourceFunc) g_main_loop_quit, loop);
  signal (SIGTSTP, SIG_IGN);

  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  retval = EXIT_SUCCESS;

  finally:

  if (server)
    g_object_unref (server);

  closelog ();

  if (flock (fd, LOCK_UN))
  {
    g_critical ("Failed to unlock file: %s/nimf/lock", g_get_user_runtime_dir ());
    retval = EXIT_FAILURE;
  }

  close (fd);

  gchar *file, *dir;

  file = g_strconcat (g_get_user_runtime_dir (), "/nimf/lock", NULL);
  dir  = g_strconcat (g_get_user_runtime_dir (), "/nimf", NULL);

  g_unlink (file);
  g_rmdir  (dir);

  g_free (file);
  g_free (dir);

  return retval;
}
