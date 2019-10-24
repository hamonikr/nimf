/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-candidate.c
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

#define NIMF_TYPE_CANDIDATE             (nimf_candidate_get_type ())
#define NIMF_CANDIDATE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_CANDIDATE, NimfCandidate))
#define NIMF_CANDIDATE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_CANDIDATE, NimfCandidateClass))
#define NIMF_IS_CANDIDATE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_CANDIDATE))
#define NIMF_IS_CANDIDATE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_CANDIDATE))
#define NIMF_CANDIDATE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_CANDIDATE, NimfCandidateClass))

typedef struct _NimfCandidate      NimfCandidate;
typedef struct _NimfCandidateClass NimfCandidateClass;

struct _NimfCandidateClass
{
  NimfServiceClass parent_class;
};

struct _NimfCandidate
{
  NimfService parent_instance;

  gchar    *id;
  gboolean  active;

  NimfServiceIC *target;
  GtkWidget     *window;
  GtkWidget     *entry;
  GtkWidget     *treeview;
  GtkWidget     *scrollbar;
  gint           cell_height;
};

GType nimf_candidate_get_type (void) G_GNUC_CONST;

enum
{
  INDEX_COLUMN,
  MAIN_COLUMN,
  EXTRA_COLUMN,
  N_COLUMNS
};

static void
nimf_candidate_show (NimfCandidatable *candidatable,
                     NimfServiceIC    *target,
                     gboolean          show_entry)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkRequisition       natural_size;
  int                  x, y, w, h;
  GdkRectangle         geometry;
  const NimfRectangle *cursor_area;

  cursor_area = nimf_service_ic_get_cursor_location (target);

#if GTK_CHECK_VERSION (3, 22, 0)
  GdkDisplay *display = gtk_widget_get_display (candidate->window);
  GdkMonitor *monitor;
  monitor = gdk_display_get_monitor_at_point (display,
                                              cursor_area->x, cursor_area->y);
  gdk_monitor_get_geometry (monitor, &geometry);
#else
  GdkScreen *screen = gtk_widget_get_screen (candidate->window);
  gint  monitor_num = gdk_screen_get_monitor_at_point (screen,
                                                       cursor_area->x,
                                                       cursor_area->y);
  gdk_screen_get_monitor_geometry (screen, monitor_num, &geometry);
#endif

  candidate->target = target;

  if (show_entry)
    gtk_widget_show (candidate->entry);
  else
    gtk_widget_hide (candidate->entry);

  gtk_widget_show_all (candidate->window);
  gtk_widget_get_preferred_size (candidate->window, NULL, &natural_size);
  gtk_window_resize (GTK_WINDOW (candidate->window),
                     natural_size.width, natural_size.height);
  gtk_window_get_size (GTK_WINDOW (candidate->window), &w, &h);

  x = cursor_area->x - cursor_area->width;
  y = cursor_area->y + cursor_area->height;

  if (x + w > geometry.x + geometry.width)
    x = geometry.x + geometry.width - w;

  if ((y + h > geometry.y + geometry.height) &&
      ((cursor_area->y - h) >= geometry.y))
    y = cursor_area->y - h;

  gtk_window_move (GTK_WINDOW (candidate->window), x, y);
}

static void
nimf_candidate_hide (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gtk_widget_hide (NIMF_CANDIDATE (candidatable)->window);
}

static gboolean
nimf_candidate_is_visible (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return gtk_widget_is_visible (NIMF_CANDIDATE (candidatable)->window);
}

static void
nimf_candidate_set_page_values (NimfCandidatable *candidatable,
                                NimfServiceIC    *target,
                                gint              page_index,
                                gint              n_pages,
                                gint              page_size)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkRange *range = GTK_RANGE (candidate->scrollbar);

  candidate->target = target;
  gtk_range_set_range (range, 1.0, (gdouble) n_pages + 1.0);

  if (page_index != (gint) gtk_range_get_value (range))
    gtk_range_set_value (range, (gdouble) page_index);

  gtk_widget_set_size_request (candidate->treeview,
                               (gint) (candidate->cell_height *  10 / 1.6),
                               candidate->cell_height * page_size);
}

static void
nimf_candidate_clear (NimfCandidatable *candidatable,
                      NimfServiceIC    *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkTreeModel *model;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  gtk_list_store_clear (GTK_LIST_STORE (model));
  nimf_candidate_set_page_values (candidatable, target, 1, 1, 5);
}

