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
#include "dasom-marshalers.h"

enum
{
  TITEL_COLUMN,
  N_COLUMNS
};

enum {
  ROW_ACTIVATED,
  LAST_SIGNAL
};

static guint candidate_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (DasomCandidate, dasom_candidate, G_TYPE_OBJECT);

static gboolean
on_changed (GtkTreeSelection *selection,
            gpointer          iter)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return gtk_tree_selection_get_selected (selection, NULL, iter);
}

static void
on_row_activated (GtkTreeView       *tree_view,
                  GtkTreePath       *path,
                  GtkTreeViewColumn *column,
                  gpointer           user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomCandidate *candidate = user_data;

  /* FIXME: 다솜 프로젝트에서 사용되는
   * 모든 g_signal_emit_by_name 경우에
   * 인자(예; text)를 여기서 free 하는 것이 나을지,
   * 아니면 callback 함수에서 free 하는 것이 나을지 결정해야 합니다 */
  gchar *text = dasom_candidate_get_selected_text (candidate);
  g_signal_emit_by_name (candidate, "row-activated", text);
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

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Title",
                                                     renderer,
                                                     "text", TITEL_COLUMN,
                                                     NULL);
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_append_column (GTK_TREE_VIEW (candidate->treeview), column);
  gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (candidate->treeview), TRUE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (candidate->treeview), FALSE);

  /* scrolled window */
  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);

  /* for default size */
  gint height;
  gtk_tree_view_column_cell_get_size (column, NULL, NULL, NULL, NULL, &height);

  /* gtk window */
  candidate->window = gtk_window_new (GTK_WINDOW_POPUP);
  guint border_width = 1;
  gtk_window_set_position (GTK_WINDOW (candidate->window), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width (GTK_CONTAINER (candidate->window), border_width);
  gtk_window_set_default_size (GTK_WINDOW (candidate->window),
                               (gint) ((gdouble) height * 10.0 / 1.6),
                               height * 10 + border_width * 2);

  gtk_container_add (GTK_CONTAINER (scrolled_window), candidate->treeview);
  gtk_container_add (GTK_CONTAINER (candidate->window), scrolled_window);

  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));

  g_signal_connect (selection, "changed", (GCallback) on_changed, &candidate->iter);
  g_signal_connect (candidate->treeview, "row-activated", (GCallback) on_row_activated, candidate);

/* FIXME: 아래는 cell 높이와 스크롤러 설정을 위한 코드입니다.
   원래 의도한 바는 gtk_widget_show_all 을 실행하지 않고서 실제로 보이게 될
   정확한 크기를 구하는 것입니다. 그런데,
   gtk_widget_show_all 실행 전과 실행 후의 cell 높이 값이 다르고,
   또한, dasom_candidate_show_window 함수에서 gtk_widget_show_all 한 후에
   실행하여도 값이 제대로 나오지를 않습니다.
   그 이유는 저도 모르겠습니다.
   그래서 실제로 보이게 될 정확한 크기를 구하고자 코드를 아래처럼 작성했습니다.
 */
  GtkTreeModel *model;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  GtkTreeIter iter;

  /* 임시로 넣는 데이터입니다. */
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set    (GTK_LIST_STORE (model), &iter,
                         TITEL_COLUMN, g_strdup ("test"),
                         -1);

  /* show 후에 바로 hide 하여도 값이 제대로 나오더군요 */
  gtk_widget_show_all (candidate->window);
  gtk_widget_hide (candidate->window);

  if (gtk_tree_model_get_iter_first (model, &candidate->iter))
  {
    GtkTreePath *path = NULL;
    GdkRectangle rect = {0};

    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));
    gtk_tree_selection_select_iter (selection, &candidate->iter);

    path = gtk_tree_model_get_path (model, &candidate->iter);
    gtk_tree_view_get_background_area (GTK_TREE_VIEW (candidate->treeview),
                                       path,
                                       NULL,
                                       &rect);

    gtk_tree_path_free (path);
    guint border_width = gtk_container_get_border_width (GTK_CONTAINER (candidate->window));
    gtk_window_resize (GTK_WINDOW (candidate->window),
                       (gint) ((gdouble) (rect.height) * 10.0 / 1.6),
                       rect.height * 10 + border_width * 2);

    GtkAdjustment *adjustment;
    adjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (candidate->treeview));
    gtk_adjustment_set_value (adjustment, 0.0);
  }

  gtk_list_store_clear (GTK_LIST_STORE (model));

/* FIXME: 크기 구하는 방법은 아래와 같은 방법으로도 구할 수 있습니다.
   아래 방법 역시 gtk_widget_show_all 전/후 값이 다릅니다.
   또한, dasom_candidate_show_window 함수에서 gtk_widget_show_all 한 후에
   실행하여도 값이 제대로 나오지를 않습니다.
   그 이유는 저도 모르겠습니다.

  gint cell_height, horizontal_separator, height;
  GtkTreeViewColumn *column = gtk_tree_view_get_column (GTK_TREE_VIEW (candidate->treeview), TITEL_COLUMN);
  gtk_tree_view_column_cell_get_size (column, NULL, NULL, NULL, NULL, &cell_height);

  gtk_widget_style_get (candidate->treeview,
                        "horizontal_separator", &horizontal_separator,
                        NULL);

  height = cell_height + horizontal_separator / 2;

  g_print ("SIZE: %d %d\n", cell_height, horizontal_separator);

  guint border_width = gtk_container_get_border_width (GTK_CONTAINER (candidate->window));
  gtk_window_resize (GTK_WINDOW (candidate->window),
                               (gint) ((gdouble) height * 10.0 / 1.6),
                               height * 10 + border_width * 2);

  GtkAdjustment *adjustment;
  adjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (candidate->treeview));
  gtk_adjustment_set_value (adjustment, 0.0);
*/
}

static void
dasom_candidate_finalize (GObject *object)
{
  /*DasomCandidate *candidate = DASOM_CANDIDATE (object);*/

  /* FIXME: 콘테이너 윈도우(candidate->window) 제거할 때,
    자식들도 함께 제거되는지 확인해야 합니다. */
  /*g_object_unref (candidate->window);*/ /* FIXME */

  G_OBJECT_CLASS (dasom_candidate_parent_class)->finalize (object);
}

static void
dasom_candidate_class_init (DasomCandidateClass *class)
{
  GObjectClass* object_class = G_OBJECT_CLASS (class);

  object_class->finalize = dasom_candidate_finalize;

  candidate_signals[ROW_ACTIVATED] =
    g_signal_new (g_intern_static_string ("row-activated"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomCandidateClass, row_activated),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

void
dasom_candidate_update_window (DasomCandidate  *candidate,
                               const gchar    **strv)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeModel *model;
  GtkTreeIter iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));
  gtk_list_store_clear (GTK_LIST_STORE (model));

  guint i;

  for (i = 0; strv[i] != NULL; i++)
  {
    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set    (GTK_LIST_STORE (model), &iter,
                           TITEL_COLUMN, strv[i],
                           -1);
  }
}

void dasom_candidate_show_window (DasomCandidate *candidate)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkTreeModel *model;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (candidate->treeview));

  if (gtk_tree_model_get_iter_first (model, &candidate->iter))
  {
    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (candidate->treeview));
    gtk_tree_selection_select_iter (selection, &candidate->iter);
  }

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
