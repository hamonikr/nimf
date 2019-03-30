/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-indicator.c
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

#include "config.h"
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "nimf.h"
#include <libappindicator/app-indicator.h>
#include <libxklavier/xklavier.h>
#include <gdk/gdkx.h>

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
  GtkWidget    *about;
};

GType nimf_indicator_get_type (void) G_GNUC_CONST;

G_DEFINE_DYNAMIC_TYPE (NimfIndicator, nimf_indicator, NIMF_TYPE_SERVICE);

static void
on_menu_engine (GtkMenuItem *menuitem,
                gpointer     server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  const gchar *engine_method;
  gchar **strv;

  engine_method = gtk_widget_get_name (GTK_WIDGET (menuitem));
  strv = g_strsplit (engine_method, ",", -1);

  if (g_strv_length (strv) == 1)
    nimf_server_change_engine_by_id (server, engine_method);
  else
    nimf_server_change_engine (server, strv[0], strv[1]);

  g_strfreev (strv);
}

static void
on_menu_settings (GtkMenuItem *menuitem,
                  gpointer     user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_spawn_command_line_async ("nimf-settings", NULL);
}

static void
on_menu_about (GtkMenuItem *menuitem,
               gpointer     user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIndicator *indicator = NIMF_INDICATOR (user_data);

  if (!indicator->about)
  {
    GtkWidget *parent;

    gchar *artists[]     = {_("Hodong Kim <cogniti@gmail.com>"), NULL};
    gchar *authors[]     = {_("Hodong Kim <cogniti@gmail.com>"), NULL};
    gchar *documenters[] = {_("Hodong Kim <cogniti@gmail.com>"),
                            _("Bumsik Kim <k.bumsik@gmail.com>"), NULL};

    parent = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    indicator->about  = gtk_about_dialog_new ();
    gtk_window_set_transient_for (GTK_WINDOW (indicator->about),
                                  GTK_WINDOW (parent));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (indicator->about), TRUE);
    gtk_window_set_icon_name (GTK_WINDOW (indicator->about), "nimf-logo");
    g_object_set (indicator->about,
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
      "website",            "https://nimf-i18n.gitlab.io",
      "website-label",      _("Website"),
      NULL);

    gtk_dialog_run (GTK_DIALOG (indicator->about));

    gtk_widget_destroy (parent);
    indicator->about = NULL;
  }
  else
  {
    gtk_window_present (GTK_WINDOW (indicator->about));
  }
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

static void
on_name_appeared (GDBusConnection *connection,
                  const gchar     *name,
                  const gchar     *name_owner,
                  gpointer         user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIndicator *indicator = user_data;

  if (!gtk_init_check (NULL, NULL))
    return;

  /* menu */
  GtkWidget *menu;

  menu = gtk_menu_new ();

  GtkWidget  *separator;
  GtkWidget  *settings_menu;
  GtkWidget  *about_menu;
  gchar     **engine_ids;
  guint       i;
  NimfServer *server = nimf_server_get_default ();

  engine_ids = nimf_server_get_loaded_engine_ids (server);

  for (i = 0; engine_ids != NULL && engine_ids[i] != NULL; i++)
  {
    GtkWidget *engine_menu;
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
      GtkWidget *submenu1 = gtk_menu_new ();
      engine_menu = gtk_menu_item_new_with_label (schema_name);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (engine_menu), submenu1);

      NimfMethodInfo **infos = get_method_infos ();
      const char *prev_group = NULL;
      GtkWidget  *submenu2   = NULL;
      gchar      *engine_method;
      gboolean    gnome;
      gint        j;

      gnome = g_str_has_suffix (g_getenv ("XDG_CURRENT_DESKTOP"), "GNOME");

      for (j = 0; infos[j]; j++)
      {
        if (!gnome && infos[j]->group && g_strcmp0 (infos[j]->group, prev_group))
        {
          submenu2 = gtk_menu_new ();
          GtkWidget *lang_menu = gtk_menu_item_new_with_label (infos[j]->group);
          gtk_menu_item_set_submenu (GTK_MENU_ITEM (lang_menu), submenu2);
          gtk_menu_shell_append (GTK_MENU_SHELL (submenu1), lang_menu);
        }

        GtkWidget *menu_item = gtk_menu_item_new_with_label (infos[j]->label);
        engine_method = g_strjoin (",", engine_ids[i], infos[j]->method_id, NULL);
        gtk_widget_set_name (menu_item, engine_method);
        g_free (engine_method);
        g_signal_connect (menu_item, "activate",
                          G_CALLBACK (on_menu_engine), server);

        if (!gnome && infos[j]->group)
          gtk_menu_shell_append (GTK_MENU_SHELL (submenu2), menu_item);
        else
          gtk_menu_shell_append (GTK_MENU_SHELL (submenu1), menu_item);

        prev_group = infos[j]->group;
      }

      nimf_method_info_freev (infos);
    }
    else
    {
      engine_menu = gtk_menu_item_new_with_label (schema_name);
      gtk_widget_set_name (engine_menu, engine_ids[i]);
      g_signal_connect (engine_menu, "activate",
                        G_CALLBACK (on_menu_engine), server);
    }

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), engine_menu);

    g_free (symbol_name);
    g_module_close (module);
    g_free (path);
    g_free (schema_name);
    g_free (schema_id);
    g_object_unref (settings);
  }

  separator     = gtk_separator_menu_item_new ();
  settings_menu = gtk_menu_item_new_with_label (_("Settings"));
  about_menu    = gtk_menu_item_new_with_label (_("About"));


  g_signal_connect (settings_menu, "activate",
                    G_CALLBACK (on_menu_settings), NULL);
  g_signal_connect (about_menu, "activate",
                    G_CALLBACK (on_menu_about), indicator);

  gtk_menu_shell_append (GTK_MENU_SHELL (menu), separator);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), settings_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), about_menu);

  gtk_widget_show_all (menu);

  g_strfreev (engine_ids);
  g_bus_unwatch_name (indicator->watcher_id);

  indicator->appindicator = app_indicator_new ("nimf-indicator",
                                               "nimf-focus-out",
                                               APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status (indicator->appindicator,
                            APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_icon_full (indicator->appindicator,
                               "nimf-focus-out", "Nimf");
  app_indicator_set_menu (indicator->appindicator, GTK_MENU (menu));

  g_signal_connect (server, "engine-changed",
                    G_CALLBACK (on_engine_changed), indicator);
  g_signal_connect (server, "engine-status-changed",
                    G_CALLBACK (on_engine_status_changed), indicator);

  /* activate xkb options */
  XklConfigRec *rec;
  GSettings    *settings;
  XklEngine    *engine;

  engine = xkl_engine_get_instance (GDK_DISPLAY_XDISPLAY
                                      (gdk_display_get_default ()));
  rec = xkl_config_rec_new ();
  settings = g_settings_new ("org.nimf.settings");

  xkl_config_rec_get_from_server (rec, engine);
  g_strfreev (rec->options);
  rec->options = g_settings_get_strv (settings, "xkb-options");
  xkl_config_rec_activate (rec, engine);

  g_object_unref (settings);
  g_object_unref (rec);
  g_object_unref (engine);
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
  return indicator->active = TRUE;
}

static void nimf_indicator_stop (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIndicator *indicator = NIMF_INDICATOR (service);

  if (!indicator->active)
    return;

  if (indicator->appindicator)
    g_object_unref (indicator->appindicator);

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
