/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-indicator.c
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
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "nimf.h"
#include <libappindicator/app-indicator.h>

#define NIMF_TYPE_INDICATOR             (nimf_indicator_get_type ())
#define NIMF_INDICATOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_INDICATOR, NimfIndicator))
#define NIMF_INDICATOR_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_INDICATOR, NimfIndicatorClass))
#define NIMF_IS_INDICATOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_INDICATOR))
#define NIMF_IS_INDICATOR_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_INDICATOR))
#define NIMF_INDICATOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_INDICATOR, NimfIndicatorClass))

typedef struct _NimfIndicator      NimfIndicator;
typedef struct _NimfIndicatorClass NimfIndicatorClass;

struct _NimfIndicatorClass
{
  NimfServiceClass parent_class;
};

struct _NimfIndicator
{
  NimfService parent_instance;

  gchar        *id;
  AppIndicator *appindicator;
  const gchar  *engine_id;
};

GType nimf_indicator_get_type (void) G_GNUC_CONST;

G_DEFINE_DYNAMIC_TYPE (NimfIndicator, nimf_indicator, NIMF_TYPE_SERVICE);

static void on_engine_menu (GtkWidget  *widget,
                            NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_server_set_engine_by_id (server, gtk_widget_get_name (widget));
}

static void on_settings_menu (GtkWidget *widget,
                              gpointer   user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_spawn_command_line_async ("nimf-settings", NULL);
}

static void on_donate_menu (GtkWidget *widget,
                            gpointer   user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_spawn_command_line_async ("xdg-open https://cogniti.github.io/nimf/donate", NULL);
}

static void on_about_menu (GtkWidget *widget,
                           gpointer   user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkWidget *about_dialog;
  GtkWidget *parent;

  gchar *artists[]     = {_("Hodong Kim <cogniti@gmail.com>"), NULL};
  gchar *authors[]     = {_("Hodong Kim <cogniti@gmail.com>"), NULL};
  gchar *documenters[] = {_("Hodong Kim <cogniti@gmail.com>"), NULL};

  parent = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  about_dialog = gtk_about_dialog_new ();
  gtk_window_set_transient_for (GTK_WINDOW (about_dialog),
                                GTK_WINDOW (parent));
  gtk_window_set_destroy_with_parent (GTK_WINDOW (about_dialog), TRUE);
  gtk_window_set_icon_name (GTK_WINDOW (about_dialog), "nimf-logo");
  g_object_set (about_dialog,
    "artists",            artists,
    "authors",            authors,
    "comments",           _("Nimf is an input method framework"),
    "copyright",          _("Copyright (c) 2015-2017 Hodong Kim"),
    "documenters",        documenters,
    "license-type",       GTK_LICENSE_LGPL_3_0,
    "logo-icon-name",     "nimf-logo",
    "program-name",       _("Nimf"),
    "translator-credits", _("Hodong Kim, N"),
    "version",            VERSION,
    "website",            "https://cogniti.github.io/nimf/",
    "website-label",      _("Website"),
    NULL);

  gtk_dialog_run (GTK_DIALOG (about_dialog));

  gtk_widget_destroy (parent);
}

static void on_engine_changed (NimfServer    *server,
                               const gchar   *engine_id,
                               const gchar   *icon_name,
                               NimfIndicator *indicator)
{
  g_debug (G_STRLOC ": %s: icon_name: %s", G_STRFUNC, icon_name);

  indicator->engine_id = engine_id;

  app_indicator_set_icon_full (indicator->appindicator, icon_name, icon_name);
}

static void on_engine_status_changed (NimfServer    *server,
                                      const gchar   *engine_id,
                                      const gchar   *icon_name,
                                      NimfIndicator *indicator)
{
  g_debug (G_STRLOC ": %s: icon_name: %s", G_STRFUNC, icon_name);

  if (!g_strcmp0 (indicator->engine_id, engine_id))
    app_indicator_set_icon_full (indicator->appindicator, icon_name, icon_name);
}

