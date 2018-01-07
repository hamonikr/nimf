/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-candidatable.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2018 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-candidatable.h"

G_DEFINE_INTERFACE (NimfCandidatable, nimf_candidatable, G_TYPE_OBJECT)

static void
nimf_candidatable_default_init (NimfCandidatableInterface *iface)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
nimf_candidatable_show (NimfCandidatable *candidatable,
                        NimfServiceIM    *target,
                        gboolean          show_entry)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->show)
    iface->show (candidatable, target, show_entry);
}

void
nimf_candidatable_hide (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->hide)
    iface->hide (candidatable);
}

gboolean
nimf_candidatable_is_visible (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_val_if_fail (NIMF_IS_CANDIDATABLE (candidatable), FALSE);

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->is_visible)
    return iface->is_visible (candidatable);

  return FALSE;
}

void
nimf_candidatable_clear (NimfCandidatable *candidatable,
                         NimfServiceIM    *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->clear)
    iface->clear (candidatable, target);
}

void
nimf_candidatable_set_page_values (NimfCandidatable *candidatable,
                                   NimfServiceIM    *target,
                                   gint              page_index,
                                   gint              n_pages,
                                   gint              page_size)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->set_page_values)
    iface->set_page_values (candidatable,
                            target,
                            page_index,
                            n_pages,
                            page_size);
}

void
nimf_candidatable_append (NimfCandidatable *candidatable,
                          const gchar      *item1,
                          const gchar      *item2)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->append)
    iface->append (candidatable, item1, item2);
}

gint
nimf_candidatable_get_selected_index (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_val_if_fail (NIMF_IS_CANDIDATABLE (candidatable), 0);

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->get_selected_index)
    return iface->get_selected_index (candidatable);

  return 0;
}

gchar *
nimf_candidatable_get_selected_text (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_val_if_fail (NIMF_IS_CANDIDATABLE (candidatable), NULL);

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->get_selected_text)
    return iface->get_selected_text (candidatable);

  return NULL;
}

void
nimf_candidatable_select_first_item_in_page (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->select_first_item_in_page)
    iface->select_first_item_in_page (candidatable);
}

void
nimf_candidatable_select_last_item_in_page (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->select_last_item_in_page)
    iface->select_last_item_in_page (candidatable);
}

void
nimf_candidatable_select_item_by_index_in_page (NimfCandidatable *candidatable,
                                                gint              index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->select_item_by_index_in_page)
    iface->select_item_by_index_in_page (candidatable, index);
}

void
nimf_candidatable_select_previous_item (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->select_previous_item)
    iface->select_previous_item (candidatable);
}

void
nimf_candidatable_select_next_item (NimfCandidatable *candidatable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->select_next_item)
    iface->select_next_item (candidatable);
}

void
nimf_candidatable_set_auxiliary_text (NimfCandidatable *candidatable,
                                      const gchar      *text,
                                      gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfCandidatableInterface *iface;

  g_return_if_fail (NIMF_IS_CANDIDATABLE (candidatable));

  iface = NIMF_CANDIDATABLE_GET_IFACE (candidatable);

  if (iface->set_auxiliary_text)
    iface->set_auxiliary_text (candidatable, text, cursor_pos);
}
