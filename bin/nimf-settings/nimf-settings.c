/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-settings.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2016-2019 Hodong Kim <cogniti@gmail.com>
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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libxklavier/xklavier.h>
#if GTK_CHECK_VERSION(4, 0, 0)
#include <gdk/x11/gdkx.h>
#else
#include <gdk/gdkx.h>
#endif
#include "config.h"
#include "nimf.h"
#include "nimf-enum-types-private.h"
#include "nimf-utils-private.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define EXPORT_ENVIRONMENT "export $(/usr/lib/systemd/user-environment-generators/30-systemd-environment-d-generator)"
#define INPUT_CONF         "50-input.conf"
/* Minimum width to prevent sidebar from collapsing on some GTK themes */
#define SIDEBAR_MIN_WIDTH  180

#define NIMF_TYPE_SETTINGS             (nimf_settings_get_type ())
#define NIMF_SETTINGS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_SETTINGS, NimfSettings))
#define NIMF_SETTINGS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_SETTINGS, NimfSettingsClass))
#define NIMF_IS_SETTINGS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_SETTINGS))
#define NIMF_IS_SETTINGS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_SETTINGS))

/* Helper function to lookup schema with XDG_DATA_DIRS fallback */
static GSettingsSchema *
nimf_settings_lookup_schema_with_fallback (GSettingsSchemaSource *source, 
                                            const gchar *schema_id)
{
  GSettingsSchema *schema = NULL;
  
  /* First try default source */
  schema = g_settings_schema_source_lookup (source, schema_id, FALSE);
  if (schema != NULL)
    return schema;
  
  /* Try schema directories from XDG_DATA_DIRS */
  const gchar *xdg_data_dirs = g_getenv ("XDG_DATA_DIRS");
  if (xdg_data_dirs == NULL)
    xdg_data_dirs = "/usr/local/share:/usr/share";
  
  gchar **data_dirs = g_strsplit (xdg_data_dirs, ":", -1);
  for (gint i = 0; data_dirs[i] != NULL && schema == NULL; i++)
  {
    gchar *schema_dir = g_build_filename (data_dirs[i], "glib-2.0", "schemas", NULL);
    if (g_file_test (schema_dir, G_FILE_TEST_IS_DIR))
    {
      GSettingsSchemaSource *additional_source = NULL;
      GError *error = NULL;
      
      additional_source = g_settings_schema_source_new_from_directory (schema_dir,
                                                                        source, FALSE, &error);
      if (additional_source != NULL)
      {
        schema = g_settings_schema_source_lookup (additional_source, schema_id, FALSE);
        g_settings_schema_source_unref (additional_source);
      }
      else if (error != NULL)
      {
        g_debug ("Failed to load schema source from %s: %s", schema_dir, error->message);
        g_error_free (error);
      }
    }
    g_free (schema_dir);
  }
  g_strfreev (data_dirs);
  
  return schema;
}
#define NIMF_SETTINGS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_SETTINGS, NimfSettingsClass))

typedef struct _NimfXkb
{
  XklEngine  *engine;
  GtkWidget  *options_box;
  GtkWidget  *scrolled;
  GtkWidget  *vbox;
  gchar     **options;
  gint        options_len;
  GSList     *toggle_buttons;
  GSList     *radio_group;
  GSList     *expanders;
} NimfXkb;

typedef struct _NimfSettings      NimfSettings;
typedef struct _NimfSettingsClass NimfSettingsClass;

struct _NimfSettings
{
  GObject parent_instance;

  GApplication *app;
};

struct _NimfSettingsClass
{
  GObjectClass parent_class;
};

GType nimf_settings_get_type (void) G_GNUC_CONST;

enum {
  SCHEMA_COLUMN = 0
};

typedef struct _NimfSettingsPage
{
  GSettings *gsettings;
  GtkWidget *box;
  GPtrArray *page_keys;
} NimfSettingsPage;

typedef struct _NimfSettingsPageKey {
  GSettings *gsettings;
  gchar     *key;
  GtkWidget *label;
  GtkWidget *treeview;
} NimfSettingsPageKey;

static GtkWidget *nimf_settings_window = NULL;

G_DEFINE_TYPE (NimfSettings, nimf_settings, G_TYPE_OBJECT);

static gboolean
on_foreach (GtkTreeModel *model,
            GtkTreePath  *path,
            GtkTreeIter  *iter,
            gpointer      user_data)
{
  gchar     *id;
  GtkWidget *widget = user_data;
  gint       retval;

  gtk_tree_model_get (model, iter, 1, &id, -1);
  retval = g_strcmp0 (id, gtk_widget_get_name (widget));

  g_free (id);

  if (!retval)
  {
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (widget), iter);
    return TRUE;
  }

  return FALSE;
}

static void
on_gsettings_changed (GSettings *settings,
                      gchar     *key,
                      GtkWidget *widget)
{
  if (GTK_IS_SWITCH (widget))
  {
    gboolean active1 = g_settings_get_boolean (settings, key);
    gboolean active2 = gtk_switch_get_active (GTK_SWITCH (widget));

    if (active1 != active2)
      gtk_switch_set_active (GTK_SWITCH (widget), active1);
  }
  else if (GTK_IS_COMBO_BOX (widget))
  {
    gchar    *id;
    gboolean  retval;

    id = g_settings_get_string (settings, key);
    retval = gtk_combo_box_set_active_id (GTK_COMBO_BOX (widget), id);

    GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (widget));

    if (!retval)
    {
      gtk_widget_set_name (widget, id);
      gtk_tree_model_foreach (model, on_foreach, widget);
    }

    if (retval == FALSE && g_strcmp0 (key, "default-engine") == 0)
      g_settings_set_string (settings, key, "nimf-system-keyboard");

    g_free (id);
  }
  else if (GTK_IS_TREE_VIEW (widget))
  {
    GtkTreeModel  *model;
    gchar        **vals;
    GtkTreeIter    iter;
    gint           i;

    vals = g_settings_get_strv (settings, key);
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));
    gtk_list_store_clear (GTK_LIST_STORE (model));

    for (i = 0; vals[i] != NULL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set    (GTK_LIST_STORE (model), &iter, 0, vals[i], -1);
    }

    g_strfreev (vals);
  }
}

static gint
on_comparison (const char *a,
               const char *b)
{
  if (g_str_has_prefix (a, "org.nimf.engines.") &&
      g_str_has_prefix (b, "org.nimf.engines."))
  {
    GSettingsSchemaSource *source;
    GSettingsSchemaKey    *key_a,     *key_b;
    GSettingsSchema       *schema_a,  *schema_b;
    GVariant              *variant_a, *variant_b;
    gchar                 *value_a,   *value_b;
    gint                   retval;

    source = g_settings_schema_source_get_default ();
    schema_a = nimf_settings_lookup_schema_with_fallback (source, a);

    if (schema_a == NULL)
    {
      g_warning (G_STRLOC ": %s: %s is not found.", G_STRFUNC, a);
      return g_strcmp0 (a, b);
    }

    schema_b = nimf_settings_lookup_schema_with_fallback (source, b);

    if (schema_b == NULL)
    {
      g_warning (G_STRLOC ": %s: %s is not found.", G_STRFUNC, b);
      return g_strcmp0 (a, b);
    }

    key_a = g_settings_schema_get_key (schema_a, "hidden-schema-name");
    key_b = g_settings_schema_get_key (schema_b, "hidden-schema-name");
    variant_a = g_settings_schema_key_get_default_value (key_a);
    variant_b = g_settings_schema_key_get_default_value (key_b);
    value_a = g_strdup (g_variant_get_string (variant_a, NULL));
    value_b = g_strdup (g_variant_get_string (variant_b, NULL));

    retval = g_utf8_collate (value_a, value_b);

    g_free (value_a);
    g_free (value_b);
    g_variant_unref (variant_a);
    g_variant_unref (variant_b);
    g_settings_schema_key_unref (key_a);
    g_settings_schema_key_unref (key_b);
    g_settings_schema_unref (schema_a);
    g_settings_schema_unref (schema_b);

    return retval;
  }

  return g_strcmp0 (a, b);
}

