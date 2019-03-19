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

#include <glib-object.h>

G_BEGIN_DECLS

/* copied from GdkModifierType in gdktypes.h for compatibility */
/**
 * NimfModifierType:
 * @NIMF_SHIFT_MASK: the Shift key.
 * @NIMF_LOCK_MASK: a Lock key (depending on the modifier mapping of the
 *  X server this may either be CapsLock or ShiftLock).
 * @NIMF_CONTROL_MASK: the Control key.
 * @NIMF_MOD1_MASK: the fourth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier, but
 *  normally it is the Alt key).
 * @NIMF_MOD2_MASK: the fifth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @NIMF_MOD3_MASK: the sixth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @NIMF_MOD4_MASK: the seventh modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @NIMF_MOD5_MASK: the eighth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @NIMF_BUTTON1_MASK: the first mouse button.
 * @NIMF_BUTTON2_MASK: the second mouse button.
 * @NIMF_BUTTON3_MASK: the third mouse button.
 * @NIMF_BUTTON4_MASK: the fourth mouse button.
 * @NIMF_BUTTON5_MASK: the fifth mouse button.
 * @NIMF_MODIFIER_RESERVED_13_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_14_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_15_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_16_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_17_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_18_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_19_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_20_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_21_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_22_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_23_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_24_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_MODIFIER_RESERVED_25_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_SUPER_MASK: the Super modifier.
 * @NIMF_HYPER_MASK: the Hyper modifier.
 * @NIMF_META_MASK: the Meta modifier.
 * @NIMF_MODIFIER_RESERVED_29_MASK: A reserved bit flag; do not use in your own code
 * @NIMF_RELEASE_MASK: exists because of compatibility.
 * @NIMF_MODIFIER_MASK: a mask covering all modifier types.
 *
 * A set of bit-flags to indicate the state of modifier keys and mouse buttons
 * in various event types. Typical modifier keys are Shift, Control, Meta,
 * Super, Hyper, Alt, Compose, Apple, CapsLock or ShiftLock.
 */
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

/**
 * NimfRectangle:
 * @x: x
 * @y: y
 * @width: width
 * @height: height
 *
 * Defines the position and size of a rectangle.
 */
typedef struct {
  int x, y;
  int width, height;
} NimfRectangle;

/**
 * NimfKey:
 * @state: a bit-mask representing the state of the modifier keys
 * @keyval: the key that was pressed or released.
 */
typedef struct {
  guint32 state;
  guint32 keyval;
} NimfKey;

/**
 * NimfPreeditState:
 * @NIMF_PREEDIT_STATE_START: presents preedit-start state.
 * @NIMF_PREEDIT_STATE_END: presents preedit-end state.
 */
typedef enum
{
  NIMF_PREEDIT_STATE_START = 1,
  NIMF_PREEDIT_STATE_END   = 0
} NimfPreeditState;

/**
 * NimfPreeditAttrType:
 * @NIMF_PREEDIT_ATTR_UNDERLINE: whether the text has an underline
 * @NIMF_PREEDIT_ATTR_HIGHLIGHT: whether the text has a highlight
 */
typedef enum
{
  NIMF_PREEDIT_ATTR_UNDERLINE,
  NIMF_PREEDIT_ATTR_HIGHLIGHT
} NimfPreeditAttrType;

/**
 * NimfPreeditAttr:
 * @type: a #NimfPreeditAttrType
 * @start_index: the start index of the range (in characters).
 * @end_index: end index of the range (in characters). The character at this
 *   index is not included in the range.
 */
typedef struct {
  NimfPreeditAttrType type;
  guint start_index; /* in characters */
  guint end_index; /* in characters. The character at this index is not included */
} NimfPreeditAttr;

/**
 * NimfMethodInfo:
 * @method_id: method id of the engine
 * @label: human readable label
 * @group: human readable group name
 */
typedef struct
{
  gchar *method_id; /* method id */
  gchar *label;     /* Human readable label */
  gchar *group;     /* Human readable group name */
} NimfMethodInfo;

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

NimfMethodInfo *nimf_method_info_new      (void);
void            nimf_method_info_free     (NimfMethodInfo  *info);
void            nimf_method_info_freev    (NimfMethodInfo **infos);

G_END_DECLS

#endif /* __NIMF_TYPES_H__ */
