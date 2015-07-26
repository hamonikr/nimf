/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * main.c
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

static void on_about (GtkWidget *widget,
                      gpointer   data)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (NULL,
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_INFO,
                                   GTK_BUTTONS_CLOSE,
                                   "Dasom Indicator for Dasom Input Method");

  g_signal_connect (dialog, "response",
                    G_CALLBACK (gtk_widget_destroy), NULL);

  gtk_widget_show (dialog);
}

static void on_exit (GtkWidget *widget,
                      gpointer   data)
{
  g_print ("on_exit\n");

  gtk_main_quit ();
}

/* TODO: name 대신에 DasomModuleInfo, DasomStatusInfo
 *       이런 걸 받아올 필요가 있습니다. */
static void on_engine_changed (DasomAgent   *agent,
                               gchar        *name,
                               AppIndicator *indicator)
{
  g_debug (G_STRLOC ": %s: name: %s", G_STRFUNC, name);

  if (g_strcmp0 (name, "en") == 0)
  {
    app_indicator_set_icon_full (indicator, "dasom-en", "english");
    app_indicator_set_attention_icon (indicator, "dasom-en");
    app_indicator_set_label  (indicator, name, name);
  }
  else if (g_strcmp0 (name, "정") == 0)
  {
    app_indicator_set_icon_full (indicator, "dasom-ko", "korean");
    app_indicator_set_attention_icon (indicator, "dasom-ko");
    app_indicator_set_label  (indicator, name, name);
  }
  else
  {
    app_indicator_set_icon_full (indicator, "input-keyboard", "Dasom");
    app_indicator_set_attention_icon (indicator, "input-keyboard");
    app_indicator_set_label  (indicator, name, name);
  }
}

int
main (int argc, char **argv)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, DASOM_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gtk_init (&argc, &argv);

  AppIndicator *indicator;
  GtkWidget    *menu_shell;
  GtkWidget    *about_menu;
  GtkWidget    *exit_menu;
  DasomAgent   *agent;

  menu_shell = gtk_menu_new();
  about_menu = gtk_menu_item_new_with_label(_("About"));
  exit_menu  = gtk_menu_item_new_with_label(_("Exit"));

  gtk_widget_show_all (about_menu);
  gtk_widget_show_all (exit_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_shell), about_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_shell), exit_menu);

  gtk_widget_show_all (menu_shell);

  indicator = app_indicator_new ("dasom-indicator",
                                 "input-keyboard",
                                 APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_label  (indicator, "Dasom",
                            "Dasom Indicator for Dasom IM");
  app_indicator_set_menu   (indicator, GTK_MENU (menu_shell));

  agent = dasom_agent_new ();
  g_signal_connect (agent, "engine-changed",
                    G_CALLBACK (on_engine_changed), indicator);
  g_signal_connect (about_menu, "activate",  G_CALLBACK (on_about), indicator);
  g_signal_connect (exit_menu,  "activate",  G_CALLBACK (on_exit),  indicator);

  gtk_main ();

  g_object_unref (agent);
  g_object_unref (indicator);

  return 0;
}
