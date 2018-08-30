/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-preeditable.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2017 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-preeditable.h"

G_DEFINE_INTERFACE (NimfPreeditable, nimf_preeditable, G_TYPE_OBJECT)

static void nimf_preeditable_default_init (NimfPreeditableInterface *iface)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void nimf_preeditable_show (NimfPreeditable *preeditable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditableInterface *iface;

  g_return_if_fail (NIMF_IS_PREEDITABLE (preeditable));

  iface = NIMF_PREEDITABLE_GET_IFACE (preeditable);

  if (iface->show)
    iface->show (preeditable);
}

void nimf_preeditable_hide (NimfPreeditable *preeditable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditableInterface *iface;

  g_return_if_fail (NIMF_IS_PREEDITABLE (preeditable));

  iface = NIMF_PREEDITABLE_GET_IFACE (preeditable);

  if (iface->hide)
    iface->hide (preeditable);
}

void nimf_preeditable_set_text (NimfPreeditable *preeditable,
                                const gchar     *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditableInterface *iface;

  g_return_if_fail (NIMF_IS_PREEDITABLE (preeditable));

  iface = NIMF_PREEDITABLE_GET_IFACE (preeditable);

  if (iface->set_text)
    iface->set_text (preeditable, text);
}

void nimf_preeditable_set_cursor_location (NimfPreeditable     *preeditable,
                                           const NimfRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditableInterface *iface;

  g_return_if_fail (NIMF_IS_PREEDITABLE (preeditable));

  iface = NIMF_PREEDITABLE_GET_IFACE (preeditable);

  if (iface->set_cursor_location)
    iface->set_cursor_location (preeditable, area);
}
