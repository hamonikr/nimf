/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-key-syms.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2016 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-key-syms.h"

GType
nimf_key_sym_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
  {
    static const GEnumValue values[] = {
      { NIMF_KEY_space, "NIMF_KEY_space", "space" },
      { NIMF_KEY_0, "NIMF_KEY_0", "0" },
      { NIMF_KEY_1, "NIMF_KEY_1", "1" },
      { NIMF_KEY_2, "NIMF_KEY_2", "2" },
      { NIMF_KEY_3, "NIMF_KEY_3", "3" },
      { NIMF_KEY_4, "NIMF_KEY_4", "4" },
      { NIMF_KEY_5, "NIMF_KEY_5", "5" },
      { NIMF_KEY_6, "NIMF_KEY_6", "6" },
      { NIMF_KEY_7, "NIMF_KEY_7", "7" },
      { NIMF_KEY_8, "NIMF_KEY_8", "8" },
      { NIMF_KEY_9, "NIMF_KEY_9", "9" },
      { NIMF_KEY_h, "NIMF_KEY_h", "h" },
      { NIMF_KEY_j, "NIMF_KEY_j", "j" },
      { NIMF_KEY_k, "NIMF_KEY_k", "k" },
      { NIMF_KEY_l, "NIMF_KEY_l", "l" },
      { NIMF_KEY_BackSpace, "NIMF_KEY_BackSpace", "backspace" },
      { NIMF_KEY_Return, "NIMF_KEY_Return", "return" },
      { NIMF_KEY_Escape, "NIMF_KEY_Escape", "escape" },
      { NIMF_KEY_Hangul, "NIMF_KEY_Hangul", "hangul" },
      { NIMF_KEY_Hangul_Hanja, "NIMF_KEY_Hangul_Hanja", "hangul-hanja" },
      { NIMF_KEY_Left, "NIMF_KEY_Left", "left" },
      { NIMF_KEY_Up, "NIMF_KEY_Up", "up" },
      { NIMF_KEY_Right, "NIMF_KEY_Right", "right" },
      { NIMF_KEY_Down, "NIMF_KEY_Down", "down" },
      { NIMF_KEY_Page_Up, "NIMF_KEY_Page_Up", "page-up" },
      { NIMF_KEY_Page_Down, "NIMF_KEY_Page_Down", "page-down" },
      { NIMF_KEY_KP_Enter, "NIMF_KEY_KP_Enter", "kp-enter" },
      { NIMF_KEY_KP_Left, "NIMF_KEY_KP_Left", "kp-left" },
      { NIMF_KEY_KP_Up, "NIMF_KEY_KP_Up", "kp-up" },
      { NIMF_KEY_KP_Right, "NIMF_KEY_KP_Right", "kp-right" },
      { NIMF_KEY_KP_Down, "NIMF_KEY_KP_Down", "kp-down" },
      { NIMF_KEY_KP_Page_Up, "NIMF_KEY_KP_Page_Up", "kp-page-up" },
      { NIMF_KEY_KP_Page_Down, "NIMF_KEY_KP_Page_Down", "kp-page-down" },
      { NIMF_KEY_KP_Delete, "NIMF_KEY_KP_Delete", "kp-delete" },
      { NIMF_KEY_KP_Multiply, "NIMF_KEY_KP_Multiply", "kp-multiply" },
      { NIMF_KEY_KP_Add, "NIMF_KEY_KP_Add", "kp-add" },
      { NIMF_KEY_KP_Subtract, "NIMF_KEY_KP_Subtract", "kp-subtract" },
      { NIMF_KEY_KP_Decimal, "NIMF_KEY_KP_Decimal", "kp-decimal" },
      { NIMF_KEY_KP_Divide, "NIMF_KEY_KP_Divide", "kp-divide" },
      { NIMF_KEY_KP_0, "NIMF_KEY_KP_0", "kp-0" },
      { NIMF_KEY_KP_1, "NIMF_KEY_KP_1", "kp-1" },
      { NIMF_KEY_KP_2, "NIMF_KEY_KP_2", "kp-2" },
      { NIMF_KEY_KP_3, "NIMF_KEY_KP_3", "kp-3" },
      { NIMF_KEY_KP_4, "NIMF_KEY_KP_4", "kp-4" },
      { NIMF_KEY_KP_5, "NIMF_KEY_KP_5", "kp-5" },
      { NIMF_KEY_KP_6, "NIMF_KEY_KP_6", "kp-6" },
      { NIMF_KEY_KP_7, "NIMF_KEY_KP_7", "kp-7" },
      { NIMF_KEY_KP_8, "NIMF_KEY_KP_8", "kp-8" },
      { NIMF_KEY_KP_9, "NIMF_KEY_KP_9", "kp-9" },
      { NIMF_KEY_F1, "NIMF_KEY_F1", "f1" },
      { NIMF_KEY_F2, "NIMF_KEY_F2", "f2" },
      { NIMF_KEY_F3, "NIMF_KEY_F3", "f3" },
      { NIMF_KEY_F4, "NIMF_KEY_F4", "f4" },
      { NIMF_KEY_F5, "NIMF_KEY_F5", "f5" },
      { NIMF_KEY_F6, "NIMF_KEY_F6", "f6" },
      { NIMF_KEY_F7, "NIMF_KEY_F7", "f7" },
      { NIMF_KEY_F8, "NIMF_KEY_F8", "f8" },
      { NIMF_KEY_F9, "NIMF_KEY_F9", "f9" },
      { NIMF_KEY_F10, "NIMF_KEY_F10", "f10" },
      { NIMF_KEY_F11, "NIMF_KEY_F11", "f11" },
      { NIMF_KEY_F12, "NIMF_KEY_F12", "f12" },
      { NIMF_KEY_Shift_L, "NIMF_KEY_Shift_L", "shift-l" },
      { NIMF_KEY_Shift_R, "NIMF_KEY_Shift_R", "shift-r" },
      { NIMF_KEY_Control_L, "NIMF_KEY_Control_L", "control-l" },
      { NIMF_KEY_Control_R, "NIMF_KEY_Control_R", "control-r" },
      { NIMF_KEY_Meta_L, "NIMF_KEY_Meta_L", "meta-l" },
      { NIMF_KEY_Meta_R, "NIMF_KEY_Meta_R", "meta-r" },
      { NIMF_KEY_Alt_L, "NIMF_KEY_Alt_L", "alt-l" },
      { NIMF_KEY_Alt_R, "NIMF_KEY_Alt_R", "alt-r" },
      { NIMF_KEY_Delete, "NIMF_KEY_Delete", "delete" },
      { NIMF_KEY_VoidSymbol, "NIMF_KEY_VoidSymbol", "voidsymbol" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id =
      g_enum_register_static (g_intern_static_string ("NimfKeySym"), values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }

  return g_define_type_id__volatile;
}

/* Thanks to Markus G. Kuhn <mkuhn@acm.org> for the ksysym to Unicode
 * mapping function, from the keysym2ucs.c which is in the public domain.
 * nimf_keyval_to_unicode() is the same as gdk_keyval_to_unicode()
 * for compatibility.
 */
guint32
nimf_keyval_to_unicode (guint keyval)
{
  int min = 0;
  int max = G_N_ELEMENTS (nimf_keysym_to_unicode_table) - 1;
  int mid;

  /* First check for Latin-1 characters (1:1 mapping) */
  if ((keyval >= 0x0020 && keyval <= 0x007e) ||
      (keyval >= 0x00a0 && keyval <= 0x00ff))
    return keyval;

  /* Also check for directly encoded 24-bit UCS characters:
   */
  if ((keyval & 0xff000000) == 0x01000000)
    return keyval & 0x00ffffff;

  /* binary search in table */
  while (max >= min) {
    mid = (min + max) / 2;
    if (nimf_keysym_to_unicode_table[mid].keysym < keyval)
      min = mid + 1;
    else if (nimf_keysym_to_unicode_table[mid].keysym > keyval)
      max = mid - 1;
    else {
      /* found it */
      return nimf_keysym_to_unicode_table[mid].ucs;
    }
  }

  /* No matching Unicode value found */
  return 0;
}