static void
nimf_candidate_append (NimfCandidatable *candidatable,
                       const gchar      *text1,
                       const gchar      *text2)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkTreeModel  *model;
  GtkTreeIter    iter;
  gint           n_row;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  n_row = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (model), NULL);

  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set    (GTK_LIST_STORE (model), &iter,
                         INDEX_COLUMN, (n_row + 1) % 10,
                         MAIN_COLUMN, text1, -1);

  if (text2)
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        EXTRA_COLUMN, text2, -1);
}

static gint
nimf_candidate_get_selected_index (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkTreeIter       iter;
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  gint              index = -1;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
    gint *indices = gtk_tree_path_get_indices (path);

    if (indices)
      index = indices[0];

    gtk_tree_path_free (path);
  }

  return index;
}

static gchar *
nimf_candidate_get_selected_text (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkTreeIter   iter;
  GtkTreeModel *model;
  gchar        *text = NULL;

  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    gtk_tree_model_get (model, &iter, MAIN_COLUMN, &text, -1);

  return text;
}

static void
nimf_candidate_select_first_item_in_page (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter       iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_model_get_iter_first (model, &iter))
    gtk_tree_selection_select_iter (selection, &iter);
}

static void
nimf_candidate_select_last_item_in_page (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter       iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));
  gint n_row = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (model), NULL);

  if (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (model),
                                     &iter, NULL, MAX (0, n_row - 1)))
    gtk_tree_selection_select_iter (selection, &iter);
}

static void
nimf_candidate_select_item_by_index_in_page (NimfCandidatable *candidatable,
                                             gint              index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter       iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (model), &iter, NULL, index))
    gtk_tree_selection_select_iter (selection, &iter);
}

static void
nimf_candidate_select_previous_item (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter       iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_selection_get_selected (selection, &model, &iter) == FALSE)
  {
    nimf_candidate_select_last_item_in_page (candidatable);
    return;
  }

  if (gtk_tree_model_iter_previous (model, &iter))
  {
    gtk_tree_selection_select_iter (selection, &iter);
  }
  else
  {
    NimfEngineClass *engine_class;
    NimfEngine      *engine;
    engine = nimf_service_ic_get_engine (candidate->target);
    engine_class = NIMF_ENGINE_GET_CLASS (engine);

    if (engine_class->candidate_page_up)
    {
      if (engine_class->candidate_page_up (engine, candidate->target))
        nimf_candidate_select_last_item_in_page (candidatable);
    }
  }
}

static void
nimf_candidate_select_next_item (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter       iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_selection_get_selected (selection, &model, &iter) == FALSE)
  {
    nimf_candidate_select_first_item_in_page (candidatable);
    return;
  }

  if (gtk_tree_model_iter_next (model, &iter))
  {
    gtk_tree_selection_select_iter (selection, &iter);
  }
  else
  {
    NimfEngineClass *engine_class;
    NimfEngine      *engine;
    engine = nimf_service_ic_get_engine (candidate->target);
    engine_class = NIMF_ENGINE_GET_CLASS (engine);

    if (engine_class->candidate_page_down)
    {
      if (engine_class->candidate_page_down (engine, candidate->target))
        nimf_candidate_select_first_item_in_page (candidatable);
    }
  }
}

static void
nimf_candidate_set_auxiliary_text (NimfCandidatable *candidatable,
                                   const gchar      *text,
                                   gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (candidatable);

  gtk_entry_set_text (GTK_ENTRY (candidate->entry), text);
  gtk_editable_set_position (GTK_EDITABLE (candidate->entry), cursor_pos);
}

static void
nimf_candidate_iface_init (NimfCandidatableInterface *iface)
{
  iface->show                         = nimf_candidate_show;
  iface->hide                         = nimf_candidate_hide;
  iface->is_visible                   = nimf_candidate_is_visible;
  iface->clear                        = nimf_candidate_clear;
  iface->set_page_values              = nimf_candidate_set_page_values;
  iface->append                       = nimf_candidate_append;
  iface->get_selected_index           = nimf_candidate_get_selected_index;
  iface->get_selected_text            = nimf_candidate_get_selected_text;
  iface->select_first_item_in_page    = nimf_candidate_select_first_item_in_page;
  iface->select_last_item_in_page     = nimf_candidate_select_last_item_in_page;
  iface->select_item_by_index_in_page = nimf_candidate_select_item_by_index_in_page;
  iface->select_previous_item         = nimf_candidate_select_previous_item;
  iface->select_next_item             = nimf_candidate_select_next_item;
  iface->set_auxiliary_text           = nimf_candidate_set_auxiliary_text;
}