const gchar *
nimf_indicator_get_id (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_SERVICE (service), NULL);

  return NIMF_INDICATOR (service)->id;
}

static gboolean nimf_indicator_start (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIndicator *indicator = NIMF_INDICATOR (service);

  g_setenv ("GTK_IM_MODULE", "gtk-im-context-simple", TRUE);

  if (!gtk_init_check (NULL, NULL))
    return FALSE;

  GtkWidget *menu_shell;
  GtkWidget *settings_menu;
  GtkWidget *about_menu;
  GtkWidget *donate_menu;

  menu_shell = gtk_menu_new ();
  indicator->appindicator = app_indicator_new ("nimf-indicator",
                                               "nimf-focus-out",
                                               APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status (indicator->appindicator,
                            APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_icon_full (indicator->appindicator,
                               "nimf-focus-out", "Nimf");
  app_indicator_set_menu (indicator->appindicator, GTK_MENU (menu_shell));

  g_signal_connect (service->server, "engine-changed",
                    G_CALLBACK (on_engine_changed), indicator);
  g_signal_connect (service->server, "engine-status-changed",
                    G_CALLBACK (on_engine_status_changed), indicator);
  /* menu */
  gchar     **engine_ids = nimf_server_get_loaded_engine_ids (service->server);
  GtkWidget  *engine_menu;

  guint i;

  for (i = 0; engine_ids != NULL && engine_ids[i] != NULL; i++)
  {
    GSettings *settings;
    gchar     *schema_id;
    gchar     *name;

    schema_id = g_strdup_printf ("org.nimf.engines.%s", engine_ids[i]);
    settings = g_settings_new (schema_id);
    name = g_settings_get_string (settings, "hidden-schema-name");

    engine_menu = gtk_menu_item_new_with_label (name);
    gtk_widget_set_name (engine_menu, engine_ids[i]);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_shell), engine_menu);
    g_signal_connect (engine_menu, "activate",
                      G_CALLBACK (on_engine_menu), service->server);

    g_free (name);
    g_free (schema_id);
    g_object_unref (settings);
  }

  g_strfreev (engine_ids);

  settings_menu = gtk_menu_item_new_with_label (_("Settings"));
  donate_menu   = gtk_menu_item_new_with_label (_("Donate"));
  about_menu    = gtk_menu_item_new_with_label (_("About"));

  g_signal_connect (settings_menu, "activate",
                    G_CALLBACK (on_settings_menu), NULL);
  g_signal_connect (donate_menu, "activate",
                    G_CALLBACK (on_donate_menu), NULL);
  g_signal_connect (about_menu, "activate",
                    G_CALLBACK (on_about_menu), NULL);

  gtk_menu_shell_append (GTK_MENU_SHELL (menu_shell), settings_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_shell), donate_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_shell), about_menu);

  gtk_widget_show_all (menu_shell);

  return TRUE;
}

static void nimf_indicator_stop (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_object_unref (NIMF_INDICATOR (service)->appindicator);
}

static void
nimf_indicator_init (NimfIndicator *indicator)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  indicator->id = g_strdup ("nimf-indicator");
}

static void
nimf_indicator_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_free (NIMF_INDICATOR (object)->id);

  G_OBJECT_CLASS (nimf_indicator_parent_class)->finalize (object);
}

static void
nimf_indicator_class_init (NimfIndicatorClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass     *object_class  = G_OBJECT_CLASS (class);
  NimfServiceClass *service_class = NIMF_SERVICE_CLASS (class);

  service_class->get_id = nimf_indicator_get_id;
  service_class->start  = nimf_indicator_start;
  service_class->stop   = nimf_indicator_stop;

  object_class->finalize = nimf_indicator_finalize;
}

static void
nimf_indicator_class_finalize (NimfIndicatorClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_indicator_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_indicator_get_type ();
}