static void
on_combo_box_changed (GtkComboBox        *widget,
                     NimfSettingsPageKey *page_key)
{
  const gchar *id1;
  gchar       *id2;

  id1 = gtk_combo_box_get_active_id (GTK_COMBO_BOX (widget));
  id2 = g_settings_get_string (page_key->gsettings, page_key->key);

  if (id1 && g_strcmp0 (id1, id2) != 0)
    g_settings_set_string (page_key->gsettings, page_key->key, id1);

  g_free (id2);
}

static gboolean
on_finding (gconstpointer a,
            gconstpointer b)
{
  return !g_strcmp0 (a, b);
}

static gboolean
on_active_engine_state_set (GtkSwitch           *widget,
                            gboolean             state,
                            NimfSettingsPageKey *page_key)
{
  if (state != g_settings_get_boolean (page_key->gsettings, page_key->key))
  {
    const gchar *engine_id;
    GPtrArray   *engine_ids;
    GSettings   *settings;
    gchar      **strv;
    gint         i;

    engine_ids = g_ptr_array_new ();
    settings   = g_settings_new ("org.nimf");
    engine_id  = gtk_widget_get_name (GTK_WIDGET (widget));
    strv       = g_settings_get_strv (settings, "hidden-active-engines");

    for (i = 0; strv[i]; i++)
      g_ptr_array_add (engine_ids, strv[i]);

    if (state)
    {
      if (!g_ptr_array_find_with_equal_func (engine_ids, engine_id, (GEqualFunc) on_finding, NULL))
        g_ptr_array_add (engine_ids, g_strdup (engine_id));
    }
    else
    {
      guint index;
      if (g_ptr_array_find_with_equal_func (engine_ids, engine_id, (GEqualFunc) on_finding, &index))
        g_ptr_array_remove_index_fast (engine_ids, index);
    }

    g_free (strv);
    g_ptr_array_add (engine_ids, NULL);
    strv = (gchar **) g_ptr_array_free (engine_ids, FALSE);

    g_settings_set_strv (settings, "hidden-active-engines", (const gchar *const *) strv);
    g_settings_set_boolean (page_key->gsettings, page_key->key, state);

    /* Force settings synchronization to ensure server picks up changes immediately */
    g_settings_sync ();
    
    /* Give the server a moment to process the settings change */
    g_usleep (100000); /* 100ms delay */

    g_strfreev (strv);
    g_object_unref (settings);
  }

  return FALSE;
}

static void
on_notify_active (GtkSwitch           *widget,
                  GParamSpec          *pspec,
                  NimfSettingsPageKey *page_key)
{
  gboolean active1;
  gboolean active2;

  active1 = gtk_switch_get_active (GTK_SWITCH (widget));
  active2 = g_settings_get_boolean (page_key->gsettings, page_key->key);

  if (active1 != active2)
    g_settings_set_boolean (page_key->gsettings, page_key->key, active1);
}

static NimfSettingsPageKey *
nimf_settings_page_key_new (GSettings   *gsettings,
                            const gchar *key,
                            const gchar *summary,
                            const gchar *desc)
{
  NimfSettingsPageKey *page_key;

  page_key            = g_slice_new (NimfSettingsPageKey);
  page_key->gsettings = gsettings;
  page_key->key       = g_strdup (key);
  page_key->label     = gtk_label_new (summary);
  gtk_widget_set_halign (page_key->label, GTK_ALIGN_START);
  gtk_widget_set_tooltip_text (page_key->label, desc);

  return page_key;
}

static void
nimf_settings_page_key_free (NimfSettingsPageKey *page_key)
{
  g_free (page_key->key);
  g_slice_free (NimfSettingsPageKey, page_key);
}

static GtkWidget *
nimf_settings_page_key_build_boolean (NimfSettingsPageKey *page_key,
                                      const gchar         *schema_id)
{
  GtkWidget *gswitch;
  GtkWidget *hbox;
  gchar     *detailed_signal;
  gboolean   is_active;

  gswitch = gtk_switch_new ();

  if (g_str_has_prefix (schema_id, "org.nimf.engines."))
    gtk_widget_set_name (gswitch, schema_id + strlen ("org.nimf.engines."));

  is_active = g_settings_get_boolean (page_key->gsettings, page_key->key);
  gtk_switch_set_active (GTK_SWITCH (gswitch), is_active);
  gtk_widget_set_halign  (gswitch, GTK_ALIGN_END);
  detailed_signal = g_strdup_printf ("changed::%s", page_key->key);

  g_signal_connect (gswitch, "notify::active",
                    G_CALLBACK (on_notify_active), page_key);
  if (!g_strcmp0 (page_key->key, "active-engine"))
    g_signal_connect (gswitch, "state-set",
                      G_CALLBACK (on_active_engine_state_set), page_key);
  g_signal_connect (page_key->gsettings, detailed_signal,
                    G_CALLBACK (on_gsettings_changed), gswitch);

  g_free (detailed_signal);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 15);
#if !GTK_CHECK_VERSION(4, 0, 0)
  gtk_box_pack_start (GTK_BOX (hbox), page_key->label, FALSE, FALSE, 0);
  gtk_box_pack_end   (GTK_BOX (hbox), gswitch, FALSE, FALSE, 0);
#else
  gtk_box_append (GTK_BOX (hbox), page_key->label);
  gtk_box_append (GTK_BOX (hbox), gswitch);
#endif

  return hbox;
}

static GtkWidget *
nimf_settings_page_key_build_string (NimfSettingsPageKey *page_key,
                                     const gchar         *schema_id,
                                     GList               *key_list)
{
  GtkTreeStore *store;
  GtkWidget    *combo;
  GtkWidget    *hbox;
  gchar        *detailed_signal;
  GtkTreeIter   iter;

  store = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
  combo = gtk_combo_box_text_new ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));
  g_object_unref (store);
  gtk_combo_box_set_id_column (GTK_COMBO_BOX (combo), 1);
  gtk_tree_model_get_iter_first ((GtkTreeModel *) store, &iter);

  if (!g_strcmp0 (schema_id, "org.nimf.engines") &&
      !g_strcmp0 (page_key->key, "default-engine"))
  {
    GSettingsSchemaSource *schema_source;
    GList                 *schema_list = NULL;
    gchar                **schemas;
    gchar                 *id1;
    gint                   i;

    id1 = g_settings_get_string (page_key->gsettings, page_key->key);
    schema_source = g_settings_schema_source_get_default ();
    g_settings_schema_source_list_schemas (schema_source, TRUE,
                                           &schemas, NULL);

    for (i = 0; schemas[i] != NULL; i++)
      if (g_str_has_prefix (schemas[i], "org.nimf.engines."))
        schema_list = g_list_prepend (schema_list, schemas[i]);

    for (schema_list = g_list_sort (schema_list, (GCompareFunc) on_comparison);
         schema_list != NULL;
         schema_list = schema_list->next)
    {
      GSettingsSchema *schema;
      GSettings       *gsettings;
      gchar           *name;
      const gchar     *id2;

      schema = g_settings_schema_source_lookup (schema_source,
                                                schema_list->data, TRUE);
      
      /* Try to lookup schema with XDG_DATA_DIRS fallback */
      if (schema == NULL)
      {
        schema = nimf_settings_lookup_schema_with_fallback (schema_source, schema_list->data);
      }
      
      if (schema == NULL)
        continue;
        
      gsettings = g_settings_new (schema_list->data);
      name = g_settings_get_string (gsettings, "hidden-schema-name");
      id2 = schema_list->data + strlen ("org.nimf.engines.");

      if (g_settings_schema_has_key (schema, "active-engine") == FALSE ||
          g_settings_get_boolean (gsettings, "active-engine"))
      {
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, name, 1, id2, -1);
      }

      g_settings_schema_unref (schema);
      g_free (name);
      g_object_unref (gsettings);
    }

    if (gtk_combo_box_set_active_id (GTK_COMBO_BOX (combo), id1) == FALSE)
    {
      g_settings_set_string (page_key->gsettings, "default-engine",
                             "nimf-system-keyboard");
      gtk_combo_box_set_active_id (GTK_COMBO_BOX (combo),
                                   "nimf-system-keyboard");
    }

    g_strfreev (schemas);
    g_list_free (schema_list);
    g_free (id1);
  }
  else if (g_str_has_prefix (page_key->key, "hidden-") == FALSE)
  {
    gchar *id1;

    id1 = g_settings_get_string (page_key->gsettings, page_key->key);

    if (g_str_has_prefix (page_key->key, "get-"))
    {
      const gchar *engine_id;
      GModule *module;
      NimfMethodInfo ** (* function) ();
      gchar  *path;
      gchar  *symbol_name;
      gchar  *p;

      engine_id = schema_id + strlen ("org.nimf.engines.");
      path   = g_module_build_path (NIMF_MODULE_DIR, engine_id);
      module = g_module_open (path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);

      symbol_name = g_strdup_printf ("%s_%s", engine_id, page_key->key);

      for (p = symbol_name; *p; p++)
        if (*p == '-')
          *p = '_';

      if (g_module_symbol (module, symbol_name, (gpointer *) &function))
      {
        GtkTreeIter parent;
        NimfMethodInfo **infos = function ();
        const gchar *prev_group = NULL;
        gint    i;

        for (i = 0; infos[i]; i++)
        {
          if (infos[i]->group && g_strcmp0 (infos[i]->group, prev_group))
          {
            gtk_tree_store_append (store, &parent, NULL);
            gtk_tree_store_set    (store, &parent, 0, infos[i]->group, -1);
          }

          if (infos[i]->group)
            gtk_tree_store_append (store, &iter, &parent);
          else
            gtk_tree_store_append (store, &iter, NULL);

          gtk_tree_store_set (store, &iter, 0, infos[i]->label,
                                            1, infos[i]->method_id, -1);

          if (g_strcmp0 (id1, infos[i]->method_id) == 0)
            gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);

          prev_group = infos[i]->group;
        }

        nimf_method_info_freev (infos);
      }
      else
      {
        g_warning (G_STRLOC ": %s", g_module_error ());
      }

      g_free (symbol_name);
      g_module_close (module);
      g_free (path);
    }

    g_free (id1);
  }

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 15);
#if !GTK_CHECK_VERSION(4, 0, 0)
  gtk_box_pack_start (GTK_BOX (hbox), page_key->label, FALSE, FALSE, 0);
  gtk_box_pack_end   (GTK_BOX (hbox), combo, FALSE, FALSE, 0);
