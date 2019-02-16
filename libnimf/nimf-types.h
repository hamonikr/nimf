/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-types.h
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

#ifndef __NIMF_TYPES_H__
#define __NIMF_TYPES_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define NIMF_ERROR  nimf_error_quark ()

typedef enum
{
  NIMF_ERROR_FAILED
} NimfError;

/* copied from GdkModifierType in gdktypes.h for compatibility */
typedef enum
{
  NIMF_SHIFT_MASK    = 1 << 0, /*< nick=<Shift> >*/
  NIMF_LOCK_MASK     = 1 << 1, /*< nick=<Lock> >*/
  NIMF_CONTROL_MASK  = 1 << 2, /*< nick=<Control> >*/
  NIMF_MOD1_MASK     = 1 << 3, /*< nick=<Mod1> >*/
  NIMF_MOD2_MASK     = 1 << 4, /*< nick=<Mod2> >*/
  NIMF_MOD3_MASK     = 1 << 5, /*< nick=<Mod3> >*/
  NIMF_MOD4_MASK     = 1 << 6, /*< nick=<Mod4> >*/
  NIMF_MOD5_MASK     = 1 << 7, /*< nick=<Mod5> >*/
  NIMF_BUTTON1_MASK  = 1 << 8, /*< nick=<Button1> >*/
  NIMF_BUTTON2_MASK  = 1 << 9, /*< nick=<Button2> >*/
  NIMF_BUTTON3_MASK  = 1 << 10, /*< nick=<Button3> >*/
  NIMF_BUTTON4_MASK  = 1 << 11, /*< nick=<Button4> >*/
  NIMF_BUTTON5_MASK  = 1 << 12, /*< nick=<Button5> >*/

  NIMF_MODIFIER_RESERVED_13_MASK  = 1 << 13,
  NIMF_MODIFIER_RESERVED_14_MASK  = 1 << 14,
  NIMF_MODIFIER_RESERVED_15_MASK  = 1 << 15,
  NIMF_MODIFIER_RESERVED_16_MASK  = 1 << 16,
  NIMF_MODIFIER_RESERVED_17_MASK  = 1 << 17,
  NIMF_MODIFIER_RESERVED_18_MASK  = 1 << 18,
  NIMF_MODIFIER_RESERVED_19_MASK  = 1 << 19,
  NIMF_MODIFIER_RESERVED_20_MASK  = 1 << 20,
  NIMF_MODIFIER_RESERVED_21_MASK  = 1 << 21,
  NIMF_MODIFIER_RESERVED_22_MASK  = 1 << 22,
  NIMF_MODIFIER_RESERVED_23_MASK  = 1 << 23,
  NIMF_MODIFIER_RESERVED_24_MASK  = 1 << 24,
  NIMF_MODIFIER_RESERVED_25_MASK  = 1 << 25,

  NIMF_SUPER_MASK    = 1 << 26, /*< nick=<Super> >*/
  NIMF_HYPER_MASK    = 1 << 27, /*< nick=<Hyper> >*/
  NIMF_META_MASK     = 1 << 28, /*< nick=<Meta> >*/

  NIMF_MODIFIER_RESERVED_29_MASK  = 1 << 29,

  NIMF_RELEASE_MASK  = 1 << 30, /*< nick=<Release> >*/

  /* Combination of NIMF_SHIFT_MASK..NIMF_BUTTON5_MASK + NIMF_SUPER_MASK
     + NIMF_HYPER_MASK + NIMF_META_MASK + NIMF_RELEASE_MASK */
  NIMF_MODIFIER_MASK = 0x5c001fff
} NimfModifierType;

typedef struct {
  int x, y;
  int width, height;
} NimfRectangle;

typedef struct {
  guint32 mods;
  guint32 keyval;
} NimfKey;

typedef enum
{
  NIMF_PREEDIT_STATE_START = 1,
  NIMF_PREEDIT_STATE_END   = 0
} NimfPreeditState;

typedef enum
{
  NIMF_PREEDIT_ATTR_UNDERLINE,
  NIMF_PREEDIT_ATTR_HIGHLIGHT
} NimfPreeditAttrType;

typedef struct {
  NimfPreeditAttrType type;
  guint start_index;
  guint end_index; /* The character at this index is not included */
} NimfPreeditAttr;

GQuark    nimf_error_quark        (void);
NimfKey  *nimf_key_new            (void);
NimfKey  *nimf_key_new_from_nicks (const gchar **nicks);
void      nimf_key_free           (NimfKey      *key);
NimfKey **nimf_key_newv           (const gchar **keys);
void      nimf_key_freev          (NimfKey     **keys);

NimfPreeditAttr  *nimf_preedit_attr_new   (NimfPreeditAttrType type,
                                           guint               start_index,
                                           guint               end_index);
NimfPreeditAttr **nimf_preedit_attrs_copy (NimfPreeditAttr   **attrs);
void              nimf_preedit_attr_free  (NimfPreeditAttr    *attr);
void              nimf_preedit_attr_freev (NimfPreeditAttr   **attrs);

G_END_DECLS

#endif /* __NIMF_TYPES_H__ */
