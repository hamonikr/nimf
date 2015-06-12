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
#include "dasom-key-syms.h"

enum
{
  TITEL_COLUMN,
  N_COLUMNS
};

G_DEFINE_TYPE (DasomCandidate, dasom_candidate, G_TYPE_OBJECT);

static void
row_activated (GtkTreeView       *tree_view,
               GtkTreePath       *path,
               GtkTreeViewColumn *column,
               gpointer           user_data)
{
  /* TODO */
  g_error ("ROW ACTIVATED");
}

static void
on_changed (GtkTreeSelection *selection,
            gpointer          iter)
{
  gtk_tree_selection_get_selected (selection, NULL, iter);
}

static void
dasom_candidate_init (DasomCandidate *candidate)
{
  GtkCellRenderer   *renderer;
  GtkTreeViewColumn *column;
  GtkListStore      *store;

  gtk_init (0, NULL); /* FIXME */

  /* gtk tree view */
  store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING);
  candidate->treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  candidate->model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  candidate->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Candidate View",
                                                     renderer,
                                                     "text", TITEL_COLUMN,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (candidate->treeview), column);

  /* gtk window */
  candidate->window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_position (GTK_WINDOW (candidate->window), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width (GTK_CONTAINER (candidate->window), 1);
  gtk_widget_set_size_request (candidate->window, 200, 320);

  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (candidate->window), scrolled_window);
  gtk_container_add (GTK_CONTAINER (scrolled_window), candidate->treeview);

  g_signal_connect (candidate->treeview, "row-activated", (GCallback) row_activated, NULL);
  g_signal_connect (candidate->selection, "changed", (GCallback) on_changed, &candidate->iter);
}

static void
dasom_candidate_finalize (GObject *object)
{
  /* TODO: Add deinitalization code here */

  G_OBJECT_CLASS (dasom_candidate_parent_class)->finalize (object);
}

static void
dasom_candidate_class_init (DasomCandidateClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dasom_candidate_finalize;
}

void
dasom_candidate_update_window (DasomCandidate  *candidate,
                               const gchar    **strv)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gtk_list_store_clear (GTK_LIST_STORE (candidate->model));
  guint i;

  for (i = 0; strv[i] != NULL; i++)
  {
    candidate->model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));

    gtk_list_store_append (GTK_LIST_STORE (candidate->model), &candidate->iter);
    gtk_list_store_set    (GTK_LIST_STORE (candidate->model), &candidate->iter,
                           TITEL_COLUMN, strv[i],
                           -1);
  }

  gtk_tree_model_get_iter_first  (candidate->model,     &candidate->iter);
  gtk_tree_selection_select_iter (candidate->selection, &candidate->iter);

  gtk_widget_show_all (candidate->window);
}

void dasom_candidate_show_window (DasomCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gtk_widget_show_all (candidate->window);
}

void dasom_candidate_hide_window (DasomCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gtk_widget_hide (candidate->window);
}

void dasom_candidate_select_previous_item (DasomCandidate *candidate)
{
  if (gtk_tree_model_iter_previous (candidate->model, &candidate->iter))
    gtk_tree_selection_select_iter (candidate->selection, &candidate->iter);
  else
    gtk_tree_model_get_iter_first (candidate->model, &candidate->iter);
}

void dasom_candidate_select_next_item (DasomCandidate *candidate)
{
  if (gtk_tree_model_iter_next (candidate->model, &candidate->iter))
    gtk_tree_selection_select_iter (candidate->selection, &candidate->iter);
  else
  {
    gint n_row = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (candidate->model), NULL);
    gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (candidate->model),
                                   &candidate->iter,
                                   NULL,
                                   n_row - 1);
  }
}

DasomCandidate *dasom_candidate_new ()
{
  return g_object_new (DASOM_TYPE_CANDIDATE, NULL);
}

gchar *dasom_candidate_get_selected_text (DasomCandidate *candidate)
{
  GtkTreeIter   iter;
  GtkTreeModel *model;
  gchar        *text = NULL;

  if (gtk_tree_selection_get_selected (candidate->selection, &model, &iter))
    gtk_tree_model_get (model, &iter, TITEL_COLUMN, &text,  -1);

  return text;
}
