/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-connection.h
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

#ifndef __NIMF_CONNECTION_H__
#define __NIMF_CONNECTION_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-engine.h"
#include "nimf-server.h"
#include "nimf-private.h"
#include <X11/Xlib.h>

G_BEGIN_DECLS

#define NIMF_TYPE_CONNECTION             (nimf_connection_get_type ())
#define NIMF_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_CONNECTION, NimfConnection))
#define NIMF_CONNECTION_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_CONNECTION, NimfConnectionClass))
#define NIMF_IS_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_CONNECTION))
#define NIMF_IS_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_CONNECTION))
#define NIMF_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_CONNECTION, NimfConnectionClass))

typedef struct _NimfServer NimfServer;
typedef struct _NimfEngine NimfEngine;
typedef struct _NimfResult NimfResult;

typedef struct _NimfConnection      NimfConnection;
typedef struct _NimfConnectionClass NimfConnectionClass;

struct _NimfConnection
{
  GObject parent_instance;

  guint16            id;
  NimfServer        *server;
  GSocket           *socket;
  NimfResult        *result;
  GSource           *source;
  GSocketConnection *socket_connection;
  GHashTable        *ims;
};

struct _NimfConnectionClass
{
  GObjectClass parent_class;
};

GType           nimf_connection_get_type         (void) G_GNUC_CONST;
NimfConnection *nimf_connection_new              (void);
guint16         nimf_connection_get_id           (NimfConnection  *connection);
void            nimf_connection_set_engine_by_id (NimfConnection  *connection,
                                                  const gchar     *engine_id);
G_END_DECLS

#endif /* __NIMF_CONNECTION_H__ */
