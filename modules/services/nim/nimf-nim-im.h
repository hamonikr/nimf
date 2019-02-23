/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-nim-im.h
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

#ifndef __NIMF_NIM_IM_H__
#define __NIMF_NIM_IM_H__

#include <glib-object.h>
#include "nimf-service-im.h"
#include "nimf-connection.h"

G_BEGIN_DECLS

#define NIMF_TYPE_NIM_IM             (nimf_nim_im_get_type ())
#define NIMF_NIM_IM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_NIM_IM, NimfNimIM))
#define NIMF_NIM_IM_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_NIM_IM, NimfNimIMClass))
#define NIMF_IS_NIM_IM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_NIM_IM))
#define NIMF_IS_NIM_IM_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_NIM_IM))
#define NIMF_NIM_IM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_NIM_IM, NimfNimIMClass))

typedef struct _NimfNimIM      NimfNimIM;
typedef struct _NimfNimIMClass NimfNimIMClass;

struct _NimfNimIMClass
{
  NimfServiceIMClass parent_class;
};

struct _NimfNimIM
{
  NimfServiceIM parent_instance;
  NimfConnection *connection;
};

GType      nimf_nim_im_get_type (void) G_GNUC_CONST;
NimfNimIM *nimf_nim_im_new      (NimfConnection *connection);

G_END_DECLS

#endif /* __NIMF_NIM_IM_H__ */
