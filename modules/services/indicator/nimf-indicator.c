/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-indicator.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2020 Hodong Kim <cogniti@gmail.com>
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
#include <libxklavier/xklavier.h>
#include <gdk/gdkx.h>
#include "nimf-utils-private.h"

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
  guint         watcher_id;
  guint         source_id;
  XklEngine    *xklengine;
  GMenu        *menu;
};

GType nimf_indicator_get_type (void) G_GNUC_CONST;

G_DEFINE_DYNAMIC_TYPE (NimfIndicator, nimf_indicator, NIMF_TYPE_SERVICE);

static void
on_menu_engine (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  const gchar *engine_method;
  gchar **strv;

  engine_method = g_variant_get_string (parameter, NULL);
  strv = g_strsplit (engine_method, ",", -1);

  if (g_strv_length (strv) == 1)
    nimf_server_change_engine_by_id (server, engine_method);
  else
    nimf_server_change_engine (server, strv[0], strv[1]);

  g_strfreev (strv);
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

  static GtkWidget *about = NULL;

  if (!about)
  {
    GtkWidget *parent;

    gchar *artists[]     = {_("Hodong Kim <cogniti@gmail.com>"), NULL};
    gchar *authors[]     = {_("Hodong Kim <cogniti@gmail.com>"), NULL};
    gchar *documenters[] = {_("Hodong Kim <cogniti@gmail.com>"),
                            _("Bumsik Kim <k.bumsik@gmail.com>"), NULL};

    parent = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    about  = gtk_about_dialog_new ();
    gtk_window_set_transient_for (GTK_WINDOW (about), GTK_WINDOW (parent));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (about), TRUE);
    gtk_window_set_icon_name (GTK_WINDOW (about), "nimf-logo");
    g_object_set (about,
      "artists",            artists,
      "authors",            authors,
      "comments",           _("Nimf is an input method framework"),
      "copyright",          _("Copyright (c) 2015-2019 Hodong Kim"),
      "documenters",        documenters,
      "license-type",       GTK_LICENSE_LGPL_3_0,
      "logo-icon-name",     "nimf-logo",
      "program-name",       _("Nimf"),
      "translator-credits", _("Hodong Kim, Max Neupert"),
      "version",            VERSION,
      "website",            "https://github.com/hamonikr/nimf",
      "website-label",      _("Website"),
      NULL);

    gtk_dialog_run (GTK_DIALOG (about));

    gtk_widget_destroy (parent);
    about = NULL;
  }
  else
  {
    gtk_window_present (GTK_WINDOW (about));
  }
}

const GActionEntry entries[] = {
  { "engine",   on_menu_engine,   "s",  NULL, NULL },
  { "settings", on_menu_settings, NULL, NULL, NULL },
  { "about",    on_menu_about,    NULL, NULL, NULL }
};

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

