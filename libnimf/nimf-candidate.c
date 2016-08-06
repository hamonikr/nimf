/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-candidate.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015,2016 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-candidate.h"
#include <gtk/gtk.h>

struct _NimfCandidate
{
  GObject parent_instance;

  GtkWidget   *window;
  GtkWidget   *treeview;
  GtkTreeIter  iter;
  NimfContext *target;
};

struct _NimfCandidateClass
{
  GObjectClass parent_class;
};

enum
{
  MAIN_COLUMN,
  EXTRA_COLUMN,
  N_COLUMNS
};

G_DEFINE_TYPE (NimfCandidate, nimf_candidate, G_TYPE_OBJECT);

static gboolean
on_changed (GtkTreeSelection *selection,
            gpointer          iter)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return gtk_tree_selection_get_selected (selection, NULL, iter);
}

static void
on_tree_view_row_activated (GtkTreeView       *tree_view,
                            GtkTreePath       *path,
                            GtkTreeViewColumn *column,
                            NimfCandidate     *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngineClass *engine_class;

  g_return_if_fail (candidate->target &&
                    NIMF_IS_ENGINE (candidate->target->engine));

  engine_class = NIMF_ENGINE_GET_CLASS (candidate->target->engine);

  gchar *text = nimf_candidate_get_selected_text (candidate);
  gint *indices = gtk_tree_path_get_indices (path);

  if (engine_class->candidate_clicked)
    engine_class->candidate_clicked (candidate->target->engine,
                                     candidate->target,
                                     text, indices[0]);
  g_free (text);
}

static void
nimf_candidate_init (NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkCellRenderer   *renderer;
  GtkTreeViewColumn *column1, *column2;
  GtkListStore      *store;
  GtkTreeSelection  *selection;

  gtk_init (NULL, NULL);

  /* gtk tree view */
  store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
  candidate->treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));
  g_signal_connect (selection, "changed", (GCallback) on_changed, &candidate->iter);

  renderer = gtk_cell_renderer_text_new ();
  column1 = gtk_tree_view_column_new_with_attributes ("Main", renderer,
                                                      "text", MAIN_COLUMN,
                                                      NULL);
  column2 = gtk_tree_view_column_new_with_attributes ("Extra", renderer,
                                                      "text", EXTRA_COLUMN,
                                                      NULL);
  gtk_tree_view_column_set_sizing (column1, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_sizing (column2, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (candidate->treeview), column1);
  gtk_tree_view_append_column (GTK_TREE_VIEW (candidate->treeview), column2);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (candidate->treeview), FALSE);

  /* scrolled window */
  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  /* gtk window */
  candidate->window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_type_hint (GTK_WINDOW (candidate->window),
                            GDK_WINDOW_TYPE_HINT_POPUP_MENU);
  gtk_widget_set_size_request (candidate->window, 200, 320);
  gtk_container_set_border_width (GTK_CONTAINER (candidate->window), 1);
  gtk_container_add (GTK_CONTAINER (scrolled_window), candidate->treeview);
  gtk_container_add (GTK_CONTAINER (candidate->window), scrolled_window);

  g_signal_connect (candidate->treeview, "row-activated",
                    (GCallback) on_tree_view_row_activated, candidate);
}

static void
nimf_candidate_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gtk_widget_destroy (NIMF_CANDIDATE (object)->window);
  G_OBJECT_CLASS (nimf_candidate_parent_class)->finalize (object);
}

static void
nimf_candidate_class_init (NimfCandidateClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = nimf_candidate_finalize;
}

/* items1 and items2 should be same length */
void
nimf_candidate_update_window (NimfCandidate  *candidate,
                              const gchar   **items1,
                              const gchar   **items2)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeModel  *model;
  GtkTreeIter    iter;
  GtkAdjustment *adjustment;
  guint          i;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  gtk_list_store_clear (GTK_LIST_STORE (model));

  for (i = 0; items1 && items1[i]; i++)
  {
    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set    (GTK_LIST_STORE (model), &iter,
                           MAIN_COLUMN, items1[i], -1);

    if (items2)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          EXTRA_COLUMN, items2[i], -1);
  }

  adjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (candidate->treeview));
  gtk_adjustment_set_value (adjustment, 0.0);
}

static void
nimf_candidate_select_first_if_available (NimfCandidate    *candidate,
                                          GtkTreeModel     *model,
                                          GtkTreeSelection *selection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (gtk_tree_model_get_iter_first (model, &candidate->iter))
  {
    GtkTreePath *path = NULL;
    gtk_tree_selection_select_iter (selection, &candidate->iter);
    path = gtk_tree_model_get_path (model, &candidate->iter);
    gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (candidate->treeview),
                                  path, NULL, TRUE, 0.0, 0.0);
    gtk_tree_path_free (path);
  }
}

