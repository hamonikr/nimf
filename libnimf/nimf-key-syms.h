/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-key-syms.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2015 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_KEY_SYMS_H__
#define __NIMF_KEY_SYMS_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

typedef enum
{
  NIMF_KEY_space        = 0x0020,

  NIMF_KEY_0            = 0x030,
  NIMF_KEY_1            = 0x031,
  NIMF_KEY_2            = 0x032,
  NIMF_KEY_3            = 0x033,
  NIMF_KEY_4            = 0x034,
  NIMF_KEY_5            = 0x035,
  NIMF_KEY_6            = 0x036,
  NIMF_KEY_7            = 0x037,
  NIMF_KEY_8            = 0x038,
  NIMF_KEY_9            = 0x039,

  NIMF_KEY_h            = 0x068,

  NIMF_KEY_j            = 0x06a,
  NIMF_KEY_k            = 0x06b,
  NIMF_KEY_l            = 0x06c,

  NIMF_KEY_BackSpace    = 0xff08,
  NIMF_KEY_Return       = 0xff0d,

  NIMF_KEY_Escape       = 0xff1b,

  NIMF_KEY_Hangul       = 0xff31,

  NIMF_KEY_Hangul_Hanja = 0xff34,

  NIMF_KEY_Left         = 0xff51,
  NIMF_KEY_Up           = 0xff52,
  NIMF_KEY_Right        = 0xff53,
  NIMF_KEY_Down         = 0xff54,
  NIMF_KEY_Page_Up      = 0xff55,
  NIMF_KEY_Page_Down    = 0xff56,

  NIMF_KEY_KP_Enter     = 0xff8d,

  NIMF_KEY_KP_Left      = 0xff96,
  NIMF_KEY_KP_Up        = 0xff97,
  NIMF_KEY_KP_Right     = 0xff98,
  NIMF_KEY_KP_Down      = 0xff99,
  NIMF_KEY_KP_Page_Up   = 0xff9a,
  NIMF_KEY_KP_Page_Down = 0xff9b,

  NIMF_KEY_KP_Delete    = 0xff9f,

  NIMF_KEY_KP_Multiply  = 0xffaa,
  NIMF_KEY_KP_Add       = 0xffab,

  NIMF_KEY_KP_Subtract  = 0xffad,
  NIMF_KEY_KP_Decimal   = 0xffae,
  NIMF_KEY_KP_Divide    = 0xffaf,
  NIMF_KEY_KP_0         = 0xffb0,
  NIMF_KEY_KP_1         = 0xffb1,
  NIMF_KEY_KP_2         = 0xffb2,
  NIMF_KEY_KP_3         = 0xffb3,
  NIMF_KEY_KP_4         = 0xffb4,
  NIMF_KEY_KP_5         = 0xffb5,
  NIMF_KEY_KP_6         = 0xffb6,
  NIMF_KEY_KP_7         = 0xffb7,
  NIMF_KEY_KP_8         = 0xffb8,
  NIMF_KEY_KP_9         = 0xffb9,

  NIMF_KEY_F1           = 0xffbe,
  NIMF_KEY_F2           = 0xffbf,
  NIMF_KEY_F3           = 0xffc0,
  NIMF_KEY_F4           = 0xffc1,
  NIMF_KEY_F5           = 0xffc2,
  NIMF_KEY_F6           = 0xffc3,
  NIMF_KEY_F7           = 0xffc4,
  NIMF_KEY_F8           = 0xffc5,
  NIMF_KEY_F9           = 0xffc6,
  NIMF_KEY_F10          = 0xffc7,
  NIMF_KEY_F11          = 0xffc8,
  NIMF_KEY_F12          = 0xffc9,

  NIMF_KEY_Shift_L      = 0xffe1,
  NIMF_KEY_Shift_R      = 0xffe2,
  NIMF_KEY_Control_L    = 0xffe3,
  NIMF_KEY_Control_R    = 0xffe4,

  NIMF_KEY_Meta_L       = 0xffe7,
  NIMF_KEY_Meta_R       = 0xffe8,
  NIMF_KEY_Alt_L        = 0xffe9,
  NIMF_KEY_Alt_R        = 0xffea,

  NIMF_KEY_Delete       = 0xffff,

  NIMF_KEY_VoidSymbol   = 0xffffff
} NimfKeySym;

#endif /* __NIMF_KEY_SYMS_H__ */
