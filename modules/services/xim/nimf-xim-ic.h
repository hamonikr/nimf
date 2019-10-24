/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-xim-ic.h
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

#ifndef __NIMF_XIM_IC_H__
#define __NIMF_XIM_IC_H__

#include <glib-object.h>
#include "nimf.h"
#include "IMdkit/Xi18n.h"
#include "nimf-xim.h"

G_BEGIN_DECLS

#define NIMF_TYPE_XIM_IC             (nimf_xim_ic_get_type ())
#define NIMF_XIM_IC(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_XIM_IC, NimfXimIC))
#define NIMF_XIM_IC_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_XIM_IC, NimfXimICClass))
#define NIMF_IS_XIM_IC(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_XIM_IC))
#define NIMF_IS_XIM_IC_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_XIM_IC))
#define NIMF_XIM_IC_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_XIM_IC, NimfXimICClass))

typedef struct _NimfXim        NimfXim;
typedef struct _NimfXimIC      NimfXimIC;
typedef struct _NimfXimICClass NimfXimICClass;

struct _NimfXimICClass
{
  NimfServiceICClass parent_class;
};

struct _NimfXimIC
{
  NimfServiceIC parent_instance;
  guint16   connect_id;
  guint16   icid;
  gint      prev_preedit_length;
  CARD32    input_style;
  Window    client_window;
  Window    focus_window;
  NimfXim  *xim;
};

GType      nimf_xim_ic_get_type (void) G_GNUC_CONST;
NimfXimIC *nimf_xim_ic_new      (NimfXim *xim,
                                 guint16  connect_id,
                                 guint16  icid);
void       nimf_xim_ic_set_cursor_location (NimfXimIC  *xic,
                                            gint        x,
                                            gint        y);

G_END_DECLS

#endif /* __NIMF_XIM_IC_H__ */

