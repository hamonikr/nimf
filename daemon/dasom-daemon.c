/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-daemon.c
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
#include <libintl.h>
#include "dasom-server.h"
#include <glib-unix.h>
#include <syslog.h>
#include "dasom-private.h"
#include <glib/gi18n.h>

gboolean syslog_initialized = FALSE;

int
main (int argc, char **argv)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomServer *server;
  GMainLoop   *loop;
  GError      *error = NULL;

  gboolean is_no_daemon = FALSE;
  gboolean is_debug     = FALSE;
  gboolean is_version   = FALSE;

  GOptionContext *context;
  GOptionEntry    entries[] = {
    {"no-daemon", 0, 0, G_OPTION_ARG_NONE, &is_no_daemon, N_("Do not daemonize"), NULL},
    {"debug", 0, 0, G_OPTION_ARG_NONE, &is_debug, N_("Log debugging message"), NULL},
    {"version", 0, 0, G_OPTION_ARG_NONE, &is_version, N_("Version"), NULL},
    {NULL}
  };

  context = g_option_context_new ("- Dasom Input Method Daemon");
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);

  if (error != NULL)
  {
    g_warning ("%s", error->message);
    g_error_free (error);
    return EXIT_FAILURE;
  }

#if ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, DASOM_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  if (is_debug)
    g_setenv ("G_MESSAGES_DEBUG", "dasom", TRUE);

  if (is_version)
  {
    g_print ("%s %s\n", argv[0], VERSION);
    exit (EXIT_SUCCESS);
  }

  if (is_no_daemon == FALSE)
  {
    openlog (g_get_prgname (), LOG_PID | LOG_PERROR, LOG_DAEMON);
    syslog_initialized = TRUE;
    g_log_set_default_handler ((GLogFunc) dasom_log_default_handler, &is_debug);

    if (daemon (0, 0) != 0)
    {
      g_critical ("Couldn't daemonize.");
      return EXIT_FAILURE;
    }
  }

  server = dasom_server_new (DASOM_ADDRESS, &error);

  if (server == NULL)
  {
    g_critical ("%s", error->message);
    g_clear_error (&error);

    return EXIT_FAILURE;
  }

  dasom_server_start (server);

  loop = g_main_loop_new (NULL, FALSE);

  g_unix_signal_add (SIGINT,  (GSourceFunc) g_main_loop_quit, loop);
  g_unix_signal_add (SIGTERM, (GSourceFunc) g_main_loop_quit, loop);

  g_main_loop_run (loop);

  g_main_loop_unref (loop);
  g_object_unref (server);

  if (syslog_initialized)
    closelog ();

  return EXIT_SUCCESS;
}