G_DEFINE_DYNAMIC_TYPE_EXTENDED (NimfCandidate,
                                nimf_candidate,
                                NIMF_TYPE_SERVICE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (NIMF_TYPE_CANDIDATABLE,
                                                               nimf_candidate_iface_init));

static const gchar *
nimf_candidate_get_id (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_SERVICE (service), NULL);

  return NIMF_CANDIDATE (service)->id;
}

static gboolean
nimf_candidate_is_active (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_CANDIDATE (service)->active;
}

static void
on_tree_view_row_activated (GtkTreeView       *tree_view,
                            GtkTreePath       *path,
                            GtkTreeViewColumn *column,
                            NimfCandidate     *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngineClass *engine_class;
  NimfEngine      *engine;

  engine = nimf_service_ic_get_engine (candidate->target);

  g_return_if_fail (candidate->target && NIMF_IS_ENGINE (engine));

  engine_class = NIMF_ENGINE_GET_CLASS (engine);

  gchar *text;
  gint  *indices = gtk_tree_path_get_indices (path);

  text = nimf_candidate_get_selected_text (NIMF_CANDIDATABLE (candidate));

  if (engine_class->candidate_clicked)
    engine_class->candidate_clicked (engine,
                                     candidate->target, text, indices[0]);
  g_free (text);
}

static gboolean
on_range_change_value (GtkRange      *range,
                       GtkScrollType  scroll,
                       gdouble        value,
                       NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngine *engine = nimf_service_ic_get_engine (candidate->target);

  g_return_val_if_fail (candidate->target && NIMF_IS_ENGINE (engine), FALSE);

  NimfEngineClass *engine_class;
  GtkAdjustment   *adjustment;
  gdouble          lower, upper;

  adjustment = gtk_range_get_adjustment (range);
  lower = gtk_adjustment_get_lower (adjustment);
  upper = gtk_adjustment_get_upper (adjustment);

  if (value < lower)
    value = lower;
  if (value > upper - 1)
    value = upper - 1;

  engine_class = NIMF_ENGINE_GET_CLASS (engine);

  if (engine_class->candidate_scrolled)
    engine_class->candidate_scrolled (engine, candidate->target, value);

  return FALSE;
}

static gboolean
on_entry_draw (GtkWidget *widget,
               cairo_t   *cr,
               gpointer   user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkStyleContext *style_context;
  PangoContext    *pango_context;
  PangoLayout     *layout;
  const char      *text;
  gint             cursor_index;
  gint             x, y;

  style_context = gtk_widget_get_style_context (widget);
  pango_context = gtk_widget_get_pango_context (widget);
  layout = gtk_entry_get_layout (GTK_ENTRY (widget));
  text = pango_layout_get_text (layout);
  gtk_entry_get_layout_offsets (GTK_ENTRY (widget), &x, &y);
  cursor_index = g_utf8_offset_to_pointer (text, gtk_editable_get_position (GTK_EDITABLE (widget))) - text;
  gtk_render_insertion_cursor (style_context, cr, x, y, layout, cursor_index,
                               pango_context_get_base_dir (pango_context));
  return FALSE;
}

