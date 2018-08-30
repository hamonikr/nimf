/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-client.h
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

#ifndef __NIMF_CLIENT_H__
#define __NIMF_CLIENT_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define NIMF_TYPE_CLIENT             (nimf_client_get_type ())
#define NIMF_CLIENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_CLIENT, NimfClient))
#define NIMF_CLIENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_CLIENT, NimfClientClass))
#define NIMF_IS_CLIENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_CLIENT))
#define NIMF_IS_CLIENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_CLIENT))
#define NIMF_CLIENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_CLIENT, NimfClientClass))

typedef struct _NimfClient      NimfClient;
typedef struct _NimfClientClass NimfClientClass;

struct _NimfClient
{
  GObject parent_instance;

  guint16       id;
  GFileMonitor *monitor;
  uid_t         uid;
  gboolean      created;
};

struct _NimfClientClass
{
  GObjectClass parent_class;
};

GType    nimf_client_get_type     (void) G_GNUC_CONST;
gboolean nimf_client_is_connected (void);

G_END_DECLS

#endif /* __NIMF_CLIENT_H__ */