static GMenu *
nimf_indicator_build_section1 (NimfIndicator *indicator,
                               NimfServer    *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **engine_ids;
  guint   i;

  GMenu *section1 = g_menu_new ();
  engine_ids = nimf_server_get_loaded_engine_ids (server);

  for (i = 0; engine_ids && engine_ids[i]; i++)
  {
    GMenuItem *engine_menu;
    GSettings *settings;
    gchar     *schema_id;
    gchar     *schema_name;
    GModule   *module;
    gchar     *path;
    gchar     *symbol_name;
    gchar     *p;
    NimfMethodInfo ** (* get_method_infos) ();

    schema_id = g_strdup_printf ("org.nimf.engines.%s", engine_ids[i]);
    settings = g_settings_new (schema_id);
    schema_name = g_settings_get_string (settings, "hidden-schema-name");

    path   = g_module_build_path (NIMF_MODULE_DIR, engine_ids[i]);
    module = g_module_open (path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);

    symbol_name = g_strdup_printf ("%s_get_method_infos", engine_ids[i]);

    for (p = symbol_name; *p; p++)
      if (*p == '-')
        *p = '_';

    if (g_module_symbol (module, symbol_name, (gpointer *) &get_method_infos))
    {
      GMenu           *submenu1;
      NimfMethodInfo **infos;
      const char      *prev_group = NULL;
      gboolean         gnome;
      gint             j;

      infos = get_method_infos ();
      submenu1 = g_menu_new ();
      engine_menu = g_menu_item_new (schema_name, "indicator.engine");
      g_menu_item_set_submenu (engine_menu, G_MENU_MODEL (submenu1));
      gnome = gnome_is_running ();

      for (j = 0; infos[j]; j++)
      {
        GMenu     *submenu2 = NULL;
        GMenuItem *menu_item;
        gchar     *engine_method;

        if (!gnome && infos[j]->group && g_strcmp0 (infos[j]->group, prev_group))
        {
          GMenuItem *lang_menu;

          submenu2 = g_menu_new ();
          lang_menu = g_menu_item_new (infos[j]->group, NULL);
          g_menu_item_set_submenu (lang_menu, G_MENU_MODEL (submenu2));
          g_menu_append_item (submenu1, lang_menu);

          g_object_unref (lang_menu);
        }

        menu_item = g_menu_item_new (infos[j]->label, "indicator.engine");
        engine_method = g_strjoin (",", engine_ids[i], infos[j]->method_id, NULL);
        g_menu_item_set_attribute (menu_item, G_MENU_ATTRIBUTE_TARGET, "s", engine_method);

        if (!gnome && infos[j]->group)
          g_menu_append_item (submenu2, menu_item);
        else
          g_menu_append_item (submenu1, menu_item);

        prev_group = infos[j]->group;

        g_object_unref (menu_item);

        if (submenu2)
          g_object_unref (submenu2);

        g_free (engine_method);
      }

      nimf_method_info_freev (infos);
      g_object_unref (submenu1);
    }
    else
    {
      engine_menu = g_menu_item_new (schema_name, "indicator.engine");
      g_menu_item_set_attribute (engine_menu, G_MENU_ATTRIBUTE_TARGET, "s", engine_ids[i]);
    }

    g_menu_append_item (section1, engine_menu);

    g_object_unref (engine_menu);
    g_free (symbol_name);
    g_module_close (module);
    g_free (path);
    g_free (schema_name);
    g_free (schema_id);
    g_object_unref (settings);
  }

  g_strfreev (engine_ids);

  return section1;
}

static GtkMenu*
nimf_indicator_build_menu (NimfIndicator *indicator)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!gtk_init_check (NULL, NULL))
    return NULL;

  /* menu */
  GtkWidget          *gtk_menu;
  GMenu              *section1;
  GMenu              *section2;
  GMenuItem          *settings_menu;
  GMenuItem          *about_menu;
  GIcon              *settings_icon;
  GIcon              *about_icon;
  GSimpleActionGroup *actions;

  indicator->menu = g_menu_new ();
  gtk_menu = gtk_menu_new_from_model (G_MENU_MODEL (indicator->menu));
  actions  = g_simple_action_group_new ();

  NimfServer *server = nimf_server_get_default ();
  g_action_map_add_action_entries (G_ACTION_MAP (actions), entries, G_N_ELEMENTS (entries), server);
  gtk_widget_insert_action_group (gtk_menu, "indicator", G_ACTION_GROUP (actions));

  section1 = nimf_indicator_build_section1 (indicator, server);
  section2 = g_menu_new ();
  settings_menu = g_menu_item_new (_("Settings"), "indicator.settings");
  about_menu    = g_menu_item_new (_("About"),    "indicator.about");

  settings_icon = g_icon_new_for_string ("preferences-system", NULL);
  about_icon    = g_icon_new_for_string ("help-about", NULL);

  g_menu_item_set_icon (settings_menu, settings_icon);
  g_menu_item_set_icon (about_menu, about_icon);

  g_menu_append_item (section2, settings_menu);
  g_menu_append_item (section2, about_menu);

  g_menu_append_section (indicator->menu, NULL, G_MENU_MODEL (section1));
  g_menu_append_section (indicator->menu, NULL, G_MENU_MODEL (section2));

  g_object_unref (section1);
  g_object_unref (section2);
  g_object_unref (settings_icon);
  g_object_unref (about_icon);
  g_object_unref (settings_menu);
  g_object_unref (about_menu);
  g_object_unref (actions);

  gtk_widget_show_all (gtk_menu);

  return GTK_MENU (gtk_menu);
}

static void
nimf_indicator_update_menu (NimfIndicator *indicator)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GMenu *section1;

  section1 = nimf_indicator_build_section1 (indicator,
                                            nimf_server_get_default ());
  g_menu_remove (indicator->menu, 0);
  g_menu_prepend_section (indicator->menu, NULL, G_MENU_MODEL (section1));

  g_object_unref (section1);
}

