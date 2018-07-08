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

#include "config.h"
#include <libintl.h>
#include "nimf-server.h"
#include <glib-unix.h>
#include <syslog.h>
#include "nimf-private.h"
#include <glib/gi18n.h>
#include <unistd.h>
#include <libaudit.h>
#include <gio/gunixsocketaddress.h>
#include <signal.h>
#include <sys/file.h>

gboolean syslog_initialized = FALSE;

gboolean start_indicator_service (gchar *addr)
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

gboolean
create_runtime_dir (uid_t uid)
{
  gchar   *path;
  gboolean retval = TRUE;

  path = g_strdup_printf (NIMF_RUNTIME_DIR, uid);

  if (g_mkdir_with_parents (path, 0700))
  {
    g_critical (G_STRLOC": Can't create directory: %s", path);
    retval = FALSE;
  }

  g_free (path);

  return retval;
}

int
open_lock_file (uid_t uid)
{
  gchar *path;
  int    fd;

  path = g_strdup_printf (NIMF_RUNTIME_DIR"/lock", uid);
  fd = open (path, O_RDWR | O_CREAT, 0600);

  if (fd == -1)
    g_critical ("Failed to open lock file: %s", path);

  g_free (path);

  return fd;
}

gboolean
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

int
main (int argc, char **argv)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServer *server = NULL;
  GMainLoop  *loop;
  int         fd;
  GError     *error = NULL;
  uid_t       uid;
  gboolean    retval = FALSE;

  gboolean no_daemon  = FALSE;
  gboolean is_debug   = FALSE;
  gboolean is_version = FALSE;
  gboolean start_indicator = FALSE;

  GOptionContext *context;
  GOptionEntry    entries[] = {
    {"no-daemon", 0, 0, G_OPTION_ARG_NONE, &no_daemon, N_("Do not daemonize"), NULL},
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

  if (no_daemon == FALSE)
  {
    openlog (g_get_prgname (), LOG_PID | LOG_PERROR, LOG_DAEMON);
    syslog_initialized = TRUE;
    g_log_set_default_handler ((GLogFunc) nimf_log_default_handler, &is_debug);

    if (daemon (0, 0) != 0)
    {
      g_critical ("Couldn't daemonize.");
      return EXIT_FAILURE;
    }
  }

  if ((uid = audit_getloginuid ()) == (uid_t) -1)
    uid = getuid ();

  if (!create_runtime_dir (uid))
    return EXIT_FAILURE;

  if ((fd = open_lock_file (uid)) == -1)
    return EXIT_FAILURE;

  if (flock (fd, LOCK_EX | LOCK_NB))
  {
    if (start_indicator)
    {
      gchar   *sock_path;
      gboolean retval;

      sock_path = g_strdup_printf (NIMF_RUNTIME_DIR"/socket", uid);
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

  server = nimf_server_new ();

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

  if (syslog_initialized)
    closelog ();

  if (flock (fd, LOCK_UN))
  {
    g_critical ("Failed to unlock file: "NIMF_RUNTIME_DIR"/lock", uid);
    retval = EXIT_FAILURE;
  }

  close (fd);

  gchar *file, *dir;

  file = g_strdup_printf (NIMF_RUNTIME_DIR"/lock", uid);
  dir  = g_strdup_printf (NIMF_RUNTIME_DIR, uid);

  unlink (file);
  rmdir  (dir);

  g_free (file);
  g_free (dir);

  return retval;
}