#else
  gtk_box_append (GTK_BOX (hbox), page_key->label);
  gtk_box_append (GTK_BOX (hbox), combo);
#endif
  detailed_signal = g_strdup_printf ("changed::%s", page_key->key);

  g_signal_connect (combo, "changed",
                    G_CALLBACK (on_combo_box_changed), page_key);
  g_signal_connect (page_key->gsettings, detailed_signal,
                    G_CALLBACK (on_gsettings_changed), combo);

  g_free (detailed_signal);

  return hbox;
}

static gboolean
on_key_press_event (GtkWidget *widget,
                    GdkEvent  *event,
                    GtkWidget *dialog)
{
#if GTK_CHECK_VERSION(4, 0, 0)
  /* Not used in GTK4; handled by GtkEventControllerKey in on_button_clicked_add */
  return FALSE;
#else
  const gchar *keystr;
  GString     *combination;
  GFlagsClass *flags_class; /* do not free */
  guint        mod;
  guint        i;

  combination = g_string_new ("");
  mod = event->key.state & NIMF_MODIFIER_MASK;
  flags_class = (GFlagsClass *) g_type_class_ref (NIMF_TYPE_MODIFIER_TYPE);

  /* does not use virtual modifiers */
  for (i = 0; i <= 12; i++)
  {
    GFlagsValue *flags_value = g_flags_get_first_value (flags_class,
                                                        mod & (1 << i));
    if (flags_value)
      g_string_append_printf (combination, "%s ", flags_value->value_nick);
  }

  g_type_class_unref (flags_class);

  keystr = nimf_keyval_to_keysym_name (event->key.keyval);

  if (keystr == NULL)
  {
    gchar *text;

    g_string_free (combination, TRUE);
    gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
                                       GTK_RESPONSE_OK, FALSE);
    text = g_strdup_printf (_("Please report a bug. (keyval: %d)"),
                            event->key.keyval);
    gtk_entry_set_text (GTK_ENTRY (widget), text);

    g_free (text);

    g_return_val_if_fail (keystr, TRUE);
  }

  g_string_append_printf (combination, "%s", keystr);
  gtk_entry_set_text (GTK_ENTRY (widget), combination->str);
  gtk_editable_set_position (GTK_EDITABLE (widget), -1);
  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
                                     GTK_RESPONSE_OK, TRUE);
  g_string_free (combination, TRUE);

  return TRUE;
#endif
}

static void
nimf_settings_page_key_update_gsettings_strv (NimfSettingsPageKey *page_key,
                                              GtkTreeModel        *model)
{
  gchar       **vals;
  GtkTreeIter   iter;
  gint          i;

  vals = g_malloc0_n (1, sizeof (gchar *));

  if (gtk_tree_model_get_iter_first (model, &iter))
  {
    i = 0;
    do {
      gtk_tree_model_get (model, &iter, 0, &vals[i], -1);
      vals = g_realloc_n (vals, sizeof (gchar *), i + 2);
      vals[i + 1] = NULL;
      i++;
    } while (gtk_tree_model_iter_next (model, &iter));
  }

  g_settings_set_strv (page_key->gsettings,
                       page_key->key, (const gchar **) vals);

  g_strfreev (vals);
}

static void
on_button_clicked_add (GtkButton           *button,
                       NimfSettingsPageKey *page_key)
{
  GtkWidget *dialog;
  GtkWidget *entry;
  GtkWidget *content_area;
  GtkDialogFlags flags;

#if GTK_CHECK_VERSION (3, 12, 0)
  flags = GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR;
#else
  flags = GTK_DIALOG_MODAL;
#endif

  dialog = gtk_dialog_new_with_buttons (_("Press key combination"),
                                        GTK_WINDOW (nimf_settings_window),
                                        flags,
                                        _("_OK"),     GTK_RESPONSE_OK,
                                        _("_Cancel"), GTK_RESPONSE_CANCEL,
                                        NULL);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "nimf-logo");
  gtk_widget_set_size_request (GTK_WIDGET (dialog), 400, -1);
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
                                     GTK_RESPONSE_OK, FALSE);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  entry = gtk_entry_new ();
  gtk_entry_set_placeholder_text (GTK_ENTRY (entry),
                                  _("Click here and then press key combination"));
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_box_append (GTK_BOX (content_area), entry);
#else
  gtk_box_pack_start (GTK_BOX (content_area), entry, TRUE, TRUE, 0);
#endif
  g_signal_connect (entry, "key-press-event",
                    G_CALLBACK (on_key_press_event), dialog);

  gtk_widget_show_all (content_area);

  switch (gtk_dialog_run (GTK_DIALOG (dialog)))
  {
    case GTK_RESPONSE_OK:
      {
        GtkTreeModel *model;
        const gchar  *text;
        GtkTreeIter   iter;

        model = gtk_tree_view_get_model (GTK_TREE_VIEW (page_key->treeview));
        text = gtk_entry_get_text (GTK_ENTRY (entry));
        gtk_list_store_append (GTK_LIST_STORE (model), &iter);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, text, -1);
        nimf_settings_page_key_update_gsettings_strv (page_key, model);
      }
      break;
    default:
      break;
  }

  gtk_widget_destroy (dialog);
}

static void
on_button_clicked_remove (GtkButton           *button,
                          NimfSettingsPageKey *page_key)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (page_key->treeview));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    nimf_settings_page_key_update_gsettings_strv (page_key, model);
  }
}

