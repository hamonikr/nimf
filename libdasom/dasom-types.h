/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-types.h
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

#ifndef __DASOM_TYPES_H__
#define __DASOM_TYPES_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define DASOM_ADDRESS "unix:abstract=dasom"

typedef enum
{
  DASOM_CONNECTION_DASOM_IM,
  DASOM_CONNECTION_DASOM_AGENT,
  DASOM_CONNECTION_XIM
} DasomConnectionType;

/* copied from GdkModifierType in gdktypes.h for compatibility */
typedef enum
{
  DASOM_SHIFT_MASK    = 1 << 0,
  DASOM_LOCK_MASK     = 1 << 1,
  DASOM_CONTROL_MASK  = 1 << 2,
  DASOM_MOD1_MASK     = 1 << 3,
  DASOM_MOD2_MASK     = 1 << 4,
  DASOM_MOD3_MASK     = 1 << 5,
  DASOM_MOD4_MASK     = 1 << 6,
  DASOM_MOD5_MASK     = 1 << 7,
  DASOM_BUTTON1_MASK  = 1 << 8,
  DASOM_BUTTON2_MASK  = 1 << 9,
  DASOM_BUTTON3_MASK  = 1 << 10,
  DASOM_BUTTON4_MASK  = 1 << 11,
  DASOM_BUTTON5_MASK  = 1 << 12,

  DASOM_MODIFIER_RESERVED_13_MASK  = 1 << 13,
  DASOM_MODIFIER_RESERVED_14_MASK  = 1 << 14,
  DASOM_MODIFIER_RESERVED_15_MASK  = 1 << 15,
  DASOM_MODIFIER_RESERVED_16_MASK  = 1 << 16,
  DASOM_MODIFIER_RESERVED_17_MASK  = 1 << 17,
  DASOM_MODIFIER_RESERVED_18_MASK  = 1 << 18,
  DASOM_MODIFIER_RESERVED_19_MASK  = 1 << 19,
  DASOM_MODIFIER_RESERVED_20_MASK  = 1 << 20,
  DASOM_MODIFIER_RESERVED_21_MASK  = 1 << 21,
  DASOM_MODIFIER_RESERVED_22_MASK  = 1 << 22,
  DASOM_MODIFIER_RESERVED_23_MASK  = 1 << 23,
  DASOM_MODIFIER_RESERVED_24_MASK  = 1 << 24,
  DASOM_MODIFIER_RESERVED_25_MASK  = 1 << 25,

  DASOM_SUPER_MASK    = 1 << 26,
  DASOM_HYPER_MASK    = 1 << 27,
  DASOM_META_MASK     = 1 << 28,

  DASOM_MODIFIER_RESERVED_29_MASK  = 1 << 29,

  DASOM_RELEASE_MASK  = 1 << 30,

  /* Combination of DASOM_SHIFT_MASK..DASOM_BUTTON5_MASK + DASOM_SUPER_MASK
     + DASOM_HYPER_MASK + DASOM_META_MASK + DASOM_RELEASE_MASK */
  DASOM_MODIFIER_MASK = 0x5c001fff
} DasomModifierType;

typedef struct {
  int x, y;
  int width, height;
} DasomRectangle;

typedef struct {
  guint mods;
  guint keyval;
} DasomKey;

typedef enum
{
  DASOM_PREEDIT_STATE_START = 1,
  DASOM_PREEDIT_STATE_END   = 0
} DasomPreeditState;

DasomKey  *dasom_key_new            (void);
DasomKey  *dasom_key_new_from_nicks (const gchar **nicks);
void       dasom_key_free           (DasomKey     *key);
DasomKey **dasom_key_newv           (const gchar **keys);
void       dasom_key_freev          (DasomKey    **keys);

G_END_DECLS

#endif /* __DASOM_TYPES_H__ */
