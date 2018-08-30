/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-server-im.h
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

#ifndef __NIMF_SERVER_IM_H__
#define __NIMF_SERVER_IM_H__

#include <glib-object.h>
#include "nimf-service-im.h"

G_BEGIN_DECLS

#define NIMF_TYPE_SERVER_IM             (nimf_server_im_get_type ())
#define NIMF_SERVER_IM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_SERVER_IM, NimfServerIM))
#define NIMF_SERVER_IM_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_SERVER_IM, NimfServerIMClass))
#define NIMF_IS_SERVER_IM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_SERVER_IM))
#define NIMF_IS_SERVER_IM_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_SERVER_IM))
#define NIMF_SERVER_IM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_SERVER_IM, NimfServerIMClass))

typedef struct _NimfServerIM      NimfServerIM;
typedef struct _NimfServerIMClass NimfServerIMClass;

struct _NimfServerIMClass
{
  NimfServiceIMClass parent_class;
};

struct _NimfServerIM
{
  NimfServiceIM parent_instance;
  NimfConnection *connection;
};

GType         nimf_server_im_get_type (void) G_GNUC_CONST;
NimfServerIM *nimf_server_im_new (NimfConnection    *connection,
                                  NimfServer        *server);
G_END_DECLS

#endif /* __NIMF_SERVER_IM_H__ */

