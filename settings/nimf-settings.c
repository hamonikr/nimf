/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-settings.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2016 Hodong Kim <cogniti@gmail.com>
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
#include "nimf.h"

#define NIMF_TYPE_SETTINGS             (nimf_settings_get_type ())
#define NIMF_SETTINGS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_SETTINGS, NimfSettings))
#define NIMF_SETTINGS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_SETTINGS, NimfSettingsClass))
#define NIMF_IS_SETTINGS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_SETTINGS))
#define NIMF_IS_SETTINGS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_SETTINGS))
#define NIMF_SETTINGS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_SETTINGS, NimfSettingsClass))

typedef struct _NimfSettings      NimfSettings;
typedef struct _NimfSettingsClass NimfSettingsClass;

struct _NimfSettings
{
  GObject parent_instance;

  GtkApplication        *app;
  GPtrArray             *pages;
  GSettingsSchemaSource *schema_source; /* do not free */
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
  GtkWidget *label;
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

static void
on_gsettings_changed (GSettings *settings,
                      gchar     *key,
                      GtkWidget *widget)
{
  if (GTK_IS_SWITCH (widget))
  {
    gboolean state = g_settings_get_boolean (settings, key);
    gtk_switch_set_active (GTK_SWITCH (widget), state);
  }
  else if (GTK_IS_COMBO_BOX (widget))
  {
    gchar *id;

    id = g_settings_get_string (settings, key);
    gtk_combo_box_set_active_id (GTK_COMBO_BOX (widget), id);

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
  return g_strcmp0 (a, b);
}

static void
on_combo_box_changd (GtkComboBox         *widget,
                     NimfSettingsPageKey *page_key)
{
  const gchar *id;

  id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (widget));
  g_settings_set_string (page_key->gsettings, page_key->key, id);
}

static gboolean
on_switch_state_set (GtkSwitch           *widget,
                     gboolean             state,
                     NimfSettingsPageKey *page_key)
{
  g_settings_set_boolean (page_key->gsettings, page_key->key, state);
  gtk_switch_set_state (GTK_SWITCH (widget), state);

  return TRUE;
}

void
on_cursor_changed (GtkTreeView *tree_view,
                   GtkStack    *stack)
{
  GtkTreeSelection *selection; /* do not free */
  GtkTreeModel *model;
  gchar        *text = NULL;
  GtkTreeIter iter;

  selection = gtk_tree_view_get_selection (tree_view);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    gtk_tree_model_get (model, &iter, SCHEMA_COLUMN, &text, -1);

  if (text)
    gtk_stack_set_visible_child_name (stack, text);
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
nimf_settings_page_key_build_boolean (NimfSettingsPageKey *page_key)
{
  GtkWidget *gswitch;
  GtkWidget *hbox;
  gchar     *detailed_signal;
  gboolean   is_active;

  gswitch = gtk_switch_new ();
  is_active = g_settings_get_boolean (page_key->gsettings, page_key->key);
  gtk_switch_set_active (GTK_SWITCH (gswitch), is_active);
  gtk_widget_set_halign  (gswitch, GTK_ALIGN_END);
  detailed_signal = g_strdup_printf ("changed::%s", page_key->key);

  g_signal_connect (gswitch, "state-set",
                    G_CALLBACK (on_switch_state_set), page_key);
  g_signal_connect (page_key->gsettings, detailed_signal,
                    G_CALLBACK (on_gsettings_changed), gswitch);

  g_free (detailed_signal);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 15);
  gtk_box_pack_start (GTK_BOX (hbox), page_key->label, FALSE, FALSE, 0);
  gtk_box_pack_end   (GTK_BOX (hbox), gswitch, FALSE, FALSE, 0);

  return hbox;
}

static GtkWidget *
nimf_settings_page_key_build_string (NimfSettingsPageKey *page_key,
                                     const gchar         *schema_id,
                                     GList               *key_list)
{
  GtkListStore *store;
  GtkWidget    *combo;
  GtkWidget    *hbox;
  gchar        *detailed_signal;
  GtkTreeIter   iter;

  store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
  combo = gtk_combo_box_text_new ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));
  g_object_unref (store);
  gtk_combo_box_set_id_column (GTK_COMBO_BOX (combo), 1);
  gtk_tree_model_get_iter_first ((GtkTreeModel *) store, &iter);

  if (g_strcmp0 (schema_id, "org.nimf.engines") == 0 &&
      g_strcmp0 (page_key->key, "default-engine") == 0)
  {
    gchar                *engine_id;
    GDir                 *dir;
    const gchar          *filename;
    const NimfEngineInfo *info;
    GError               *error = NULL;
    NimfEngineInfo *(* module_get_info) (void);

    dir = g_dir_open (NIMF_MODULE_DIR, 0, &error);

    if (error)
      g_error (G_STRLOC ": %s: %s", G_STRFUNC, error->message);

    engine_id = g_settings_get_string (page_key->gsettings, page_key->key);

    while ((filename = g_dir_read_name (dir)))
    {
      gchar   *path;
      GModule *library;

      path = g_build_path (G_DIR_SEPARATOR_S, NIMF_MODULE_DIR, filename, NULL);
      library = g_module_open (path, G_MODULE_BIND_LAZY |
                                     G_MODULE_BIND_LOCAL);

      if (!g_module_symbol (library, "module_get_info",
                            (gpointer *) &module_get_info))
        g_error (G_STRLOC ": %s", g_module_error ());

      info = module_get_info ();
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, info->engine_name,
                                        1, info->engine_id, -1);

      if (g_strcmp0 (engine_id, info->engine_id) == 0)
        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);

      g_module_close (library);
      g_free (path);
    }

    g_dir_close (dir);
    g_free (engine_id);
  }
  else if (g_str_has_prefix (page_key->key, "hidden-") == FALSE)
  {
    gchar *id1;
    GList *list;

    id1 = g_settings_get_string (page_key->gsettings, page_key->key);

    for (list = key_list; list != NULL; list = list->next)
    {
      gchar *key2;
      gchar *prefix;

      key2 = list->data;
      prefix = g_strdup_printf ("hidden-%s-", page_key->key);

      if (g_str_has_prefix (key2, prefix))
      {
        gchar *val;
        const gchar *id2 = key2 + strlen (prefix);

        val = g_settings_get_string (page_key->gsettings, key2);
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, val,
                                          1, id2, -1);

        if (g_strcmp0 (id1, id2) == 0)
          gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);

        g_free (val);
      }

      g_free (prefix);
    }

    g_free (id1);
  }

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 15);
  gtk_box_pack_start (GTK_BOX (hbox), page_key->label, FALSE, FALSE, 0);
  gtk_box_pack_end   (GTK_BOX (hbox), combo, FALSE, FALSE, 0);
  detailed_signal = g_strdup_printf ("changed::%s", page_key->key);

  g_signal_connect (combo, "changed",
                    G_CALLBACK (on_combo_box_changd), page_key);
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
  const gchar *keystr;
  GString     *string;
  gchar       *combination;
  GFlagsClass *flags_class; /* do not free */
  guint        mod;
  guint        i;

  string = g_string_new ("");
  mod = event->key.state & NIMF_MODIFIER_MASK;
  flags_class = (GFlagsClass *) g_type_class_ref (NIMF_TYPE_MODIFIER_TYPE);

  for (i = 1; i <= (1 << 30); i = i << 1)
  {
    GFlagsValue *flags_value = g_flags_get_first_value (flags_class, mod & i);

    if (flags_value)
      g_string_append_printf (string, "%s ", flags_value->value_nick);
  }

  g_type_class_unref (flags_class);

  keystr = nimf_keyval_to_string (event->key.keyval);

  if (keystr == NULL)
  {
    gchar *text;

    g_string_free (string, TRUE);
    gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
                                       GTK_RESPONSE_ACCEPT, FALSE);
    text = g_strdup_printf (_("Please report a bug. (keyval: %d)"),
                            event->key.keyval);
    gtk_entry_set_text (GTK_ENTRY (widget), text);

    g_free (text);

    g_return_val_if_fail (keystr, TRUE);
  }

  g_string_append_printf (string, "%s", keystr);
  combination = g_string_free (string, FALSE);
  gtk_entry_set_text (GTK_ENTRY (widget), combination);
  gtk_editable_set_position (GTK_EDITABLE (widget), -1);
  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
                                     GTK_RESPONSE_ACCEPT, TRUE);
  g_free (combination);

  return TRUE;
}

