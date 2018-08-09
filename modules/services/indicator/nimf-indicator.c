/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-indicator.c
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
  gboolean      active;
  AppIndicator *appindicator;
  gchar        *engine_id;
};

GType nimf_indicator_get_type (void) G_GNUC_CONST;

G_DEFINE_DYNAMIC_TYPE (NimfIndicator, nimf_indicator, NIMF_TYPE_SERVICE);

static void
on_menu_engine (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  const gchar *id = g_variant_get_string (parameter, NULL);
  nimf_server_set_engine_by_id (server, id);
}

static void
on_menu_settings (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_spawn_command_line_async ("nimf-settings", NULL);
}

static void
on_menu_about (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkWidget *about_dialog;
  GtkWidget *parent;

  gchar *artists[]     = {_("Hodong Kim <cogniti@gmail.com>"), NULL};
  gchar *authors[]     = {_("Hodong Kim <cogniti@gmail.com>"), NULL};
  gchar *documenters[] = {_("Hodong Kim <cogniti@gmail.com>"),
                          _("Bumsik Kim <k.bumsik@gmail.com>"), NULL};

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
    "copyright",          _("Copyright (c) 2015-2018 Hodong Kim"),
    "documenters",        documenters,
    "license-type",       GTK_LICENSE_LGPL_3_0,
    "logo-icon-name",     "nimf-logo",
    "program-name",       _("Nimf"),
    "translator-credits", _("Hodong Kim, N"),
    "version",            VERSION,
    "website",            "https://gitlab.com/nimf-i18n/nimf",
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

  g_free (indicator->engine_id);
  indicator->engine_id = g_strdup (engine_id);

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

static gboolean nimf_indicator_is_active (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_INDICATOR (service)->active;
}

static gboolean nimf_indicator_start (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIndicator *indicator = NIMF_INDICATOR (service);

  if (indicator->active)
    return TRUE;

  g_setenv ("GTK_IM_MODULE", "gtk-im-context-simple", TRUE);

  if (!gtk_init_check (NULL, NULL))
    return FALSE;

  GtkWidget *gtk_menu;
  GMenu     *menu;

  menu = g_menu_new ();
  gtk_menu = gtk_menu_new_from_model (G_MENU_MODEL (menu));
  indicator->appindicator = app_indicator_new ("nimf-indicator",
                                               "nimf-focus-out",
                                               APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status (indicator->appindicator,
                            APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_icon_full (indicator->appindicator,
                               "nimf-focus-out", "Nimf");
  app_indicator_set_menu (indicator->appindicator, GTK_MENU (gtk_menu));

  g_signal_connect (service->server, "engine-changed",
                    G_CALLBACK (on_engine_changed), indicator);
  g_signal_connect (service->server, "engine-status-changed",
                    G_CALLBACK (on_engine_status_changed), indicator);
  /* menu */
  GMenu               *section1;
  GMenu               *section2;
  GMenuItem           *settings_menu;
  GMenuItem           *about_menu;
  GIcon               *settings_icon;
  GIcon               *about_icon;
  GSimpleActionGroup  *group;
  gchar              **engine_ids;
  guint                i;

  const GActionEntry entries[] = {
    { "engine",   on_menu_engine,   "s",  NULL, NULL },
    { "settings", on_menu_settings, NULL, NULL, NULL },
    { "about",    on_menu_about,    NULL, NULL, NULL }
  };

  section1 = g_menu_new ();
  section2 = g_menu_new ();
  group    = g_simple_action_group_new ();
  g_action_map_add_action_entries (G_ACTION_MAP (group), entries, G_N_ELEMENTS (entries), service->server);
  gtk_widget_insert_action_group (gtk_menu, "indicator", G_ACTION_GROUP (group));

  engine_ids = nimf_server_get_loaded_engine_ids (service->server);

  for (i = 0; engine_ids != NULL && engine_ids[i] != NULL; i++)
  {
    GMenuItem *engine_menu;
    GSettings *settings;
    gchar     *schema_id;
    gchar     *name;

    schema_id = g_strdup_printf ("org.nimf.engines.%s", engine_ids[i]);
    settings = g_settings_new (schema_id);
    name = g_settings_get_string (settings, "hidden-schema-name");

    engine_menu = g_menu_item_new (name, "indicator.engine");
    g_menu_item_set_attribute (engine_menu, G_MENU_ATTRIBUTE_TARGET, "s", engine_ids[i]);
    g_menu_append_item (section1, engine_menu);

    g_object_unref (engine_menu);
    g_free (name);
    g_free (schema_id);
    g_object_unref (settings);
  }

  settings_menu = g_menu_item_new (_("Settings"), "indicator.settings");
  about_menu    = g_menu_item_new (_("About"),    "indicator.about");

  settings_icon = g_icon_new_for_string ("preferences-system", NULL);
  about_icon    = g_icon_new_for_string ("help-about", NULL);

  g_menu_item_set_icon (settings_menu, settings_icon);
  g_menu_item_set_icon (about_menu, about_icon);

  g_menu_append_item (section2, settings_menu);
  g_menu_append_item (section2, about_menu);

  g_menu_append_section (menu, NULL, G_MENU_MODEL (section1));
  g_menu_append_section (menu, NULL, G_MENU_MODEL (section2));

  g_object_unref (section1);
  g_object_unref (section2);
  g_object_unref (settings_icon);
  g_object_unref (about_icon);
  g_object_unref (settings_menu);
  g_object_unref (about_menu);
  g_object_unref (group);
  g_strfreev (engine_ids);

  gtk_widget_show_all (gtk_menu);

  indicator->active = TRUE;

  return TRUE;
}

static void nimf_indicator_stop (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIndicator *indicator = NIMF_INDICATOR (service);

  if (!indicator->active)
    return;

  g_object_unref (NIMF_INDICATOR (service)->appindicator);

  indicator->active = FALSE;
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

  NimfIndicator *indicator = NIMF_INDICATOR (object);

  if (indicator->active)
    nimf_indicator_stop (NIMF_SERVICE (indicator));

  g_free (indicator->engine_id);
  g_free (indicator->id);

  G_OBJECT_CLASS (nimf_indicator_parent_class)->finalize (object);
}

static void
nimf_indicator_class_init (NimfIndicatorClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass     *object_class  = G_OBJECT_CLASS (class);
  NimfServiceClass *service_class = NIMF_SERVICE_CLASS (class);

  service_class->get_id    = nimf_indicator_get_id;
  service_class->start     = nimf_indicator_start;
  service_class->stop      = nimf_indicator_stop;
  service_class->is_active = nimf_indicator_is_active;

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