static void
on_tree_view_realize (GtkWidget         *tree_view,
                      GtkScrolledWindow *scrolled_w)
{
  GtkTreeViewColumn *column;
  gint height;

  column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), 0);
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_tree_view_column_cell_get_size (column, NULL, NULL, &height);
#else
  gtk_tree_view_column_cell_get_size (column, NULL, NULL, NULL, NULL, &height);
#endif
  /* Increase visible rows for hotkey list (default: 4 rows) */
  gtk_widget_set_size_request (GTK_WIDGET (scrolled_w), -1, height * 4 );
}

static GtkWidget *
nimf_settings_page_key_build_string_array (NimfSettingsPageKey *page_key)
{
  GtkListStore      *store;
  GtkWidget         *treeview;
  GtkCellRenderer   *renderer;
  GtkTreeViewColumn *column;
  GtkWidget         *scrolled_w;
  GtkWidget         *hbox;
  GtkWidget         *vbox;
  GtkWidget         *button1;
  GtkWidget         *button2;
  gchar            **strv;
  gchar             *detailed_signal;
  GtkTreeIter        iter;
  gint               j;

#if GTK_CHECK_VERSION(4, 0, 0)
  button1 = gtk_button_new_from_icon_name ("list-add");
  button2 = gtk_button_new_from_icon_name ("list-remove");
#else
  button1 = gtk_button_new_from_icon_name ("list-add",    GTK_ICON_SIZE_SMALL_TOOLBAR);
  button2 = gtk_button_new_from_icon_name ("list-remove", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_button_set_relief (GTK_BUTTON (button1), GTK_RELIEF_NONE);
  gtk_button_set_relief (GTK_BUTTON (button2), GTK_RELIEF_NONE);
#endif

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

#if GTK_CHECK_VERSION (3, 12, 0)
  gtk_widget_set_margin_end   (page_key->label, 15);
#else
  gtk_widget_set_margin_right (page_key->label, 15);
#endif

  gtk_box_pack_start (GTK_BOX (hbox), page_key->label,   FALSE, FALSE, 0);
  gtk_box_pack_end   (GTK_BOX (hbox), button2, FALSE, FALSE, 0);
  gtk_box_pack_end   (GTK_BOX (hbox), button1, FALSE, FALSE, 0);

  store = gtk_list_store_new (1, G_TYPE_STRING);
  strv = g_settings_get_strv (page_key->gsettings, page_key->key);

  for (j = 0; strv[j] != NULL; j++)
  {
    gtk_list_store_append (store, &iter);
    gtk_list_store_set    (store, &iter, 0, strv[j], -1);
  }

  treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Hotkeys", renderer,
                                                     "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);

  scrolled_w = gtk_scrolled_window_new (NULL, NULL);
#if !GTK_CHECK_VERSION(4, 0, 0)
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_w),
                                       GTK_SHADOW_ETCHED_IN);
#endif
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_w),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled_w), treeview);
#else
  gtk_container_add (GTK_CONTAINER (scrolled_w), treeview);
#endif

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_box_append (GTK_BOX (vbox), hbox);
  gtk_box_append (GTK_BOX (vbox), scrolled_w);
#else
  gtk_box_pack_start (GTK_BOX (vbox), hbox,       FALSE, FALSE, 0);
  gtk_box_pack_end   (GTK_BOX (vbox), scrolled_w, FALSE, FALSE, 0);
#endif

  page_key->treeview = treeview;
  detailed_signal = g_strdup_printf ("changed::%s", page_key->key);

  g_signal_connect (treeview, "realize",
                    G_CALLBACK (on_tree_view_realize), scrolled_w);
  g_signal_connect (button1, "clicked", G_CALLBACK (on_button_clicked_add), page_key);
  g_signal_connect (button2, "clicked", G_CALLBACK (on_button_clicked_remove), page_key);
  g_signal_connect (page_key->gsettings, detailed_signal,
                    G_CALLBACK (on_gsettings_changed), page_key->treeview);
  g_strfreev (strv);
  g_free (detailed_signal);

  return vbox;
}

static gboolean
xprofile_contains_generator (GFile *file)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GFileInputStream *fis;
  GDataInputStream *dis;
  gboolean          found = FALSE;

  if (!(fis = g_file_read (file, NULL, NULL)))
    return FALSE;

  dis  = g_data_input_stream_new (G_INPUT_STREAM (fis));

  do {
    char *line;

    line = g_data_input_stream_read_line (dis, NULL, NULL, NULL);

    if (!line)
      break;

    g_strstrip (line);
    found = !g_strcmp0 (line, EXPORT_ENVIRONMENT);

    g_free (line);
  } while (found == FALSE);

  g_input_stream_close (G_INPUT_STREAM (dis), NULL, NULL);
  g_input_stream_close (G_INPUT_STREAM (fis), NULL, NULL);
  g_object_unref (dis);
  g_object_unref (fis);

  return found;
}

static void
on_setup_environment (GSettings  *settings,
                      gchar      *key,
                      gpointer    user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (g_settings_get_boolean (settings, key))
  {
    GFile   *file;
    gchar   *filename;
    gchar   *xprofile;
    gboolean success = TRUE;

    filename = g_build_filename (G_DIR_SEPARATOR_S, g_get_user_config_dir (),
                                 "environment.d", INPUT_CONF, NULL);
    if (g_access (filename, F_OK))
    {
      gchar *path;

      path = g_build_filename (G_DIR_SEPARATOR_S, g_get_user_config_dir (),
                               "environment.d", NULL);
      g_unlink (filename);
      success = !g_mkdir_with_parents (path, 0700) &&
                !symlink ("/etc/input.d/nimf.conf", filename);

      g_free (path);
    }

    g_free (filename);

    if (!success)
    {
      g_settings_set_boolean (settings, key, FALSE);
      return;
    }

    xprofile = g_build_filename (G_DIR_SEPARATOR_S, g_get_home_dir (),
                                 ".xprofile", NULL);
    file  = g_file_new_for_path (xprofile);

    if (xprofile_contains_generator (file) == FALSE)
    {
      GFileOutputStream *fos;
      gboolean           success = FALSE;

      fos = g_file_append_to (file, G_FILE_CREATE_NONE, NULL, NULL);

      if (fos)
      {
        gsize count = strlen (EXPORT_ENVIRONMENT) + 1;

        if (g_output_stream_write (G_OUTPUT_STREAM (fos),
                                   EXPORT_ENVIRONMENT"\n", count,
                                   NULL, NULL) == count)
          success = TRUE;

        g_output_stream_close (G_OUTPUT_STREAM (fos), NULL, NULL);
        g_object_unref (fos);
      }

      if (success == FALSE)
        g_settings_set_boolean (settings, key, FALSE);
    }

    g_object_unref (file);
    g_free (xprofile);
  }
  else
  {
    gchar *filename;

    filename = g_build_filename (G_DIR_SEPARATOR_S, g_get_user_config_dir (),
                                 "environment.d", INPUT_CONF, NULL);
    if (!g_access (filename, F_OK))
      g_unlink (filename);

    g_free (filename);
  }
}

static void
nimf_settings_page_free (NimfSettingsPage *page)
{
  g_object_unref (page->gsettings);
  g_ptr_array_free (page->page_keys, TRUE);
  g_slice_free (NimfSettingsPage, page);
}

static NimfSettingsPage *
nimf_settings_page_new (const gchar  *schema_id)
{
  NimfSettingsPage *page;
  GSettingsSchema  *schema;
  GList            *key_list = NULL;
  gchar           **keys;
  GList            *l;
  gint              i;

  if (schema_id == NULL || *schema_id == '\0')
  {
    g_warning ("Invalid schema_id provided to nimf_settings_page_new");
    return NULL;
  }

  GSettingsSchemaSource *source = g_settings_schema_source_get_default ();
  
  /* Try to lookup schema with XDG_DATA_DIRS fallback */
  schema = nimf_settings_lookup_schema_with_fallback (source, schema_id);
  
  if (schema == NULL)
  {
    g_warning ("Schema '%s' is not installed, cannot create settings page", schema_id);
    return NULL;
  }

  page = g_slice_new0 (NimfSettingsPage);
  page->gsettings = g_settings_new (schema_id);
  page->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 15);
  page->page_keys = g_ptr_array_new_with_free_func ((GDestroyNotify) nimf_settings_page_key_free);
  g_object_set_data_full (G_OBJECT (page->box), "free", page, (GDestroyNotify) nimf_settings_page_free);