static void
nimf_settings_page_key_update_gsettings_strv (NimfSettingsPageKey *page_key,
                                              GtkTreeModel        *model)
{
  gchar       **vals;
  GtkTreeIter   iter;
  gint          i;

  vals = g_malloc0_n (sizeof (gchar *), 1);

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

  dialog = gtk_dialog_new_with_buttons (_("Press key combination..."),
                                        GTK_WINDOW (nimf_settings_window),
                                        GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR,
                                        _("_OK"),     GTK_RESPONSE_ACCEPT,
                                        _("_Cancel"), GTK_RESPONSE_REJECT,
                                        NULL);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "nimf");
  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
                                     GTK_RESPONSE_ACCEPT, FALSE);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (entry), _("Press the combination of the keys"));
  gtk_editable_set_position (GTK_EDITABLE (entry), -1);
  gtk_container_add (GTK_CONTAINER (content_area), entry);
  g_signal_connect (entry, "key-press-event",
                    G_CALLBACK (on_key_press_event), dialog);

  gtk_widget_show_all (content_area);

  switch (gtk_dialog_run (GTK_DIALOG (dialog)))
  {
    case GTK_RESPONSE_ACCEPT:
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

  button1 = gtk_button_new_from_icon_name ("gtk-add",    GTK_ICON_SIZE_SMALL_TOOLBAR);
  button2 = gtk_button_new_from_icon_name ("gtk-remove", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_button_set_relief (GTK_BUTTON (button1), GTK_RELIEF_NONE);
  gtk_button_set_relief (GTK_BUTTON (button2), GTK_RELIEF_NONE);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_margin_end (page_key->label, 15);
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
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_w),
                                       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_w),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_container_add (GTK_CONTAINER (scrolled_w), treeview);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox,       FALSE, FALSE, 0);
  gtk_box_pack_end   (GTK_BOX (vbox), scrolled_w, FALSE, FALSE, 0);

  page_key->treeview = treeview;
  detailed_signal = g_strdup_printf ("changed::%s", page_key->key);

  g_signal_connect (button1, "clicked", G_CALLBACK (on_button_clicked_add), page_key);
  g_signal_connect (button2, "clicked", G_CALLBACK (on_button_clicked_remove), page_key);
  g_signal_connect (page_key->gsettings, detailed_signal,
                    G_CALLBACK (on_gsettings_changed), page_key->treeview);
  g_strfreev (strv);
  g_free (detailed_signal);

  return vbox;
}

