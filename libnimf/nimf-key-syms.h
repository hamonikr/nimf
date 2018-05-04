/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-key-syms.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2017 Hodong Kim <cogniti@gmail.com>
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

#include <glib.h>

G_BEGIN_DECLS

typedef enum
{
  NIMF_KEY_space            = 0x020, /*< nick=space >*/
  NIMF_KEY_exclam           = 0x021, /*< nick=exclam >*/
  NIMF_KEY_quotedbl         = 0x022, /*< nick=quotedbl >*/
  NIMF_KEY_numbersign       = 0x023, /*< nick=numbersign >*/
  NIMF_KEY_dollar           = 0x024, /*< nick=dollar >*/
  NIMF_KEY_percent          = 0x025, /*< nick=percent >*/
  NIMF_KEY_ampersand        = 0x026, /*< nick=ampersand >*/
  NIMF_KEY_apostrophe       = 0x027, /*< nick=apostrophe >*/
  NIMF_KEY_parenleft        = 0x028, /*< nick=parenleft >*/
  NIMF_KEY_parenright       = 0x029, /*< nick=parenright >*/
  NIMF_KEY_asterisk         = 0x02a, /*< nick=asterisk >*/
  NIMF_KEY_plus             = 0x02b, /*< nick=plus >*/
  NIMF_KEY_comma            = 0x02c, /*< nick=comma >*/
  NIMF_KEY_minus            = 0x02d, /*< nick=minus >*/
  NIMF_KEY_period           = 0x02e, /*< nick=period >*/
  NIMF_KEY_slash            = 0x02f, /*< nick=slash >*/
  NIMF_KEY_0                = 0x030, /*< nick=0 >*/
  NIMF_KEY_1                = 0x031, /*< nick=1 >*/
  NIMF_KEY_2                = 0x032, /*< nick=2 >*/
  NIMF_KEY_3                = 0x033, /*< nick=3 >*/
  NIMF_KEY_4                = 0x034, /*< nick=4 >*/
  NIMF_KEY_5                = 0x035, /*< nick=5 >*/
  NIMF_KEY_6                = 0x036, /*< nick=6 >*/
  NIMF_KEY_7                = 0x037, /*< nick=7 >*/
  NIMF_KEY_8                = 0x038, /*< nick=8 >*/
  NIMF_KEY_9                = 0x039, /*< nick=9 >*/
  NIMF_KEY_colon            = 0x03a, /*< nick=colon >*/
  NIMF_KEY_semicolon        = 0x03b, /*< nick=semicolon >*/
  NIMF_KEY_less             = 0x03c, /*< nick=less >*/
  NIMF_KEY_equal            = 0x03d, /*< nick=equal >*/
  NIMF_KEY_greater          = 0x03e, /*< nick=greater >*/
  NIMF_KEY_question         = 0x03f, /*< nick=question >*/
  NIMF_KEY_at               = 0x040, /*< nick=at >*/
  NIMF_KEY_A                = 0x041, /*< nick=A >*/
  NIMF_KEY_B                = 0x042, /*< nick=B >*/
  NIMF_KEY_C                = 0x043, /*< nick=C >*/
  NIMF_KEY_D                = 0x044, /*< nick=D >*/
  NIMF_KEY_E                = 0x045, /*< nick=E >*/
  NIMF_KEY_F                = 0x046, /*< nick=F >*/
  NIMF_KEY_G                = 0x047, /*< nick=G >*/
  NIMF_KEY_H                = 0x048, /*< nick=H >*/
  NIMF_KEY_I                = 0x049, /*< nick=I >*/
  NIMF_KEY_J                = 0x04a, /*< nick=J >*/
  NIMF_KEY_K                = 0x04b, /*< nick=K >*/
  NIMF_KEY_L                = 0x04c, /*< nick=L >*/
  NIMF_KEY_M                = 0x04d, /*< nick=M >*/
  NIMF_KEY_N                = 0x04e, /*< nick=N >*/
  NIMF_KEY_O                = 0x04f, /*< nick=O >*/
  NIMF_KEY_P                = 0x050, /*< nick=P >*/
  NIMF_KEY_Q                = 0x051, /*< nick=Q >*/
  NIMF_KEY_R                = 0x052, /*< nick=R >*/
  NIMF_KEY_S                = 0x053, /*< nick=S >*/
  NIMF_KEY_T                = 0x054, /*< nick=T >*/
  NIMF_KEY_U                = 0x055, /*< nick=U >*/
  NIMF_KEY_V                = 0x056, /*< nick=V >*/
  NIMF_KEY_W                = 0x057, /*< nick=W >*/
  NIMF_KEY_X                = 0x058, /*< nick=X >*/
  NIMF_KEY_Y                = 0x059, /*< nick=Y >*/
  NIMF_KEY_Z                = 0x05a, /*< nick=Z >*/
  NIMF_KEY_bracketleft      = 0x05b, /*< nick=bracketleft >*/
  NIMF_KEY_backslash        = 0x05c, /*< nick=backslash >*/
  NIMF_KEY_bracketright     = 0x05d, /*< nick=bracketright >*/
  NIMF_KEY_asciicircum      = 0x05e, /*< nick=asciicircum >*/
  NIMF_KEY_underscore       = 0x05f, /*< nick=underscore >*/
  NIMF_KEY_grave            = 0x060, /*< nick=grave >*/
  NIMF_KEY_a                = 0x061, /*< nick=a >*/
  NIMF_KEY_b                = 0x062, /*< nick=b >*/
  NIMF_KEY_c                = 0x063, /*< nick=c >*/
  NIMF_KEY_d                = 0x064, /*< nick=d >*/
  NIMF_KEY_e                = 0x065, /*< nick=e >*/
  NIMF_KEY_f                = 0x066, /*< nick=f >*/
  NIMF_KEY_g                = 0x067, /*< nick=g >*/
  NIMF_KEY_h                = 0x068, /*< nick=h >*/
  NIMF_KEY_i                = 0x069, /*< nick=i >*/
  NIMF_KEY_j                = 0x06a, /*< nick=j >*/
  NIMF_KEY_k                = 0x06b, /*< nick=k >*/
  NIMF_KEY_l                = 0x06c, /*< nick=l >*/
  NIMF_KEY_m                = 0x06d, /*< nick=m >*/
  NIMF_KEY_n                = 0x06e, /*< nick=n >*/
  NIMF_KEY_o                = 0x06f, /*< nick=o >*/
  NIMF_KEY_p                = 0x070, /*< nick=p >*/
  NIMF_KEY_q                = 0x071, /*< nick=q >*/
  NIMF_KEY_r                = 0x072, /*< nick=r >*/
  NIMF_KEY_s                = 0x073, /*< nick=s >*/
  NIMF_KEY_t                = 0x074, /*< nick=t >*/
  NIMF_KEY_u                = 0x075, /*< nick=u >*/
  NIMF_KEY_v                = 0x076, /*< nick=v >*/
  NIMF_KEY_w                = 0x077, /*< nick=w >*/
  NIMF_KEY_x                = 0x078, /*< nick=x >*/
  NIMF_KEY_y                = 0x079, /*< nick=y >*/
  NIMF_KEY_z                = 0x07a, /*< nick=z >*/
  NIMF_KEY_braceleft        = 0x07b, /*< nick=braceleft >*/
  NIMF_KEY_bar              = 0x07c, /*< nick=bar >*/
  NIMF_KEY_braceright       = 0x07d, /*< nick=braceright >*/
  NIMF_KEY_asciitilde       = 0x07e, /*< nick=asciitilde >*/

  NIMF_KEY_ISO_Level3_Shift = 0xfe03, /*< nick=ISO_Level3_Shift >*/
  NIMF_KEY_ISO_Left_Tab     = 0xfe20, /*< nick=ISO_Left_Tab >*/

  NIMF_KEY_BackSpace        = 0xff08, /*< nick=BackSpace >*/
  NIMF_KEY_Tab              = 0xff09, /*< nick=Tab >*/

  NIMF_KEY_Return           = 0xff0d, /*< nick=Return >*/

  NIMF_KEY_Pause            = 0xff13, /*< nick=Pause >*/
  NIMF_KEY_Scroll_Lock      = 0xff14, /*< nick=Scroll_Lock >*/
  NIMF_KEY_Sys_Req          = 0xff15, /*< nick=Sys_Req >*/

  NIMF_KEY_Escape           = 0xff1b, /*< nick=Escape >*/

  NIMF_KEY_Multi_key        = 0xff20, /*< nick=Multi_key >*/

  NIMF_KEY_Hangul           = 0xff31, /*< nick=Hangul >*/

  NIMF_KEY_Hangul_Hanja     = 0xff34, /*< nick=Hangul_Hanja >*/

  NIMF_KEY_Home             = 0xff50, /*< nick=Home >*/
  NIMF_KEY_Left             = 0xff51, /*< nick=Left >*/
  NIMF_KEY_Up               = 0xff52, /*< nick=Up >*/
  NIMF_KEY_Right            = 0xff53, /*< nick=Right >*/
  NIMF_KEY_Down             = 0xff54, /*< nick=Down >*/
  NIMF_KEY_Page_Up          = 0xff55, /*< nick=Page_Up >*/
  NIMF_KEY_Page_Down        = 0xff56, /*< nick=Page_Down >*/
  NIMF_KEY_End              = 0xff57, /*< nick=End >*/

  NIMF_KEY_Print            = 0xff61, /*< nick=Print >*/
  NIMF_KEY_Execute          = 0xff62, /*< nick=Execut >*/
  NIMF_KEY_Insert           = 0xff63, /*< nick=Insert >*/

  NIMF_KEY_Menu             = 0xff67, /*< nick=Menu >*/

  NIMF_KEY_Break            = 0xff6b, /*< nick=Break >*/

  NIMF_KEY_KP_Enter         = 0xff8d, /*< nick=KP_Enter >*/

  NIMF_KEY_KP_Left          = 0xff96, /*< nick=KP_Left >*/
  NIMF_KEY_KP_Up            = 0xff97, /*< nick=KP_Up >*/
  NIMF_KEY_KP_Right         = 0xff98, /*< nick=KP_Right >*/
  NIMF_KEY_KP_Down          = 0xff99, /*< nick=KP_Down >*/
  NIMF_KEY_KP_Page_Up       = 0xff9a, /*< nick=KP_Page_Up >*/
  NIMF_KEY_KP_Page_Down     = 0xff9b, /*< nick=KP_Page_Down >*/

  NIMF_KEY_KP_Delete        = 0xff9f, /*< nick=KP_Delete >*/

  NIMF_KEY_KP_Multiply      = 0xffaa, /*< nick=KP_Multiply >*/
  NIMF_KEY_KP_Add           = 0xffab, /*< nick=KP_Add >*/

  NIMF_KEY_KP_Subtract      = 0xffad, /*< nick=KP_Subtract >*/
  NIMF_KEY_KP_Decimal       = 0xffae, /*< nick=KP_Decimal >*/
  NIMF_KEY_KP_Divide        = 0xffaf, /*< nick=KP_Divide >*/
  NIMF_KEY_KP_0             = 0xffb0, /*< nick=KP_0 >*/
  NIMF_KEY_KP_1             = 0xffb1, /*< nick=KP_1 >*/
  NIMF_KEY_KP_2             = 0xffb2, /*< nick=KP_2 >*/
  NIMF_KEY_KP_3             = 0xffb3, /*< nick=KP_3 >*/
  NIMF_KEY_KP_4             = 0xffb4, /*< nick=KP_4 >*/
  NIMF_KEY_KP_5             = 0xffb5, /*< nick=KP_5 >*/
  NIMF_KEY_KP_6             = 0xffb6, /*< nick=KP_6 >*/
  NIMF_KEY_KP_7             = 0xffb7, /*< nick=KP_7 >*/
  NIMF_KEY_KP_8             = 0xffb8, /*< nick=KP_8 >*/
  NIMF_KEY_KP_9             = 0xffb9, /*< nick=KP_9 >*/

  NIMF_KEY_F1               = 0xffbe, /*< nick=F1 >*/
  NIMF_KEY_F2               = 0xffbf, /*< nick=F2 >*/
  NIMF_KEY_F3               = 0xffc0, /*< nick=F3 >*/
  NIMF_KEY_F4               = 0xffc1, /*< nick=F4 >*/
  NIMF_KEY_F5               = 0xffc2, /*< nick=F5 >*/
  NIMF_KEY_F6               = 0xffc3, /*< nick=F6 >*/
  NIMF_KEY_F7               = 0xffc4, /*< nick=F7 >*/
  NIMF_KEY_F8               = 0xffc5, /*< nick=F8 >*/
  NIMF_KEY_F9               = 0xffc6, /*< nick=F9 >*/
  NIMF_KEY_F10              = 0xffc7, /*< nick=F10 >*/
  NIMF_KEY_F11              = 0xffc8, /*< nick=F11 >*/
  NIMF_KEY_F12              = 0xffc9, /*< nick=F12 >*/

  NIMF_KEY_Shift_L          = 0xffe1, /*< nick=Shift_L >*/
  NIMF_KEY_Shift_R          = 0xffe2, /*< nick=Shift_R >*/
  NIMF_KEY_Control_L        = 0xffe3, /*< nick=Control_L >*/
  NIMF_KEY_Control_R        = 0xffe4, /*< nick=Control_R >*/
  NIMF_KEY_Caps_Lock        = 0xffe5, /*< nick=Caps_Lock >*/
  NIMF_KEY_Shift_Lock       = 0xffe6, /*< nick=Shift_Lock >*/
  NIMF_KEY_Meta_L           = 0xffe7, /*< nick=Meta_L >*/
  NIMF_KEY_Meta_R           = 0xffe8, /*< nick=Meta_R >*/
  NIMF_KEY_Alt_L            = 0xffe9, /*< nick=Alt_L >*/
  NIMF_KEY_Alt_R            = 0xffea, /*< nick=Alt_R >*/
  NIMF_KEY_Super_L          = 0xffeb, /*< nick=Super_L >*/
  NIMF_KEY_Super_R          = 0xffec, /*< nick=Super_R >*/

  NIMF_KEY_Delete           = 0xffff, /*< nick=Delete >*/

  NIMF_KEY_VoidSymbol       = 0xffffff, /*< nick=VoidSymbol >*/
  NIMF_KEY_WakeUp           = 0x1008ff2b, /*< nick=WakeUp >*/
  NIMF_KEY_WebCam           = 0x1008ff8f, /*< nick=WebCam >*/
  NIMF_KEY_WLAN             = 0x1008ff95 /*< nick=WLAN >*/
} NimfKeySym;

const gchar *nimf_keyval_to_keysym_name (guint keyval);

G_END_DECLS

#endif /* __NIMF_KEY_SYMS_H__ */
