/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-wayland-ic.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2017-2019 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-wayland-ic.h"
#include "input-method-unstable-v1-client-protocol.h"

G_DEFINE_TYPE (NimfWaylandIC, nimf_wayland_ic, NIMF_TYPE_SERVICE_IC);

NimfWaylandIC *nimf_wayland_ic_new (NimfWayland *wayland)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWaylandIC *wic;

  wic = g_object_new (NIMF_TYPE_WAYLAND_IC, NULL);
  wic->wayland = wayland;

  return wic;
}

void
nimf_wayland_ic_emit_commit (NimfServiceIC *ic,
                             const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWayland *wayland = NIMF_WAYLAND_IC (ic)->wayland;

  zwp_input_method_context_v1_commit_string (wayland->context,
                                             wayland->serial,
                                             text);
}

void nimf_wayland_ic_emit_preedit_start (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
nimf_wayland_ic_emit_preedit_changed (NimfServiceIC    *ic,
                                      const gchar      *preedit_string,
                                      NimfPreeditAttr **attrs,
                                      gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWayland *wayland = NIMF_WAYLAND_IC (ic)->wayland;

  zwp_input_method_context_v1_preedit_cursor (wayland->context, cursor_pos);
  zwp_input_method_context_v1_preedit_string (wayland->context,
               wayland->serial, preedit_string, preedit_string);
}

void nimf_wayland_ic_emit_preedit_end (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_wayland_ic_init (NimfWaylandIC *wic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_wayland_ic_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  G_OBJECT_CLASS (nimf_wayland_ic_parent_class)->finalize (object);
}

static void
nimf_wayland_ic_class_init (NimfWaylandICClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass       *object_class     = G_OBJECT_CLASS (class);
  NimfServiceICClass *service_im_class = NIMF_SERVICE_IC_CLASS (class);

  object_class->finalize = nimf_wayland_ic_finalize;

  service_im_class->emit_commit          = nimf_wayland_ic_emit_commit;
  service_im_class->emit_preedit_start   = nimf_wayland_ic_emit_preedit_start;
  service_im_class->emit_preedit_changed = nimf_wayland_ic_emit_preedit_changed;
  service_im_class->emit_preedit_end     = nimf_wayland_ic_emit_preedit_end;
}
