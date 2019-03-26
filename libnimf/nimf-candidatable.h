/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-candidatable.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2018,2019 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_CANDIDATABLE_H__
#define __NIMF_CANDIDATABLE_H__

#include <glib-object.h>
#include "nimf-service-ic.h"

G_BEGIN_DECLS

#define NIMF_TYPE_CANDIDATABLE nimf_candidatable_get_type ()

G_DECLARE_INTERFACE (NimfCandidatable, nimf_candidatable, NIMF, CANDIDATABLE, GObject)

#ifndef __GTK_DOC_IGNORE__
typedef struct _NimfServiceIC NimfServiceIC;
#endif
typedef struct _NimfCandidatable          NimfCandidatable; /* dummy typedef */
typedef struct _NimfCandidatableInterface NimfCandidatableInterface;

/**
 * NimfCandidatableInterface:
 * @parent: The parent interface.
 * @show: Shows the candidatable.
 * @hide: Hides the candidatable.
 * @is_visible: Determines whether the candidatable is visible.
 * @clear: Clears the contents of the candidatable.
 * @set_page_values: Sets page values.
 * @append: Appends a new candidate.
 * @get_selected_index: Gets the index of the selected candidatable.
 * @get_selected_text: Gets the text of the selected candidatable.
 * @select_first_item_in_page: Selects first item in page.
 * @select_last_item_in_page: Selects last item in page.
 * @select_item_by_index_in_page: Selects item in page with the index given as the argument.
 * @select_previous_item: Selects previus item.
 * @select_next_item: Selects a next item.
 * @set_auxiliary_text: Selects auxiliary text.
 *
 * Provides an interface for candidate window.
 */
struct _NimfCandidatableInterface
{
  GTypeInterface parent;

  void     (* show)                         (NimfCandidatable *candidatable,
                                             NimfServiceIC    *target,
                                             gboolean          show_entry);
  void     (* hide)                         (NimfCandidatable *candidatable);
  gboolean (* is_visible)                   (NimfCandidatable *candidatable);
  void     (* clear)                        (NimfCandidatable *candidatable,
                                             NimfServiceIC    *target);
  void     (* set_page_values)              (NimfCandidatable *candidatable,
                                             NimfServiceIC    *target,
                                             gint              page_index,
                                             gint              n_pages,
                                             gint              page_size);
  void     (* append)                       (NimfCandidatable *candidatable,
                                             const gchar      *item1,
                                             const gchar      *item2);
  gint     (* get_selected_index)           (NimfCandidatable *candidatable);
  gchar *  (* get_selected_text)            (NimfCandidatable *candidatable);
  void     (* select_first_item_in_page)    (NimfCandidatable *candidatable);
  void     (* select_last_item_in_page)     (NimfCandidatable *candidatable);
  void     (* select_item_by_index_in_page) (NimfCandidatable *candidatable,
                                             gint              index);
  void     (* select_previous_item)         (NimfCandidatable *candidatable);
  void     (* select_next_item)             (NimfCandidatable *candidatable);
  void     (* set_auxiliary_text)           (NimfCandidatable *candidatable,
                                             const gchar      *text,
                                             gint              cursor_pos);
};

void     nimf_candidatable_show            (NimfCandidatable *candidatable,
                                            NimfServiceIC    *target,
                                            gboolean          show_entry);
void     nimf_candidatable_hide            (NimfCandidatable *candidatable);
gboolean nimf_candidatable_is_visible      (NimfCandidatable *candidatable);
void     nimf_candidatable_clear           (NimfCandidatable *candidatable,
                                            NimfServiceIC    *target);
void     nimf_candidatable_set_page_values (NimfCandidatable *candidatable,
                                            NimfServiceIC    *target,
                                            gint              page_index,
                                            gint              n_pages,
                                            gint              page_size);
void     nimf_candidatable_append          (NimfCandidatable *candidatable,
                                            const gchar      *text1,
                                            const gchar      *text2);
gint     nimf_candidatable_get_selected_index
                                           (NimfCandidatable *candidatable);
gchar   *nimf_candidatable_get_selected_text
                                           (NimfCandidatable *candidatable);
void     nimf_candidatable_select_first_item_in_page
                                           (NimfCandidatable *candidatable);
void     nimf_candidatable_select_last_item_in_page
                                           (NimfCandidatable *candidatable);
void     nimf_candidatable_select_item_by_index_in_page
                                           (NimfCandidatable *candidatable,
                                            gint              index);
void     nimf_candidatable_select_previous_item
                                           (NimfCandidatable *candidatable);
void     nimf_candidatable_select_next_item
                                           (NimfCandidatable *candidatable);
void     nimf_candidatable_set_auxiliary_text
                                           (NimfCandidatable *candidatable,
                                            const gchar      *text,
                                            gint              cursor_pos);
G_END_DECLS

#endif /* __NIMF_CANDIDATABLE_H__ */