static void
nimf_indicator_create_appindicator (NimfIndicator *indicator)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkMenu    *gtk_menu = nimf_indicator_build_menu (indicator);
  NimfServer *server   = nimf_server_get_default ();
  indicator->appindicator = app_indicator_new ("nimf-indicator",
                                               "nimf-focus-out",
                                               APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status (indicator->appindicator,
                            APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_icon_full (indicator->appindicator,
                               "nimf-focus-out", "Nimf");
  app_indicator_set_menu (indicator->appindicator, gtk_menu);

  g_signal_connect (server, "engine-changed",
                    G_CALLBACK (on_engine_changed), indicator);
  g_signal_connect (server, "engine-status-changed",
                    G_CALLBACK (on_engine_status_changed), indicator);
  g_signal_connect_swapped (server, "engine-loaded",
                            G_CALLBACK (nimf_indicator_update_menu), indicator);
  g_signal_connect_swapped (server, "engine-unloaded",
                            G_CALLBACK (nimf_indicator_update_menu), indicator);

  /* activate xkb options for x11 */
  if ((!gnome_xkb_is_available () || !gnome_is_running ()) &&
      !g_strcmp0 (g_getenv ("XDG_SESSION_TYPE"), "x11"))
  {
    XklConfigRec *rec;
    GSettings    *settings;

    if (!indicator->xklengine)
      indicator->xklengine = xkl_engine_get_instance (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()));

    rec = xkl_config_rec_new ();
    settings = g_settings_new ("org.nimf.settings");

    xkl_config_rec_get_from_server (rec, indicator->xklengine);
    g_strfreev (rec->options);
    rec->options = g_settings_get_strv (settings, "xkb-options");
    xkl_config_rec_activate (rec, indicator->xklengine);

    g_object_unref (settings);
    g_object_unref (rec);
  }
}

static void
on_name_appeared (GDBusConnection *connection,
                  const gchar     *name,
                  const gchar     *name_owner,
                  gpointer         user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIndicator *indicator = user_data;

  if (indicator->source_id)
  {
    g_source_remove (indicator->source_id);
    indicator->source_id = 0;
  }

  g_bus_unwatch_name (indicator->watcher_id);
  indicator->watcher_id = 0;

  nimf_indicator_create_appindicator (indicator);
}

static gboolean
on_timeout (NimfIndicator *indicator)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (indicator->watcher_id)
  {
    g_bus_unwatch_name (indicator->watcher_id);
    indicator->watcher_id = 0;
  }

  indicator->source_id = 0;

  nimf_indicator_create_appindicator (indicator);

  return G_SOURCE_REMOVE;
}

static gboolean nimf_indicator_start (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIndicator *indicator = NIMF_INDICATOR (service);

  if (indicator->active)
    return TRUE;

  /*
   *  After the watcher is ready, nimf-indicator should be started.
   * Otherwise, the icon of nimf-indicator may appears strange in some
   * environments. In addition, after the window manager sets the xkb options,
   * the nimf-indicator will set the xkb options again.
   * So we will delay the launch of the nimf-indicator until the watcher is
   * ready.
   */
  indicator->watcher_id = g_bus_watch_name (G_BUS_TYPE_SESSION,
                                            "org.nimf.settings",
                                            G_BUS_NAME_WATCHER_FLAGS_NONE,
                                            on_name_appeared, NULL,
                                            indicator, NULL);
  indicator->source_id = g_timeout_add_seconds (3, (GSourceFunc) on_timeout, indicator);

  return indicator->active = TRUE;
}

static void nimf_indicator_stop (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIndicator *indicator = NIMF_INDICATOR (service);

  if (!indicator->active)
    return;

  if (indicator->watcher_id)
  {
    g_bus_unwatch_name (indicator->watcher_id);
    indicator->watcher_id = 0;
  }

  if (indicator->source_id)
  {
    g_source_remove (indicator->source_id);
    indicator->source_id = 0;
  }

  if (indicator->appindicator)
  {
    g_signal_handlers_disconnect_by_data (nimf_server_get_default (), indicator);
    g_object_unref (indicator->appindicator);
  }

  if (indicator->menu)
    g_object_unref (indicator->menu);

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

  if (indicator->xklengine)
    g_object_unref (indicator->xklengine);

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
