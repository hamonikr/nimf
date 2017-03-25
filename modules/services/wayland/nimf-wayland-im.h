/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-wayland-im.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2017 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_WAYLAND_IM_H__
#define __NIMF_WAYLAND_IM_H__

#include "nimf.h"
#include "nimf-wayland.h"

G_BEGIN_DECLS

#define NIMF_TYPE_WAYLAND_IM             (nimf_wayland_im_get_type ())
#define NIMF_WAYLAND_IM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_WAYLAND_IM, NimfWaylandIM))
#define NIMF_WAYLAND_IM_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_WAYLAND_IM, NimfWaylandIMClass))
#define NIMF_IS_WAYLAND_IM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_WAYLAND_IM))
#define NIMF_IS_WAYLAND_IM_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_WAYLAND_IM))
#define NIMF_WAYLAND_IM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_WAYLAND_IM, NimfWaylandIMClass))

typedef struct _NimfWayland NimfWayland;

typedef struct _NimfWaylandIM      NimfWaylandIM;
typedef struct _NimfWaylandIMClass NimfWaylandIMClass;

struct _NimfWaylandIMClass
{
  NimfServiceIMClass parent_class;
};

struct _NimfWaylandIM
{
  NimfServiceIM parent_instance;
  NimfWayland *wayland;
};

GType          nimf_wayland_im_get_type (void) G_GNUC_CONST;
NimfWaylandIM *nimf_wayland_im_new      (NimfServer  *server,
                                         NimfWayland *wayland);

G_END_DECLS

#endif /* __NIMF_WAYLAND_IM_H__ */

