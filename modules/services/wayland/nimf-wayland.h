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

typedef struct _NimfWaylandIC  NimfWaylandIC;

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
  NimfWaylandIC *wic;

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
