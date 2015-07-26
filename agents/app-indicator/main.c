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

static void on_clicked (GtkWidget *widget,
                        gpointer   data )
{
  g_print ("클릭됨\n");
}

static gboolean on_delete_event (GtkWidget *widget,
                                 GdkEvent  *event,
                                 gpointer   data )
{
  g_print ("delete event\n");

  return FALSE;
}

static void on_destroy (GtkWidget *widget,
                        gpointer   data )
{
  gtk_main_quit ();
}

static void on_engine_changed (DasomAgent *agent,
                               gchar      *name,
                               gpointer    data )
{
  g_print ("engine changed: %s\n", name);
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

  GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  GtkWidget *button = gtk_button_new_with_label ("Dasom");

  g_signal_connect (window, "delete-event",
                    G_CALLBACK (on_delete_event), NULL);
  g_signal_connect (window, "destroy",
                    G_CALLBACK (on_destroy), NULL);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (on_clicked), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 10);
  gtk_container_add (GTK_CONTAINER (window), button);

  gtk_widget_show (button);
  gtk_widget_show (window);

  DasomAgent *agent;
  agent = dasom_agent_new ();
  g_signal_connect (agent, "engine-changed", G_CALLBACK (on_engine_changed), NULL);

  gtk_main ();

  return 0;
}