static GtkWidget *
nimf_settings_page_build_label (NimfSettingsPage *page, const gchar *schema_id)
{
  GString   *string;
  gchar     *str;
  GtkWidget *tab_label;
  gchar     *p;

  str = g_settings_get_string (page->gsettings, "hidden-schema-name");
  string = g_string_new (str);
  g_free (str);

  for (p = (gchar *) schema_id; *p != 0; p++)
    if (*p == '.')
      g_string_prepend (string, "  ");

  str = g_string_free (string, FALSE);
  tab_label = gtk_label_new (str);
  gtk_widget_set_halign (tab_label, GTK_ALIGN_START);

  g_free (str);

  return tab_label;
}

static NimfSettingsPage *
nimf_settings_page_new (NimfSettings *nsettings,
                        const gchar  *schema_id)
{
  NimfSettingsPage *page;
  GSettingsSchema  *schema;
  GList            *key_list = NULL;
  gchar           **keys;
  GList            *l;
  gint              i;

  page = g_slice_new0 (NimfSettingsPage);
  page->gsettings = g_settings_new (schema_id);
  page->label = nimf_settings_page_build_label (page, schema_id);
  page->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 15);
  page->page_keys = g_ptr_array_new_with_free_func ((GDestroyNotify) nimf_settings_page_key_free);
  gtk_widget_set_margin_start  (page->box, 15);
  gtk_widget_set_margin_end    (page->box, 15);
  gtk_widget_set_margin_top    (page->box, 15);
  gtk_widget_set_margin_bottom (page->box, 15);

  schema = g_settings_schema_source_lookup (nsettings->schema_source,
                                            schema_id, TRUE);
  keys = g_settings_list_keys (page->gsettings);

  for (i = 0; keys[i] != NULL; i++)
    key_list = g_list_prepend (key_list, keys[i]);

  key_list = g_list_sort (key_list, (GCompareFunc) on_comparison);

  for (i = 0, l = key_list; l != NULL; l = l->next, i++)
  {
    GVariant            *variant;
    GSettingsSchemaKey  *schema_key = NULL;
    NimfSettingsPageKey *page_key;
    const GVariantType  *type;
    const gchar         *key;
    const gchar         *summary;
    const gchar         *desc;

    key = l->data;

    if (g_str_has_prefix (key, "hidden-"))
      continue;

    variant = g_settings_get_value (page->gsettings, key);
    type = g_variant_get_type (variant);
    schema_key = g_settings_schema_get_key (schema, key);
    summary = g_settings_schema_key_get_summary     (schema_key);
    desc    = g_settings_schema_key_get_description (schema_key);

    page_key = nimf_settings_page_key_new (page->gsettings, key, summary, desc);
    g_ptr_array_add (page->page_keys, page_key);

    if (g_variant_type_equal (type, G_VARIANT_TYPE_BOOLEAN))
    {
      GtkWidget *item;
      item = nimf_settings_page_key_build_boolean (page_key);
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
    g_variant_unref (variant);
  }

  g_strfreev (keys);
  g_list_free (key_list);
  g_settings_schema_unref (schema);

  return page;
}