#if GTK_CHECK_VERSION (3, 12, 0)
  gtk_widget_set_margin_start  (page->box, 15);
  gtk_widget_set_margin_end    (page->box, 15);
#else
  gtk_widget_set_margin_left   (page->box, 15);
  gtk_widget_set_margin_right  (page->box, 15);
#endif

  gtk_widget_set_margin_top    (page->box, 15);
  gtk_widget_set_margin_bottom (page->box, 15);

  GSettingsSchemaSource *default_source = g_settings_schema_source_get_default ();
  schema = nimf_settings_lookup_schema_with_fallback (default_source, schema_id);
#if GLIB_CHECK_VERSION (2, 46, 0)
  keys = g_settings_schema_list_keys (schema);
#else
  keys = g_settings_list_keys (page->gsettings);
#endif

  for (i = 0; keys[i] != NULL; i++)
    key_list = g_list_prepend (key_list, keys[i]);

  key_list = g_list_sort (key_list, (GCompareFunc) g_strcmp0);

  for (i = 0, l = key_list; l != NULL; l = l->next, i++)
  {
    GSettingsSchemaKey  *schema_key = NULL;
    NimfSettingsPageKey *page_key;
    const GVariantType  *type;
    const gchar         *key;
    const gchar         *summary;
    const gchar         *desc;

    key = l->data;

    if (g_str_has_prefix (key, "hidden-"))
      continue;

    schema_key = g_settings_schema_get_key (schema, key);
    type = g_settings_schema_key_get_value_type (schema_key);
    summary = g_settings_schema_key_get_summary     (schema_key);
    desc    = g_settings_schema_key_get_description (schema_key);

    page_key = nimf_settings_page_key_new (page->gsettings, key, summary, desc);
    g_ptr_array_add (page->page_keys, page_key);

    if (g_variant_type_equal (type, G_VARIANT_TYPE_BOOLEAN))
    {
      GtkWidget *item;
      item = nimf_settings_page_key_build_boolean (page_key, schema_id);
  gtk_box_pack_start (GTK_BOX (page->box), item, FALSE, FALSE, 0);
    }
    else if (g_variant_type_equal (type, G_VARIANT_TYPE_STRING))
    {
      GtkWidget *item;
      item = nimf_settings_page_key_build_string (page_key, schema_id, key_list);
  gtk_box_pack_start (GTK_BOX (page->box), item, FALSE, FALSE, 0);
    }
    else if (g_variant_type_equal (type, G_VARIANT_TYPE_STRING_ARRAY))
    {
      GtkWidget *item;
      item = nimf_settings_page_key_build_string_array (page_key);
  gtk_box_pack_start (GTK_BOX (page->box), item, FALSE, FALSE, 0);
    }
    else
      g_error (G_STRLOC ": %s: not supported variant type: \"%s\"",
               G_STRFUNC, (gchar *) type);

    g_settings_schema_key_unref (schema_key);
  }

  if (!g_strcmp0 (schema_id, "org.nimf"))
  {
    GFile *file;
    gchar *xprofile;
    gchar *conf;

    xprofile = g_build_filename (G_DIR_SEPARATOR_S, g_get_home_dir (),
                                 ".xprofile", NULL);
    file = g_file_new_for_path (xprofile);
    conf = g_build_filename (G_DIR_SEPARATOR_S, g_get_user_config_dir (),
                             "environment.d", INPUT_CONF, NULL);
    g_settings_set_boolean (page->gsettings, "setup-environment-variables",
                            !g_access (conf, F_OK) &&
                            xprofile_contains_generator (file));
    g_signal_connect (page->gsettings, "changed::setup-environment-variables",
                      G_CALLBACK (on_setup_environment), NULL);

    g_object_unref (file);
    g_free (xprofile);
    g_free (conf);
  }

  g_strfreev (keys);
  g_list_free (key_list);
  g_settings_schema_unref (schema);

  return page;
}

static void
on_destroy (GtkWidget *widget, GApplication *app)
{
  g_application_release (app);
}

static void
build_xkb_options (GtkToggleButton *toggle_button,
                   NimfXkb         *xkb)
{
  if (gtk_toggle_button_get_active (toggle_button))
  {
    gchar *name = g_strdup (gtk_widget_get_name (GTK_WIDGET (toggle_button)));
    xkb->options_len += 1;
    xkb->options = g_realloc_n (xkb->options,
                                xkb->options_len + 1,
                                sizeof (gchar *));
    xkb->options[xkb->options_len - 1] = name;
    xkb->options[xkb->options_len]     = NULL;
  }
}

static void
on_toggled (GtkToggleButton *toggle_button,
            NimfXkb         *xkb)
{
  if (GTK_IS_RADIO_BUTTON (toggle_button) &&
      !gtk_toggle_button_get_active (toggle_button))
    return;

  if (xkb->options_len > 0)
  {
    xkb->options_len = 0;
    xkb->options = g_realloc_n (xkb->options, 1, sizeof (gchar *));
    xkb->options[0] = NULL;
  }

  g_slist_foreach (xkb->toggle_buttons, (GFunc) build_xkb_options, xkb);

  if (gnome_xkb_is_available () && gnome_is_running ())
  {
    GSettings *settings;

    settings = g_settings_new ("org.gnome.desktop.input-sources");
    g_settings_set_strv (settings, "xkb-options",
                         (const gchar *const *) xkb->options);

    g_object_unref (settings);
  }
  else if (g_strcmp0 (g_getenv ("XDG_SESSION_TYPE"), "x11") == 0)
  {
    XklConfigRec *rec;
    GSettings    *settings;

    rec = xkl_config_rec_new ();
    xkl_config_rec_get_from_server (rec, xkb->engine);
    g_strfreev (rec->options);
    rec->options = g_strdupv (xkb->options);
    xkl_config_rec_activate (rec, xkb->engine);

    settings = g_settings_new ("org.nimf.settings");
    g_settings_set_strv (settings, "xkb-options",
                         (const gchar *const *) xkb->options);

    g_object_unref (settings);
    g_object_unref (rec);
  }
}

static void
configure_button (GtkWidget *button, const XklConfigItem *item, NimfXkb *xkb)
{
  gtk_widget_set_name (button, item->name);

  if (g_strv_contains ((const gchar * const *) xkb->options, item->name))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

  g_signal_connect (button, "toggled", G_CALLBACK (on_toggled), xkb);
  xkb->toggle_buttons = g_slist_append (xkb->toggle_buttons, button);
  gtk_box_pack_start (GTK_BOX (xkb->vbox), button, FALSE, FALSE, 0);
}

static void
build_check_button_option (XklConfigRegistry   *config,
                           const XklConfigItem *item,
                           NimfXkb             *xkb)
{
  configure_button (gtk_check_button_new_with_label (item->description),
                    item, xkb);
}

static void
build_radio_button_option (XklConfigRegistry   *config,
                           const XklConfigItem *item,
                           NimfXkb             *xkb)
{
  GtkWidget *radio_button;
  radio_button = gtk_radio_button_new_with_label (xkb->radio_group,
                                                  item->description);
  xkb->radio_group  = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_button));
  configure_button (radio_button, item, xkb);
}

/* XKB 옵션 그룹 아코디언 동작: 하나만 펼쳐지도록 콜백 */
static void
on_xkb_expander_notify_expanded (GtkExpander *expander,
                                 GParamSpec  *pspec,
                                 gpointer     user_data)
{
  NimfXkb *xkb = (NimfXkb *) user_data;

  if (!gtk_expander_get_expanded (expander))
    return;

  for (GSList *l = xkb->expanders; l; l = l->next)
  {
    if (l->data != expander)
      gtk_expander_set_expanded (GTK_EXPANDER (l->data), FALSE);
  }
}

