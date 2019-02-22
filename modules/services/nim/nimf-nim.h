/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-nim.h
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

#ifndef __NIMF_NIM_H__
#define __NIMF_NIM_H__

#include "config.h"
#include <nimf.h>
#include <glib/gi18n.h>

G_BEGIN_DECLS

#define NIMF_TYPE_NIM               (nimf_nim_get_type ())
#define NIMF_NIM(object)            (G_TYPE_CHECK_INSTANCE_CAST ((object), NIMF_TYPE_NIM, NimfNim))
#define NIMF_NIM_CLASS(class)       (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_NIM, NimfNimClass))
#define NIMF_IS_NIM(object)         (G_TYPE_CHECK_INSTANCE_TYPE ((object), NIMF_TYPE_NIM))
#define NIMF_IS_NIM_CLASS(class)    (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_NIM))
#define NIMF_NIM_GET_CLASS(object)  (G_TYPE_INSTANCE_GET_CLASS ((object), NIMF_TYPE_NIM, NimfNimClass))

typedef struct _NimfNim      NimfNim;
typedef struct _NimfNimClass NimfNimClass;

struct _NimfNim
{
  NimfService parent_instance;

  gchar          *id;
  gboolean        active;
  GHashTable     *connections;
  guint16         next_id;
  guint16         last_focused_conn_id;
  guint16         last_focused_icid;
  GSocketService *service;
};

struct _NimfNimClass
{
  NimfServiceClass parent_class;
};

GType nimf_nim_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __NIMF_NIM_H__ */