void nimf_candidate_show_window (NimfCandidate *candidate,
                                 NimfContext   *target,
                                 gboolean       select_first)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeModel   *model;
  GtkRequisition  natural_size;
  int             x, y, w, h;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));

  if (select_first)
  {
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));
    nimf_candidate_select_first_if_available (candidate, model, selection);
  }

  candidate->target = target;
  gtk_widget_show_all (candidate->window);
  gtk_widget_get_preferred_size (candidate->window, NULL, &natural_size);
  gtk_window_resize (GTK_WINDOW (candidate->window),
                     natural_size.width, natural_size.height);

  gtk_window_get_size (GTK_WINDOW (candidate->window), &w, &h);

  x = target->cursor_area.x - target->cursor_area.width;
  y = target->cursor_area.y + target->cursor_area.height;

  if (x + w > gdk_screen_width ())
    x = gdk_screen_width () - w;

  if (y + h > gdk_screen_height ())
    y = target->cursor_area.y - h;

  gtk_window_move (GTK_WINDOW (candidate->window), x, y);
}

void nimf_candidate_hide_window (NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gtk_widget_hide (candidate->window);
}

gboolean nimf_candidate_is_window_visible (NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return gtk_widget_is_visible (candidate->window);
}

static gboolean
nimf_candidate_scroll_to_cell (NimfCandidate    *candidate,
                               GtkTreeModel     *model,
                               GtkTreeSelection *selection,
                               gfloat            align)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreePath  *path = NULL;
  GdkWindow    *bin_window;
  GdkRectangle  rect;
  gboolean      retval = TRUE;

  gtk_tree_selection_select_iter (selection, &candidate->iter);
  path = gtk_tree_model_get_path (model, &candidate->iter);
  gtk_tree_view_get_cell_area (GTK_TREE_VIEW (candidate->treeview),
                               path, NULL, &rect);
  bin_window = gtk_tree_view_get_bin_window
                                      (GTK_TREE_VIEW (candidate->treeview));
  if (rect.y + rect.height >= gdk_window_get_height (bin_window) ||
      rect.y < 0)
  {
    gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (candidate->treeview),
                                  path, NULL, TRUE, align, 0.0);
    retval = FALSE;
  }

  gtk_tree_path_free (path);

  return retval;
}

static gboolean
nimf_candidate_select_previous_item_return_val (NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter       iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_selection_get_selected (selection, &model, &iter) == FALSE)
  {
    nimf_candidate_select_first_if_available (candidate, model, selection);
    return FALSE;
  }

  if (gtk_tree_model_iter_previous (model, &candidate->iter))
    return nimf_candidate_scroll_to_cell (candidate, model, selection, 1.0);
  else
    gtk_tree_model_get_iter_first (model, &candidate->iter);

  return FALSE;
}

void
nimf_candidate_select_previous_item (NimfCandidate *candidate)
{
  nimf_candidate_select_previous_item_return_val (candidate);
}

static gboolean
nimf_candidate_select_next_item_return_val (NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter       iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_selection_get_selected (selection, &model, &iter) == FALSE)
  {
    nimf_candidate_select_first_if_available (candidate, model, selection);
    return FALSE;
  }

  if (gtk_tree_model_iter_next (model, &candidate->iter))
  {
    return nimf_candidate_scroll_to_cell (candidate, model, selection, 0.0);
  }
  else
  {
    gint n_row = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (model), NULL);
    gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (model), &candidate->iter,
                                   NULL, n_row - 1);
  }

  return FALSE;
}

void
nimf_candidate_select_next_item (NimfCandidate *candidate)
{
  nimf_candidate_select_next_item_return_val (candidate);
}

void nimf_candidate_select_page_up_item (NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  while (nimf_candidate_select_previous_item_return_val (candidate))
  { }
}

void nimf_candidate_select_page_down_item (NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  while (nimf_candidate_select_next_item_return_val (candidate))
  { }
}

NimfCandidate *nimf_candidate_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (NIMF_TYPE_CANDIDATE, NULL);
}

gchar *nimf_candidate_get_selected_text (NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeIter   iter;
  GtkTreeModel *model;
  gchar        *text = NULL;

  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    gtk_tree_model_get (model, &iter, MAIN_COLUMN, &text, -1);

  return text;
}

gint nimf_candidate_get_selected_index (NimfCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

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
