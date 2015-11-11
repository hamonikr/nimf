/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-key-syms.h
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

#ifndef __DASOM_KEY_SYMS_H__
#define __DASOM_KEY_SYMS_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

typedef enum
{
  DASOM_KEY_space        = 0x0020,

  DASOM_KEY_0            = 0x030,
  DASOM_KEY_1            = 0x031,
  DASOM_KEY_2            = 0x032,
  DASOM_KEY_3            = 0x033,
  DASOM_KEY_4            = 0x034,
  DASOM_KEY_5            = 0x035,
  DASOM_KEY_6            = 0x036,
  DASOM_KEY_7            = 0x037,
  DASOM_KEY_8            = 0x038,
  DASOM_KEY_9            = 0x039,

  DASOM_KEY_h            = 0x068,

  DASOM_KEY_j            = 0x06a,
  DASOM_KEY_k            = 0x06b,
  DASOM_KEY_l            = 0x06c,

  DASOM_KEY_BackSpace    = 0xff08,
  DASOM_KEY_Return       = 0xff0d,

  DASOM_KEY_Escape       = 0xff1b,

  DASOM_KEY_Hangul       = 0xff31,

  DASOM_KEY_Hangul_Hanja = 0xff34,

  DASOM_KEY_Left         = 0xff51,
  DASOM_KEY_Up           = 0xff52,
  DASOM_KEY_Right        = 0xff53,
  DASOM_KEY_Down         = 0xff54,
  DASOM_KEY_Page_Up      = 0xff55,
  DASOM_KEY_Page_Down    = 0xff56,

  DASOM_KEY_KP_Enter     = 0xff8d,

  DASOM_KEY_KP_Left      = 0xff96,
  DASOM_KEY_KP_Up        = 0xff97,
  DASOM_KEY_KP_Right     = 0xff98,
  DASOM_KEY_KP_Down      = 0xff99,
  DASOM_KEY_KP_Page_Up   = 0xff9a,
  DASOM_KEY_KP_Page_Down = 0xff9b,

  DASOM_KEY_KP_Delete    = 0xff9f,

  DASOM_KEY_KP_Multiply  = 0xffaa,
  DASOM_KEY_KP_Add       = 0xffab,

  DASOM_KEY_KP_Subtract  = 0xffad,
  DASOM_KEY_KP_Decimal   = 0xffae,
  DASOM_KEY_KP_Divide    = 0xffaf,
  DASOM_KEY_KP_0         = 0xffb0,
  DASOM_KEY_KP_1         = 0xffb1,
  DASOM_KEY_KP_2         = 0xffb2,
  DASOM_KEY_KP_3         = 0xffb3,
  DASOM_KEY_KP_4         = 0xffb4,
  DASOM_KEY_KP_5         = 0xffb5,
  DASOM_KEY_KP_6         = 0xffb6,
  DASOM_KEY_KP_7         = 0xffb7,
  DASOM_KEY_KP_8         = 0xffb8,
  DASOM_KEY_KP_9         = 0xffb9,

  DASOM_KEY_F1           = 0xffbe,
  DASOM_KEY_F2           = 0xffbf,
  DASOM_KEY_F3           = 0xffc0,
  DASOM_KEY_F4           = 0xffc1,
  DASOM_KEY_F5           = 0xffc2,
  DASOM_KEY_F6           = 0xffc3,
  DASOM_KEY_F7           = 0xffc4,
  DASOM_KEY_F8           = 0xffc5,
  DASOM_KEY_F9           = 0xffc6,
  DASOM_KEY_F10          = 0xffc7,
  DASOM_KEY_F11          = 0xffc8,
  DASOM_KEY_F12          = 0xffc9,

  DASOM_KEY_Shift_L      = 0xffe1,
  DASOM_KEY_Shift_R      = 0xffe2,
  DASOM_KEY_Control_L    = 0xffe3,
  DASOM_KEY_Control_R    = 0xffe4,

  DASOM_KEY_Meta_L       = 0xffe7,
  DASOM_KEY_Meta_R       = 0xffe8,
  DASOM_KEY_Alt_L        = 0xffe9,
  DASOM_KEY_Alt_R        = 0xffea,

  DASOM_KEY_Delete       = 0xffff,

  DASOM_KEY_VoidSymbol   = 0xffffff
} DasomKeySym;

#endif /* __DASOM_KEY_SYMS_H__ */
