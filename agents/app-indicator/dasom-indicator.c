/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-indicator.c
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
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "dasom.h"
#include <gio/gunixsocketaddress.h>
#include <libappindicator/app-indicator.h>
#include <syslog.h>
#include "dasom-private.h"
#include <stdlib.h>

gboolean syslog_initialized = FALSE;

static void on_about_menu (GtkWidget *widget,
                           gpointer   data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (NULL,
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_INFO,
                                   GTK_BUTTONS_CLOSE,
                                   _("dasom %s"), VERSION);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Dasom Indicator"));
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

static void on_exit_menu (GtkWidget *widget,
                          gpointer   data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gtk_main_quit ();
}

/* FIXME: need DasomEngineInfo or id instead of name */
static void on_engine_changed (DasomAgent   *agent,
                               gchar        *name,
                               AppIndicator *indicator)
{
  g_debug (G_STRLOC ": %s: name: %s", G_STRFUNC, name);

  gchar *icon_name;

  if (g_strcmp0 (name, "focus-out") == 0)
    icon_name = g_strdup ("dasom-indicator");
  else
    icon_name = g_strdup_printf ("dasom-indicator-%s", name);

  app_indicator_set_icon_full (indicator, icon_name, name);

  g_free (icon_name);
}

static void on_disconnected (DasomAgent   *agent,
                             AppIndicator *indicator)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  app_indicator_set_icon_full (indicator,
                               "dasom-indicator-warning", "disconnected");
}

int
main (int argc, char **argv)
{
  GError         *error     = NULL;
  gboolean        is_no_daemon = FALSE;
  gboolean        is_debug     = FALSE;
  gboolean        is_version   = FALSE;
  GOptionContext *context;
  GOptionEntry    entries[] = {
    {"no-daemon", 0, 0, G_OPTION_ARG_NONE, &is_no_daemon, N_("Do not daemonize"), NULL},
    {"debug", 0, 0, G_OPTION_ARG_NONE, &is_debug, N_("Log debugging message"), NULL},
    {"version", 0, 0, G_OPTION_ARG_NONE, &is_version, N_("Version"), NULL},
    {NULL}
  };

  context = g_option_context_new ("- dasom daemon");
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);

  if (error != NULL)
  {
    g_warning ("%s", error->message);
    g_error_free (error);
    return EXIT_FAILURE;
  }

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, DASOM_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

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

  gtk_init (&argc, &argv);

  AppIndicator *indicator;
  GtkWidget    *menu_shell;
  GtkWidget    *about_menu;
  GtkWidget    *exit_menu;
  DasomAgent   *agent;

  menu_shell = gtk_menu_new ();
  about_menu = gtk_menu_item_new_with_label (_("About"));
  exit_menu  = gtk_menu_item_new_with_label (_("Exit"));

  gtk_widget_show_all (about_menu);
  gtk_widget_show_all (exit_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_shell), about_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_shell), exit_menu);

  gtk_widget_show_all (menu_shell);

  indicator = app_indicator_new ("dasom-indicator", "input-keyboard",
                                 APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_icon_full (indicator, "dasom-indicator", "Dasom");
  app_indicator_set_menu (indicator, GTK_MENU (menu_shell));

  g_signal_connect (about_menu, "activate",
                    G_CALLBACK (on_about_menu), indicator);
  g_signal_connect (exit_menu,  "activate",
                    G_CALLBACK (on_exit_menu),  indicator);

  agent = dasom_agent_new ();

  g_signal_connect (agent, "engine-changed",
                    G_CALLBACK (on_engine_changed), indicator);
  g_signal_connect (agent, "disconnected",
                    G_CALLBACK (on_disconnected), indicator);

  dasom_agent_connect_to_server (agent);

  gtk_main ();

  g_object_unref (agent);
  g_object_unref (indicator);

  if (syslog_initialized)
    closelog ();

  return EXIT_SUCCESS;
}