static void
build_option_group (XklConfigRegistry   *config,
                    const XklConfigItem *item,
                    NimfXkb             *xkb)
{
  xkb->radio_group = NULL;
  GtkWidget *expander = gtk_expander_new (item->description);
  xkb->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_margin_start  (xkb->vbox, 15);
  gtk_container_add (GTK_CONTAINER (expander), xkb->vbox);
  gtk_box_pack_start (GTK_BOX (xkb->options_box), expander, FALSE, FALSE, 0);

  /* 아코디언: 하나만 펼쳐지도록 관리 */
  xkb->expanders = g_slist_append (xkb->expanders, expander);
  g_signal_connect (expander, "notify::expanded",
                    G_CALLBACK (on_xkb_expander_notify_expanded), xkb);

  if (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), XCI_PROP_ALLOW_MULTIPLE_SELECTION)))
  {
    xkl_config_registry_foreach_option (config, item->name,
                                        (XklConfigItemProcessFunc) build_check_button_option,
                                        xkb);
  }
  else
  {
    GtkWidget *radio_button;
    radio_button = gtk_radio_button_new_with_label (xkb->radio_group,
                                                    _("Default"));
    g_signal_connect (radio_button, "toggled", G_CALLBACK (on_toggled), xkb);
    xkb->radio_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_button));
    gtk_box_pack_start (GTK_BOX (xkb->vbox), radio_button, FALSE, FALSE, 0);
    xkl_config_registry_foreach_option (config, item->name,
                                        (XklConfigItemProcessFunc) build_radio_button_option,
                                        xkb);
  }
}

static void
nimf_xkb_free (NimfXkb *xkb)
{
  g_strfreev (xkb->options);
/*
  if (xkb->engine)
    g_object_unref  (xkb->engine);
*/
  g_slist_free (xkb->toggle_buttons);
  g_slist_free (xkb->radio_group);
  g_slist_free (xkb->expanders);
  g_slice_free (NimfXkb, xkb);
}

static GtkWidget *
nimf_settings_build_xkb_options_ui ()
{
  NimfXkb           *xkb;
  XklConfigRegistry *config_registry;
  GSettings         *settings;

  if (gnome_xkb_is_available () && gnome_is_running ())
    settings = g_settings_new ("org.gnome.desktop.input-sources");
  else if (g_strcmp0 (g_getenv ("XDG_SESSION_TYPE"), "x11") == 0)
    settings = g_settings_new ("org.nimf.settings");
  else
    return NULL;

  xkb = g_slice_new0 (NimfXkb);
  xkb->options = g_settings_get_strv (settings, "xkb-options");
  xkb->options_len = g_strv_length (xkb->options);
  xkb->options_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  g_object_set_data_full (G_OBJECT (xkb->options_box), "xkb", xkb,
                          (GDestroyNotify) nimf_xkb_free);
  gtk_widget_set_margin_start  (xkb->options_box, 15);
  gtk_widget_set_margin_end    (xkb->options_box, 15);
  gtk_widget_set_margin_top    (xkb->options_box, 15);
  gtk_widget_set_margin_bottom (xkb->options_box, 15);

  /* Check if we're running on X11 before using X11-specific functions */
  if (GDK_IS_X11_DISPLAY (gdk_display_get_default ()))
  {
    xkb->engine =
      xkl_engine_get_instance (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()));
    if (xkb->engine)
    {
      config_registry = xkl_config_registry_get_instance (xkb->engine);
      xkl_config_registry_load (config_registry, TRUE);
      xkl_config_registry_foreach_option_group (config_registry,
                                                (ConfigItemProcessFunc) build_option_group,
                                                xkb);
      g_object_unref (config_registry);
    }
  }
  g_object_unref (settings);

  /* 스크롤 컨테이너로 감싸서 높이를 제한 */
  GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);
  xkb->scrolled = scrolled;
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
#if GTK_CHECK_VERSION(3,16,0)
  gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (scrolled), 220);
#endif
  gtk_widget_set_size_request (scrolled, -1, 260);
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), xkb->options_box);
#else
  gtk_container_add (GTK_CONTAINER (scrolled), xkb->options_box);
#endif

  return scrolled;
}

static void
on_row_selected (GtkListBox    *box,
                 GtkListBoxRow *row,
                 gpointer       user_data)
{
  NimfSettingsPage *page;
  const gchar      *schema_id;
  GtkWidget        *content = user_data;
  GtkWidget        *child;

#if GTK_CHECK_VERSION(4, 0, 0)
  if ((child = gtk_scrolled_window_get_child (GTK_SCROLLED_WINDOW (content))))
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (content), NULL);
#else
  if ((child = gtk_bin_get_child (GTK_BIN (content))))
    gtk_container_remove (GTK_CONTAINER (content), child);
#endif

  schema_id = gtk_widget_get_name (GTK_WIDGET (row));

  if (schema_id == NULL || *schema_id == '\0')
  {
    g_warning ("Empty schema_id for row widget, skipping");
    return;
  }

  if (g_strcmp0 (schema_id, "xkb-options"))
  {
    page = nimf_settings_page_new (schema_id);
    if (page != NULL)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
      gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (content), page->box);
#else
      gtk_container_add (GTK_CONTAINER (content), page->box);
#endif
    }
  }
  else
  {
    GtkWidget *xkb = nimf_settings_build_xkb_options_ui ();
    if (xkb)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
      gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (content), xkb);
#else
      gtk_container_add (GTK_CONTAINER (content), xkb);
#endif
    }
  }

  gtk_widget_show_all (content);
}

/* -----------------------------
 * 새 UI 구성 요소 (GTK3/GTK4 겸용)
 * - 좌측 사이드바는 범주 단위로만 구성하고,
 *   각 범주에 해당하는 페이지를 스택으로 전환한다.
 * ----------------------------- */

static GtkWidget *
build_group_title (const gchar *title)
{
  GtkWidget *label = gtk_label_new (NULL);
  gchar *markup = g_strdup_printf ("<span weight=\"bold\" size=\"large\">%s</span>", title);
  gtk_label_set_markup (GTK_LABEL (label), markup);
  gtk_widget_set_halign (label, GTK_ALIGN_START);
#if GTK_CHECK_VERSION(3,12,0)
  gtk_widget_set_margin_top (label, 10);
  gtk_widget_set_margin_bottom (label, 6);
#endif
  g_free (markup);
  return label;
}

static GtkWidget *
nimf_settings_build_keyboard_page (void)
{
  GtkWidget *page;
  GtkWidget *section;

  page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 15);
#if GTK_CHECK_VERSION (3, 12, 0)
  gtk_widget_set_margin_start  (page, 15);
  gtk_widget_set_margin_end    (page, 15);
#else
  gtk_widget_set_margin_left   (page, 15);
  gtk_widget_set_margin_right  (page, 15);
#endif
  gtk_widget_set_margin_top    (page, 15);
  gtk_widget_set_margin_bottom (page, 15);

  /* 전역 설정: 단축키 등 */
  section = build_group_title (_("기본 설정"));
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_box_append (GTK_BOX (page), section);
#else
  gtk_box_pack_start (GTK_BOX (page), section, FALSE, FALSE, 0);
#endif
  {
    NimfSettingsPage *general = nimf_settings_page_new ("org.nimf");
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_box_append (GTK_BOX (page), general->box);
#else
    gtk_box_pack_start (GTK_BOX (page), general->box, FALSE, FALSE, 0);
#endif
  }

  /* 기본 엔진 선택 섹션 */
  section = build_group_title (_("기본 엔진"));
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_box_append (GTK_BOX (page), section);
#else
  gtk_box_pack_start (GTK_BOX (page), section, FALSE, FALSE, 0);
#endif
  {
    NimfSettingsPage *engines_root = nimf_settings_page_new ("org.nimf.engines");
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_box_append (GTK_BOX (page), engines_root->box);
#else
    gtk_box_pack_start (GTK_BOX (page), engines_root->box, FALSE, FALSE, 0);
#endif
  }

  /* XKB 옵션 섹션 */
  section = build_group_title (_("XKB 옵션"));
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_box_append (GTK_BOX (page), section);
#else
  gtk_box_pack_start (GTK_BOX (page), section, FALSE, FALSE, 0);
