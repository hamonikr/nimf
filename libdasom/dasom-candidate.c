/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-candidate.c
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

#include "dasom-candidate.h"

enum
{
  TITEL_COLUMN,
  N_COLUMNS
};

G_DEFINE_TYPE (DasomCandidate, dasom_candidate, G_TYPE_OBJECT);

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
                            DasomCandidate    *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomEngineClass *engine_class;
  engine_class = DASOM_ENGINE_GET_CLASS (candidate->target->engine);

  gchar *text = dasom_candidate_get_selected_text (candidate);

  if (engine_class->candidate_clicked)
    engine_class->candidate_clicked (candidate->target->engine,
                                     candidate->target, text);
  g_free (text);
}

static void
on_tree_view_realize (GtkWidget      *tree_view,
                      DasomCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeViewColumn *column;
  GtkAdjustment *adjustment;
  gint horizontal_space, height;
  guint border_width;

  column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), TITEL_COLUMN);
  gtk_tree_view_column_cell_get_size (column, NULL, NULL, NULL, NULL, &height);
  border_width = gtk_container_get_border_width (GTK_CONTAINER (candidate->window));
  gtk_widget_style_get (tree_view, "horizontal-separator",
                        &horizontal_space, NULL);
  height = height + horizontal_space / 2;
  gtk_window_resize (GTK_WINDOW (candidate->window),
                     height * 10 / 1.6,
                     height * 10 + border_width * 2);

  adjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (candidate->treeview));
  gtk_adjustment_set_value (adjustment, 0.0);
}

static void
dasom_candidate_init (DasomCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkCellRenderer   *renderer;
  GtkTreeViewColumn *column;
  GtkListStore      *store;
  GtkTreeSelection  *selection;

  gtk_init (0, NULL);

  /* gtk tree view */
  store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING);
  candidate->treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));
  g_signal_connect (candidate->treeview, "realize",
                    (GCallback) on_tree_view_realize, candidate);
  g_signal_connect (selection, "changed", (GCallback) on_changed, &candidate->iter);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Title", renderer,
                                                     "text", TITEL_COLUMN,
                                                     NULL);
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_append_column (GTK_TREE_VIEW (candidate->treeview), column);
  gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (candidate->treeview), TRUE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (candidate->treeview), FALSE);

  /* scrolled window */
  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);

  /* gtk window */
  candidate->window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_position (GTK_WINDOW (candidate->window), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width (GTK_CONTAINER (candidate->window), 1);
  gtk_container_add (GTK_CONTAINER (scrolled_window), candidate->treeview);
  gtk_container_add (GTK_CONTAINER (candidate->window), scrolled_window);

  g_signal_connect (candidate->treeview, "row-activated",
                    (GCallback) on_tree_view_row_activated, candidate);
}

static void
dasom_candidate_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gtk_widget_destroy (DASOM_CANDIDATE (object)->window);
  G_OBJECT_CLASS (dasom_candidate_parent_class)->finalize (object);
}

static void
dasom_candidate_class_init (DasomCandidateClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass* object_class = G_OBJECT_CLASS (class);

  object_class->finalize = dasom_candidate_finalize;
}

void
dasom_candidate_update_window (DasomCandidate  *candidate,
                               const gchar    **strv)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeModel  *model;
  GtkTreeIter    iter;
  GtkAdjustment *adjustment;
  guint          i;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  gtk_list_store_clear (GTK_LIST_STORE (model));

  for (i = 0; strv[i] != NULL; i++)
  {
    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set    (GTK_LIST_STORE (model), &iter,
                           TITEL_COLUMN, strv[i], -1);
  }

  adjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (candidate->treeview));
  gtk_adjustment_set_value (adjustment, 0.0);
}

void dasom_candidate_show_window (DasomCandidate  *candidate,
                                  DasomConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeModel *model;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_model_get_iter_first (model, &candidate->iter))
  {
    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));
    gtk_tree_selection_select_iter (selection, &candidate->iter);
  }

  candidate->target = target;

  gtk_window_move (GTK_WINDOW (candidate->window),
                   target->cursor_area.x, target->cursor_area.y);
  gtk_widget_show_all (candidate->window);
}

void dasom_candidate_hide_window (DasomCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gtk_widget_hide (candidate->window);
}

void dasom_candidate_select_previous_item (DasomCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeModel *model;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_model_iter_previous (model, &candidate->iter))
  {
    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));
    gtk_tree_selection_select_iter (selection, &candidate->iter);

    GtkAdjustment *adjustment;
    adjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (candidate->treeview));
    gdouble page_increment = gtk_adjustment_get_page_increment (adjustment);
    gtk_adjustment_set_step_increment (adjustment, page_increment);

    GtkTreePath *path = NULL;
    GdkRectangle rect = {0};

    path = gtk_tree_model_get_path (model, &candidate->iter);
    gtk_tree_view_get_background_area (GTK_TREE_VIEW (candidate->treeview),
                                       path,
                                       NULL,
                                       &rect);

    gint *index = gtk_tree_path_get_indices (path); /* DO NOT FREE *index */
    gtk_adjustment_set_value (adjustment, rect.height * 10 * (index[0] / 10));

    gtk_tree_path_free (path);
  }
  else
    gtk_tree_model_get_iter_first (model, &candidate->iter);
}

void dasom_candidate_select_next_item (DasomCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeModel *model;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_model_iter_next (model, &candidate->iter))
  {
    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));
    gtk_tree_selection_select_iter (selection, &candidate->iter);

    GtkAdjustment *adjustment;
    adjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (candidate->treeview));
    gdouble page_increment = gtk_adjustment_get_page_increment (adjustment);
    gtk_adjustment_set_step_increment (adjustment, page_increment);

    GtkTreePath *path = NULL;
    GdkRectangle rect = {0};

    gtk_tree_selection_select_iter (selection, &candidate->iter);
    path = gtk_tree_model_get_path (model, &candidate->iter);
    gtk_tree_view_get_background_area (GTK_TREE_VIEW (candidate->treeview),
                                       path,
                                       NULL,
                                       &rect);

    gint *index = gtk_tree_path_get_indices (path); /* DO NOT FREE *index */
    gtk_adjustment_set_value (adjustment, rect.height * 10 * (index[0] / 10));

    gtk_tree_path_free (path);
  }
  else
  {
    gint n_row = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (model), NULL);
    gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (model),
                                   &candidate->iter,
                                   NULL,
                                   n_row - 1);
  }
}

DasomCandidate *dasom_candidate_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (DASOM_TYPE_CANDIDATE, NULL);
}

gchar *dasom_candidate_get_selected_text (DasomCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeIter   iter;
  GtkTreeModel *model;
  gchar        *text = NULL;

  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    gtk_tree_model_get (model, &iter, TITEL_COLUMN, &text, -1);

  g_print ("TEXT: %s\n", text);
  return text;
}