static gboolean
nimf_candidate_start (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (service);

  if (candidate->active)
    return TRUE;

  GtkCellRenderer   *renderer;
  GtkTreeViewColumn *column[N_COLUMNS];
  GtkListStore      *store;
  gint               fixed_height = 32;
  gint               horizontal_space;

  gtk_init (NULL, NULL);

  /* gtk entry */
  candidate->entry = gtk_entry_new ();
  gtk_editable_set_editable (GTK_EDITABLE (candidate->entry), FALSE);
  gtk_widget_set_no_show_all (candidate->entry, TRUE);
  g_signal_connect_after (candidate->entry, "draw",
                          G_CALLBACK (on_entry_draw), NULL);
  /* gtk tree view */
  store = gtk_list_store_new (N_COLUMNS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
  candidate->treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (candidate->treeview), FALSE);
  gtk_tree_view_set_activate_on_single_click (GTK_TREE_VIEW (candidate->treeview), TRUE);
  gtk_widget_style_get (candidate->treeview, "horizontal-separator",
                        &horizontal_space, NULL);
  candidate->cell_height = fixed_height + horizontal_space / 2;
  gtk_widget_set_size_request (candidate->treeview,
                               (gint) (candidate->cell_height * 10 / 1.6),
                               candidate->cell_height * 10);
  g_signal_connect (candidate->treeview, "row-activated",
                    (GCallback) on_tree_view_row_activated, candidate);
  /* column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer, "height", fixed_height, "font", "Sans 14", NULL);

  column[INDEX_COLUMN] = gtk_tree_view_column_new_with_attributes ("Index",
                                        renderer, "text", INDEX_COLUMN, NULL);
  column[MAIN_COLUMN]  = gtk_tree_view_column_new_with_attributes ("Main",
                                        renderer, "text", MAIN_COLUMN, NULL);
  column[EXTRA_COLUMN] = gtk_tree_view_column_new_with_attributes ("Extra",
                                        renderer, "text", EXTRA_COLUMN, NULL);
  gtk_tree_view_column_set_sizing (column[INDEX_COLUMN], GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_sizing (column[MAIN_COLUMN],  GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_sizing (column[EXTRA_COLUMN], GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (candidate->treeview),
                               column[INDEX_COLUMN]);
  gtk_tree_view_append_column (GTK_TREE_VIEW (candidate->treeview),
                               column[MAIN_COLUMN]);
  gtk_tree_view_append_column (GTK_TREE_VIEW (candidate->treeview),
                               column[EXTRA_COLUMN]);
  /* scrollbar */
  GtkAdjustment *adjustment = gtk_adjustment_new (1.0, 1.0, 2.0, 1.0, 1.0, 1.0);
  candidate->scrollbar = gtk_scrollbar_new (GTK_ORIENTATION_VERTICAL, adjustment);
  gtk_range_set_slider_size_fixed (GTK_RANGE (candidate->scrollbar), FALSE);
  g_signal_connect (candidate->scrollbar, "change-value",
                    G_CALLBACK (on_range_change_value), candidate);
  GtkCssProvider  *provider;
  GtkStyleContext *style_context;
  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),
                       ".scrollbar {"
                       "  -GtkScrollbar-has-backward-stepper: true;"
                       "  -GtkScrollbar-has-forward-stepper:  true;"
                       "  -GtkScrollbar-has-secondary-forward-stepper:  true;"
                       "}" , -1, NULL);
  style_context = gtk_widget_get_style_context (candidate->scrollbar);
  gtk_style_context_add_provider (style_context,
                                  GTK_STYLE_PROVIDER (provider),
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider);

  /* gtk box */
  GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  gtk_box_pack_start (GTK_BOX (vbox), candidate->entry, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (hbox), candidate->treeview,  TRUE,  TRUE, 0);
  gtk_box_pack_end   (GTK_BOX (hbox), candidate->scrollbar, FALSE, TRUE, 0);

  /* gtk window */
  candidate->window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_type_hint (GTK_WINDOW (candidate->window),
                            GDK_WINDOW_TYPE_HINT_POPUP_MENU);
  gtk_container_set_border_width (GTK_CONTAINER (candidate->window), 1);
  gtk_container_add (GTK_CONTAINER (candidate->window), vbox);
  gtk_widget_realize (candidate->window);

  candidate->active = TRUE;

  return TRUE;
}

static void
nimf_candidate_stop (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (service);

  if (!candidate->active)
    return;

  candidate->active = FALSE;
}

static void
nimf_candidate_init (NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  candidate->id = g_strdup ("nimf-candidate");
}

static void
nimf_candidate_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidate *candidate = NIMF_CANDIDATE (object);

  if (candidate->active)
    nimf_candidate_stop (NIMF_SERVICE (candidate));

  gtk_widget_destroy (candidate->window);
  g_free (candidate->id);

  G_OBJECT_CLASS (nimf_candidate_parent_class)->finalize (object);
}

static void
nimf_candidate_class_init (NimfCandidateClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass     *object_class  = G_OBJECT_CLASS (class);
  NimfServiceClass *service_class = NIMF_SERVICE_CLASS (class);

  service_class->get_id    = nimf_candidate_get_id;
  service_class->start     = nimf_candidate_start;
  service_class->stop      = nimf_candidate_stop;
  service_class->is_active = nimf_candidate_is_active;

  object_class->finalize = nimf_candidate_finalize;
}

static void
nimf_candidate_class_finalize (NimfCandidateClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_candidate_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_candidate_get_type ();
}