#endif
  {
    GtkWidget *xkb = nimf_settings_build_xkb_options_ui ();
    if (xkb)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
      gtk_box_append (GTK_BOX (page), xkb);
#else
      gtk_box_pack_start (GTK_BOX (page), xkb, FALSE, FALSE, 0);
#endif
    }
  }

  return page;
}

typedef struct
{
  gchar *schema_id;
} EngineDialogData;

static void
on_engine_settings_clicked (GtkButton *button, gpointer user_data)
{
  EngineDialogData *data = (EngineDialogData *) user_data;
  NimfSettingsPage *page = nimf_settings_page_new (data->schema_id);

  GtkWidget *dialog;
  GtkWidget *content_area;
#if GTK_CHECK_VERSION (3, 12, 0)
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR;
#else
  GtkDialogFlags flags = GTK_DIALOG_MODAL;
#endif
  dialog = gtk_dialog_new_with_buttons (_("엔진 설정"),
                                        GTK_WINDOW (nimf_settings_window),
                                        flags,
                                        _("_Close"), GTK_RESPONSE_CLOSE,
                                        NULL);
  gtk_widget_set_size_request (dialog, 560, 420);
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_box_append (GTK_BOX (content_area), page->box);
#else
  gtk_box_pack_start (GTK_BOX (content_area), page->box, TRUE, TRUE, 0);
#endif
  gtk_widget_show_all (content_area);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
  g_free (data->schema_id);
  g_slice_free (EngineDialogData, data);
}

static void
on_engine_active_toggled (GtkSwitch *sw, GParamSpec *pspec, gpointer user_data)
{
  const gchar *schema_id = (const gchar *) user_data;
  gboolean active = gtk_switch_get_active (sw);

  GSettingsSchemaSource *source = g_settings_schema_source_get_default ();
  GSettingsSchema *schema = nimf_settings_lookup_schema_with_fallback (source, schema_id);
  
  if (schema == NULL)
  {
    g_warning ("Schema '%s' is not installed, skipping activation toggle", schema_id);
    return;
  }

  GSettings *gsettings = g_settings_new (schema_id);
  if (g_settings_schema_has_key (schema, "active-engine"))
    g_settings_set_boolean (gsettings, "active-engine", active);

  g_settings_schema_unref (schema);
  g_object_unref (gsettings);
}

static GtkWidget *
nimf_settings_build_engines_page (void)
{
  GtkWidget *outer = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
#if GTK_CHECK_VERSION (3, 12, 0)
  gtk_widget_set_margin_start  (outer, 15);
  gtk_widget_set_margin_end    (outer, 15);
#else
  gtk_widget_set_margin_left   (outer, 15);
  gtk_widget_set_margin_right  (outer, 15);
#endif
  gtk_widget_set_margin_top    (outer, 15);
  gtk_widget_set_margin_bottom (outer, 15);

  GtkWidget *title = build_group_title (_("언어 엔진"));
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_box_append (GTK_BOX (outer), title);
#else
  gtk_box_pack_start (GTK_BOX (outer), title, FALSE, FALSE, 0);
#endif

  GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  GtkWidget *list = gtk_list_box_new ();
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), list);
  gtk_box_append (GTK_BOX (outer), scrolled);
#else
  gtk_container_add (GTK_CONTAINER (scrolled), list);
  gtk_box_pack_start (GTK_BOX (outer), scrolled, TRUE, TRUE, 0);
#endif

  /* 엔진 스키마를 열거하여 한 페이지로 나열 */
  GSettingsSchemaSource *source = g_settings_schema_source_get_default ();
  gchar **schemas = NULL;
  g_settings_schema_source_list_schemas (source, TRUE, &schemas, NULL);
  for (gint i = 0; schemas[i] != NULL; i++)
  {
    const gchar *schema_id = schemas[i];
    if (g_str_has_prefix (schema_id, "org.nimf.engines.") == FALSE)
      continue;

    GSettingsSchema *schema = nimf_settings_lookup_schema_with_fallback (source, schema_id);
    
    if (schema == NULL)
      continue;

    GSettings *gs = g_settings_new (schema_id);
    gchar *name = g_settings_get_string (gs, "hidden-schema-name");

    GtkWidget *row = gtk_list_box_row_new ();
    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label = gtk_label_new (name ? name : schema_id);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_hexpand (label, TRUE);

    GtkWidget *settings_btn = gtk_button_new_with_label (_("설정…"));
    EngineDialogData *data = g_slice_new0 (EngineDialogData);
    data->schema_id = g_strdup (schema_id);
    g_signal_connect (settings_btn, "clicked",
                      G_CALLBACK (on_engine_settings_clicked), data);

    // Only show toggle switch for engines that can be activated/deactivated
    GtkWidget *active_sw = NULL;
    if (g_settings_schema_has_key (schema, "active-engine"))
    {
      active_sw = gtk_switch_new ();
      gboolean active = g_settings_get_boolean (gs, "active-engine");
      gtk_switch_set_active (GTK_SWITCH (active_sw), active);
      gchar *schema_id_copy = g_strdup (schema_id);
      g_signal_connect_data (active_sw, "notify::active",
                             G_CALLBACK (on_engine_active_toggled), schema_id_copy,
                             (GClosureNotify) g_free, 0);
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_box_append (GTK_BOX (hbox), label);
    if (active_sw)
      gtk_box_append (GTK_BOX (hbox), active_sw);
    gtk_box_append (GTK_BOX (hbox), settings_btn);
    gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (row), hbox);
#else
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
    gtk_box_pack_end   (GTK_BOX (hbox), settings_btn, FALSE, FALSE, 0);
    if (active_sw)
      gtk_box_pack_end   (GTK_BOX (hbox), active_sw, FALSE, FALSE, 0);
    gtk_container_add  (GTK_CONTAINER (row), hbox);
#endif

    gtk_list_box_insert (GTK_LIST_BOX (list), row, -1);

    g_free (name);
    g_object_unref (gs);
    g_settings_schema_unref (schema);
  }
  g_strfreev (schemas);

  return outer;
}

static GtkWidget *
nimf_settings_build_clients_page (void)
{
  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 15);
#if GTK_CHECK_VERSION (3, 12, 0)
  gtk_widget_set_margin_start  (page, 15);
  gtk_widget_set_margin_end    (page, 15);
#else
  gtk_widget_set_margin_left   (page, 15);
  gtk_widget_set_margin_right  (page, 15);
#endif
  gtk_widget_set_margin_top    (page, 15);
  gtk_widget_set_margin_bottom (page, 15);

  struct { const gchar *schema; const gchar *title; } sections[] = {
    { "org.nimf.clients.gtk", _("GTK+") },
    { "org.nimf.clients.qt5", _("Qt5") },
    { "org.nimf.clients.qt6", _("Qt6") },
  };

  for (guint i = 0; i < G_N_ELEMENTS (sections); i++)
  {
    GtkWidget *title = build_group_title (sections[i].title);
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_box_append (GTK_BOX (page), title);
#else
    gtk_box_pack_start (GTK_BOX (page), title, FALSE, FALSE, 0);
#endif
    NimfSettingsPage *sec = nimf_settings_page_new (sections[i].schema);
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_box_append (GTK_BOX (page), sec->box);
#else
    gtk_box_pack_start (GTK_BOX (page), sec->box, FALSE, FALSE, 0);
#endif
  }

  return page;
}

static void
append_xkb_menu_after_nimf_menu (GtkWidget *listbox)
{
  GtkWidget *label;
  GtkWidget *row;
  label = gtk_label_new (_("XKB Options"));
  row   = gtk_list_box_row_new ();
  gtk_widget_set_name (row, "xkb-options");
  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), FALSE);
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (row), label);
#else
  gtk_container_add (GTK_CONTAINER (row), label);
