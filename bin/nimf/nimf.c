/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf.c
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

#include <glib.h>
#include "nimf-utils.h"
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "nimf-server.h"
#include "nimf-server-private.h"
#include "nimf-service.h"
#include <glib/gi18n.h>
#include "config.h"
#include <syslog.h>
#include <errno.h>
#include <glib-unix.h>

static gchar *
nimf_get_nimf_path ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_strconcat (g_get_user_runtime_dir (), "/nimf", NULL);
}

static gboolean
create_nimf_runtime_dir ()
{
  gchar   *nimf_path;
  gboolean retval = TRUE;

  nimf_path = nimf_get_nimf_path ();

  if (g_mkdir_with_parents (nimf_path, 0700))
  {
    g_critical (G_STRLOC": Can't create directory: %s", nimf_path);
    retval = FALSE;
  }

  g_free (nimf_path);

  return retval;
}

static gchar *
nimf_get_lock_path ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_strconcat (g_get_user_runtime_dir (), "/nimf/lock.pid", NULL);
}

static int
open_lock_file ()
{
  gchar *path;
  int    fd;

  path = nimf_get_lock_path ();
  fd = g_open (path, O_RDWR | O_CREAT, 0600);

  if (fd == -1)
    g_critical ("Failed to open lock file: %s", path);

  g_free (path);

  return fd;
}

static int
set_lock (int fd, int type)
{
  struct flock lock;
  lock.l_type   = type;
  lock.l_start  = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len    = 0;

  return fcntl (fd, F_SETLK, &lock);
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

static void unlink_socket_file ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar *path;
  path = nimf_get_socket_path ();
  g_unlink (path);
  g_free (path);
}

static void
nimf_log_default_handler (const gchar    *log_domain,
                          GLogLevelFlags  log_level,
                          const gchar    *message,
                          gboolean       *debug)
{
  int priority;
  const gchar *prefix;

  switch (log_level & G_LOG_LEVEL_MASK)
  {
    case G_LOG_LEVEL_ERROR:
      priority = LOG_ERR;
      prefix = "ERROR **";
      break;
    case G_LOG_LEVEL_CRITICAL:
      priority = LOG_CRIT;
      prefix = "CRITICAL **";
      break;
    case G_LOG_LEVEL_WARNING:
      priority = LOG_WARNING;
      prefix = "WARNING **";
      break;
    case G_LOG_LEVEL_MESSAGE:
      priority = LOG_NOTICE;
      prefix = "Message";
      break;
    case G_LOG_LEVEL_INFO:
      priority = LOG_INFO;
      prefix = "INFO";
      break;
    case G_LOG_LEVEL_DEBUG:
      priority = LOG_DEBUG;
      prefix = "DEBUG";
      break;
    default:
      priority = LOG_NOTICE;
      prefix = "LOG";
      break;
  }

  if (priority == LOG_DEBUG && (debug == NULL || *debug == FALSE))
    return;

  syslog (priority, "%s-%s: %s", log_domain, prefix, message ? message : "(NULL) message");
}

/**
 * SECTION:nimf
 * @title: nimf
 * @section_id: nimf
 */

/**
 * PROGRAM:nimf
 * @short_description: Nimf input method daemon
 * @synopsis: nimf [*OPTIONS*...]
 * @see_also: nimf-settings(1)
 * @-h, --help: Show help messages
 * @--debug: Log debugging messages
 * @--version: Show version
 *
 * nimf is an input method daemon.
 *
 * Returns: 0 on success, non-zero on failure
 */
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

  GOptionContext *context;
  GOptionEntry    entries[] = {
    {"debug",   0, 0, G_OPTION_ARG_NONE, &is_debug,   N_("Log debugging message"), NULL},
    {"version", 0, 0, G_OPTION_ARG_NONE, &is_version, N_("Version"), NULL},
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

  if (set_lock (fd, F_WRLCK))
  {
    if (errno == EACCES || errno == EAGAIN)
    {
      close(fd);
      g_message (G_STRLOC ": %s: Another instance appears to be running: %s",
                 G_STRFUNC, strerror (errno));
      return EXIT_FAILURE;
    }
    else
    {
      g_warning (G_STRLOC ": %s: %s", G_STRFUNC, strerror (errno));

      return EXIT_FAILURE;
    }
  }

  if (!write_pid (fd))
  {
    g_critical ("Can't write pid");
    goto finally;
  }

  unlink_socket_file ();

  server = g_object_new (NIMF_TYPE_SERVER, NULL);

  if (!nimf_server_start (server))
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

  if (set_lock (fd, F_UNLCK))
  {
    g_critical ("Failed to unlock file: %s", nimf_get_lock_path ());
    retval = EXIT_FAILURE;
  }

  close (fd);

  gchar *lock_path, *nimf_path;

  lock_path = nimf_get_lock_path ();
  nimf_path = nimf_get_nimf_path ();

  unlink_socket_file ();
  g_unlink (lock_path);
  g_rmdir  (nimf_path);

  g_free (lock_path);
  g_free (nimf_path);

  return retval;
}