static GtkWidget *
nimf_settings_build_main_window (NimfSettings *nsettings)
{
  GtkWidget  *window;
  GtkWidget  *notebook;
  GList      *schema_list = NULL;
  gchar     **non_relocatable;
  gint        i;

  window = gtk_application_window_new (nsettings->app);
  gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);
  gtk_window_set_title        (GTK_WINDOW (window), _("Nimf Settings"));
  gtk_window_set_icon_name    (GTK_WINDOW (window), "nimf");

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos    (GTK_NOTEBOOK (notebook), GTK_POS_LEFT);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
  gtk_container_add (GTK_CONTAINER (window), notebook);

  g_settings_schema_source_list_schemas (nsettings->schema_source, TRUE,
                                         &non_relocatable, NULL);

  for (i = 0; non_relocatable[i] != NULL; i++)
    if (g_str_has_prefix (non_relocatable[i], "org.nimf"))
      schema_list = g_list_prepend (schema_list, non_relocatable[i]);

  for (schema_list = g_list_sort (schema_list, (GCompareFunc) on_comparison);
       schema_list != NULL;
       schema_list = schema_list->next)
  {
    NimfSettingsPage  *page;
    GtkWidget         *scrolled_w;

    scrolled_w = gtk_scrolled_window_new (NULL, NULL);
    page = nimf_settings_page_new (nsettings,
                                   (const gchar *) schema_list->data);
    gtk_container_add (GTK_CONTAINER (scrolled_w), page->box);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolled_w, page->label);
    g_ptr_array_add (nsettings->pages, page);
  }

  g_strfreev (non_relocatable);
  g_list_free (schema_list);

  return window;
}

static void
on_activate (GtkApplication *app, NimfSettings *nsettings)
{
  const GList  *window_list;

  window_list = gtk_application_get_windows (app);

  if (window_list)
  {
    gtk_window_present (GTK_WINDOW (window_list->data));
    return;
  }

  nimf_settings_window = nimf_settings_build_main_window (nsettings);
  gtk_application_add_window (GTK_APPLICATION (nsettings->app),
                              GTK_WINDOW (nimf_settings_window));

  gtk_widget_show_all (nimf_settings_window);
}

int
nimf_settings_run (NimfSettings  *nsettings,
                   int            argc,
                   char         **argv)
{
  return g_application_run (G_APPLICATION (nsettings->app), argc, argv);
}

NimfSettings *
nimf_settings_new ()
{
  return g_object_new (NIMF_TYPE_SETTINGS, NULL);
}

static void nimf_settings_page_free (NimfSettingsPage *page)
{
  g_object_unref (page->gsettings);
  g_slice_free (NimfSettingsPage, page);
}

static void
nimf_settings_init (NimfSettings *nsettings)
{
  nsettings->schema_source = g_settings_schema_source_get_default ();
  nsettings->pages = g_ptr_array_new_with_free_func ((GDestroyNotify) nimf_settings_page_free);
  nsettings->app = gtk_application_new ("org.nimf.settings",
                                        G_APPLICATION_FLAGS_NONE);
  g_signal_connect (nsettings->app, "activate",
                    G_CALLBACK (on_activate), nsettings);
}

static void
nimf_settings_finalize (GObject *object)
{
  NimfSettings *nsettings = NIMF_SETTINGS (object);

  g_ptr_array_unref (nsettings->pages);
  g_object_unref (nsettings->app);

  G_OBJECT_CLASS (nimf_settings_parent_class)->finalize (object);
}

static void
nimf_settings_class_init (NimfSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = nimf_settings_finalize;
}

int main (int argc, char **argv)
{
  NimfSettings *nsettings;
  int status;

  nsettings = nimf_settings_new ();
  status = nimf_settings_run (nsettings, argc, argv);

  g_object_unref (nsettings);

  return status;
}