#endif
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_margin_start  (label, 15);
  gtk_widget_set_margin_end    (label, 15);
  gtk_widget_set_margin_top    (label, 5);
  gtk_widget_set_margin_bottom (label, 5);
  gtk_list_box_insert (GTK_LIST_BOX (listbox), row, 1);
}

static GtkWidget *
nimf_settings_build_main_window (NimfSettings *nsettings)
{
  GtkWidget  *window;
  GtkWidget  *sidebar;
  GtkWidget  *stack;
  GtkWidget  *container;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 880, 520);
  gtk_window_set_title        (GTK_WINDOW (window), _("Nimf Settings"));
  gtk_window_set_icon_name    (GTK_WINDOW (window), "nimf-logo");

  /* 스택과 사이드바 구성 */
#if GTK_CHECK_VERSION(4, 0, 0)
  stack = gtk_stack_new ();
  gtk_stack_set_transition_type (GTK_STACK (stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  GtkWidget *sidebar_widget = gtk_stack_sidebar_new (); /* GTK4도 동일 명칭 */
  gtk_stack_sidebar_set_stack (GTK_STACK_SIDEBAR (sidebar_widget), GTK_STACK (stack));
#else
  stack = gtk_stack_new ();
  gtk_stack_set_transition_type (GTK_STACK (stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  GtkWidget *sidebar_widget = gtk_stack_sidebar_new ();
  gtk_stack_sidebar_set_stack (GTK_STACK_SIDEBAR (sidebar_widget), GTK_STACK (stack));
#endif

  /* 페이지 추가: 키보드, 클라이언트, 언어 엔진 */
  GtkWidget *page_keyboard = nimf_settings_build_keyboard_page ();
  GtkWidget *page_clients  = nimf_settings_build_clients_page ();
  GtkWidget *page_engines  = nimf_settings_build_engines_page ();

#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_stack_add_titled (GTK_STACK (stack), page_keyboard, "keyboard", _("키보드"));
  gtk_stack_add_titled (GTK_STACK (stack), page_clients,  "clients",  _("클라이언트"));
  gtk_stack_add_titled (GTK_STACK (stack), page_engines,  "engines",  _("언어 엔진"));
#else
  gtk_stack_add_titled (GTK_STACK (stack), page_keyboard, "keyboard", _("키보드"));
  gtk_stack_add_titled (GTK_STACK (stack), page_clients,  "clients",  _("클라이언트"));
  gtk_stack_add_titled (GTK_STACK (stack), page_engines,  "engines",  _("언어 엔진"));
#endif

  /* 레이아웃 배치 */
  sidebar = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sidebar),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (sidebar), sidebar_widget);
#else
  gtk_container_add (GTK_CONTAINER (sidebar), sidebar_widget);
#endif

  container = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_widget_set_size_request (sidebar, SIDEBAR_MIN_WIDTH, -1);
  gtk_box_append (GTK_BOX (container), sidebar);
  gtk_box_append (GTK_BOX (container), stack);
  gtk_window_set_child (GTK_WINDOW (window), container);
#else
  gtk_widget_set_size_request (sidebar, SIDEBAR_MIN_WIDTH, -1);
  gtk_box_pack_start (GTK_BOX (container), sidebar, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (container), stack, TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (window), container);
#endif

  g_signal_connect (window, "destroy",
                    G_CALLBACK (on_destroy), nsettings->app);

  return window;
}

static void
nimf_settings_open_help (GtkAccelGroup *accel,
           GObject *acceleratoable,
           guint keyval,
           GdkModifierType modifier,
           gpointer user_data)
{
  gtk_show_uri_on_window (GTK_WINDOW(user_data), "https://github.com/hamonikr/nimf/wiki",
                          gtk_get_current_event_time (),NULL);
}

static void
nimf_settings_accel_init(NimfSettings *nsettings)
{
  GtkAccelGroup *accel_group;
  guint accelerator_key;
  GdkModifierType accelerator_mod;
  GClosure *closure;
  GtkWindow *window = GTK_WINDOW(nimf_settings_window);
  
  accel_group = gtk_accel_group_new();
  gtk_accelerator_parse ( "F1", &accelerator_key, &accelerator_mod);
  closure = g_cclosure_new_object (G_CALLBACK (nimf_settings_open_help),G_OBJECT(window));
  gtk_accel_group_connect (accel_group, accelerator_key,accelerator_mod, GTK_ACCEL_VISIBLE, closure);
  gtk_window_add_accel_group (window, accel_group);
}

static void
on_activate (GApplication *app, NimfSettings *nsettings)
{
  g_application_hold (app);

  if (nimf_settings_window)
  {
    gtk_window_present (GTK_WINDOW (nimf_settings_window));
    g_application_release (app);

    return;
  }

  nimf_settings_window = nimf_settings_build_main_window (nsettings);
  nimf_settings_accel_init(nsettings);
  gtk_widget_show_all (nimf_settings_window);
}

static int
nimf_settings_run (NimfSettings  *nsettings,
                   int            argc,
                   char         **argv)
{
  return g_application_run (G_APPLICATION (nsettings->app), argc, argv);
}

static NimfSettings *
nimf_settings_new ()
{
  return g_object_new (NIMF_TYPE_SETTINGS, NULL);
}

static void
nimf_settings_init (NimfSettings *nsettings)
{
  nsettings->app = g_application_new ("org.nimf.settings",
#if GLIB_CHECK_VERSION(2, 74, 0)
                                      G_APPLICATION_DEFAULT_FLAGS);
#else
                                      G_APPLICATION_FLAGS_NONE);
#endif
  g_signal_connect (nsettings->app, "activate",
                    G_CALLBACK (on_activate), nsettings);
}

static void
nimf_settings_finalize (GObject *object)
{
  g_object_unref (NIMF_SETTINGS (object)->app);

  G_OBJECT_CLASS (nimf_settings_parent_class)->finalize (object);
}

static void
nimf_settings_class_init (NimfSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = nimf_settings_finalize;
}

/**
 * SECTION:nimf-settings
 * @title: nimf-settings
 * @section_id: nimf-settings
 */

/**
 * PROGRAM:nimf-settings
 * @short_description: Tool for setting up Nimf
 * @synopsis: nimf-settings [*OPTIONS*...]
 * @see_also: nimf(1)
 * @-h, --help: Show help messages
 * @--help-all: Show all options
 * @--help-gapplication: Show GApplication options
 * @--gapplication-service: Enter GApplication service mode. Currently this
 *                          option is used to trigger the nimf indicator.
 *
 * nimf-settings is a GUI tool for configuring nimf.
 *
 * Returns: 0 on success, non-zero on failure
 */
int main (int argc, char **argv)
{
  NimfSettings *nsettings;
  int status;

  g_setenv ("GTK_IM_MODULE", "gtk-im-context-simple", TRUE);
  
  const gchar *display_name = g_getenv ("DISPLAY");
  const gchar *wayland_display = g_getenv ("WAYLAND_DISPLAY");
  
  if (display_name && !wayland_display)
  {
    g_setenv ("GDK_BACKEND", "x11", TRUE);
  }

  /* Ensure GLib can find compiled schemas even when XDG_DATA_DIRS is misconfigured.
   * If the app is started without GSETTINGS_SCHEMA_DIR and default lookup fails
   * in some environments, explicitly point to the system schemas directory. */
  if (g_getenv ("GSETTINGS_SCHEMA_DIR") == NULL)
  {
    const gchar *system_schema_dir = "/usr/share/glib-2.0/schemas";
    gchar       *compiled = g_build_filename (G_DIR_SEPARATOR_S,
                                              system_schema_dir,
                                              "gschemas.compiled",
                                              NULL);
    if (g_file_test (compiled, G_FILE_TEST_EXISTS))
      g_setenv ("GSETTINGS_SCHEMA_DIR", system_schema_dir, TRUE);
    g_free (compiled);
  }

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, NIMF_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gtk_init (&argc, &argv);

  nsettings = nimf_settings_new ();
  status = nimf_settings_run (nsettings, argc, argv);

  g_object_unref (nsettings);

  return status;
}
