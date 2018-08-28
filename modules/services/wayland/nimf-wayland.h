/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-wayland.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2012 Intel Corporation
 * Copyright (C) 2017 Hodong Kim <cogniti@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*
 * # 법적 고지
 *
 * Nimf 소프트웨어는 대한민국 저작권법과 국제 조약의 보호를 받습니다.
 * Nimf 개발자는 대한민국 법률의 보호를 받습니다.
 * 커뮤니티의 위력을 이용하여 개발자의 시간과 노동력을 약탈하려는 행위를 금하시기 바랍니다.
 *
 * * 커뮤니티 게시판에 개발자를 욕(비난)하거나
 * * 욕보이는(음해하는) 글을 작성하거나
 * * 허위 사실을 공표하거나
 * * 명예를 훼손하는
 *
 * 등의 행위는 정보통신망 이용촉진 및 정보보호 등에 관한 법률의 제재를 받습니다.
 *
 * # 면책 조항
 *
 * Nimf 는 무료로 배포되는 오픈소스 소프트웨어입니다.
 * Nimf 개발자는 개발 및 유지보수에 대해 어떠한 의무도 없고 어떠한 책임도 없습니다.
 * 어떠한 경우에도 보증하지 않습니다. 도덕적 보증 책임도 없고, 도의적 보증 책임도 없습니다.
 * Nimf 개발자는 리브레오피스, 이클립스 등 귀하가 사용하시는 소프트웨어의 버그를 해결해야 할 의무가 없습니다.
 * Nimf 개발자는 귀하가 사용하시는 배포판에 대해 기술 지원을 해드려야 할 의무가 없습니다.
 */

#ifndef __NIMF_WAYLAND_H__
#define __NIMF_WAYLAND_H__

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <wayland-client.h>
#include <wayland-server.h>
#include <xkbcommon/xkbcommon.h>
#include "input-method-unstable-v1-client-protocol.h"

#define NIMF_TYPE_WAYLAND             (nimf_wayland_get_type ())
#define NIMF_WAYLAND(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_WAYLAND, NimfWayland))
#define NIMF_WAYLAND_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_WAYLAND, NimfWaylandClass))
#define NIMF_IS_WAYLAND(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_WAYLAND))
#define NIMF_IS_WAYLAND_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_WAYLAND))
#define NIMF_WAYLAND_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_WAYLAND, NimfWaylandClass))

typedef struct _NimfWaylandIM  NimfWaylandIM;

typedef struct _NimfWayland      NimfWayland;
typedef struct _NimfWaylandClass NimfWaylandClass;

struct _NimfWaylandClass
{
  NimfServiceClass parent_class;
};

struct _NimfWayland
{
  NimfService parent_instance;

  GSource       *event_source;
  gchar         *id;
  gboolean       active;
  NimfWaylandIM *im;

  struct zwp_input_method_v1 *input_method;
  struct zwp_input_method_context_v1 *context;
  struct wl_display  *display;
  struct wl_registry *registry;
  struct wl_keyboard *keyboard;

  struct xkb_context *xkb_context;

  NimfModifierType modifiers;

  struct xkb_keymap *keymap;
  struct xkb_state *state;
  xkb_mod_mask_t shift_mask;
  xkb_mod_mask_t lock_mask;
  xkb_mod_mask_t control_mask;
  xkb_mod_mask_t mod1_mask;
  xkb_mod_mask_t mod2_mask;
  xkb_mod_mask_t mod3_mask;
  xkb_mod_mask_t mod4_mask;
  xkb_mod_mask_t mod5_mask;
  xkb_mod_mask_t super_mask;
  xkb_mod_mask_t hyper_mask;
  xkb_mod_mask_t meta_mask;

  uint32_t serial;
};

GType nimf_wayland_get_type (void) G_GNUC_CONST;

#endif /* __NIMF_WAYLAND_H__ */
