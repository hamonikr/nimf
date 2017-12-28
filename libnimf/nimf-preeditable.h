/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-preeditable.h
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

#ifndef __NIMF_PREEDITABLE_H__
#define __NIMF_PREEDITABLE_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-types.h"

G_BEGIN_DECLS

#define NIMF_TYPE_PREEDITABLE nimf_preeditable_get_type ()

G_DECLARE_INTERFACE (NimfPreeditable, nimf_preeditable, NIMF, PREEDITABLE, GObject)

struct _NimfPreeditableInterface
{
  GTypeInterface parent;

  void (* show)                (NimfPreeditable     *preeditable);
  void (* hide)                (NimfPreeditable     *preeditable);
  void (* set_text)            (NimfPreeditable     *preeditable,
                                const gchar         *text);
  void (* set_cursor_location) (NimfPreeditable     *preeditable,
                                const NimfRectangle *area);
};

void nimf_preeditable_show                (NimfPreeditable     *preeditable);
void nimf_preeditable_hide                (NimfPreeditable     *preeditable);
void nimf_preeditable_set_text            (NimfPreeditable     *preeditable,
                                           const gchar         *text);
void nimf_preeditable_set_cursor_location (NimfPreeditable     *preeditable,
                                           const NimfRectangle *area);

G_END_DECLS

#endif /* __NIMF_PREEDITABLE_H__ */
