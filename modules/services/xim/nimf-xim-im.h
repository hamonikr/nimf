/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-xim-im.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2018 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_XIM_IM_H__
#define __NIMF_XIM_IM_H__

#include <glib-object.h>
#include "nimf.h"
#include "IMdkit/Xi18n.h"
#include "nimf-xim.h"

G_BEGIN_DECLS

#define NIMF_TYPE_XIM_IM             (nimf_xim_im_get_type ())
#define NIMF_XIM_IM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_XIM_IM, NimfXimIM))
#define NIMF_XIM_IM_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_XIM_IM, NimfXimIMClass))
#define NIMF_IS_XIM_IM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_XIM_IM))
#define NIMF_IS_XIM_IM_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_XIM_IM))
#define NIMF_XIM_IM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_XIM_IM, NimfXimIMClass))

typedef struct _NimfXim        NimfXim;
typedef struct _NimfXimIM      NimfXimIM;
typedef struct _NimfXimIMClass NimfXimIMClass;

struct _NimfXimIMClass
{
  NimfServiceIMClass parent_class;
};

struct _NimfXimIM
{
  NimfServiceIM parent_instance;
  guint16   connect_id;
  gint      prev_preedit_length;
  CARD32    input_style;
  gboolean  draw_preedit_on_the_server_side;
  Window    client_window;
  Window    focus_window;
  NimfXim  *xim;
};

GType nimf_xim_im_get_type (void) G_GNUC_CONST;
NimfXimIM *nimf_xim_im_new (NimfServer *server,
                            NimfXim    *xim);

G_END_DECLS

#endif /* __NIMF_XIM_IM_H__ */

