/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-server.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2015 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_SERVER_H__
#define __NIMF_SERVER_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-module-manager.h"
#include <gio/gio.h>
#include "nimf-types.h"
#include "nimf-candidate.h"
#include "nimf-engine.h"

G_BEGIN_DECLS

#define NIMF_TYPE_SERVER             (nimf_server_get_type ())
#define NIMF_SERVER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_SERVER, NimfServer))
#define NIMF_SERVER_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_SERVER, NimfServerClass))
#define NIMF_IS_SERVER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_SERVER))
#define NIMF_IS_SERVER_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_SERVER))
#define NIMF_SERVER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_SERVER, NimfServerClass))

typedef struct _NimfEngine      NimfEngine;
typedef struct _NimfCandidate   NimfCandidate;

typedef struct _NimfServer      NimfServer;
typedef struct _NimfServerClass NimfServerClass;

struct _NimfServer
{
  GObject parent_instance;

  GMainContext       *main_context;
  NimfModuleManager  *module_manager;
  GList              *instances;
  GSocketListener    *listener;
  GHashTable         *connections;
  GList              *agents_list;
  NimfKey           **hotkeys;
  NimfCandidate      *candidate;
  GSource            *xevent_source;
  guint16             next_id;

  gchar     *address;
  gboolean   active;
  gboolean   is_using_listener;
  gulong     run_signal_handler_id;
};

struct _NimfServerClass
{
  GObjectClass parent_class;
};

GType       nimf_server_get_type           (void) G_GNUC_CONST;

NimfServer *nimf_server_new                (const gchar  *address,
                                            GError      **error);
void        nimf_server_start              (NimfServer   *server);
void        nimf_server_stop               (NimfServer   *server);
NimfEngine *nimf_server_get_default_engine (NimfServer   *server);
NimfEngine *nimf_server_get_next_instance  (NimfServer   *server,
                                            NimfEngine   *engine);
NimfEngine *nimf_server_get_instance       (NimfServer   *server,
                                            const gchar  *module_name);
G_END_DECLS

#endif /* __NIMF_SERVER_H__ */

