/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-server.h
 * This file is part of Dasom.
 *
 * Copyright (C) 2015 Hodong Kim <hodong@cogno.org>
 *
 * Dasom is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Dasom is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program;  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DASOM_SERVER_H__
#define __DASOM_SERVER_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>
#include "dasom-module-manager.h"
#include <gio/gio.h>
#include "dasom-types.h"
#include "dasom-candidate.h"
#include "dasom-engine.h"

G_BEGIN_DECLS

#define DASOM_TYPE_SERVER             (dasom_server_get_type ())
#define DASOM_SERVER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_SERVER, DasomServer))
#define DASOM_SERVER_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), DASOM_TYPE_SERVER, DasomServerClass))
#define DASOM_IS_SERVER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_SERVER))
#define DASOM_IS_SERVER_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), DASOM_TYPE_SERVER))
#define DASOM_SERVER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_SERVER, DasomServerClass))

typedef struct _DasomEngine      DasomEngine;
typedef struct _DasomCandidate   DasomCandidate;

typedef struct _DasomServer      DasomServer;
typedef struct _DasomServerClass DasomServerClass;

struct _DasomServer
{
  GObject parent_instance;

  GMainContext        *main_context;
  DasomModuleManager  *module_manager;
  GList               *instances;
  GSocketListener     *listener;
  GHashTable          *connections;
  GList               *agents_list;
  DasomKey           **hotkeys;
  DasomCandidate      *candidate;
  GSource             *xevent_source;
  guint16              next_id;

  gchar     *address;
  gboolean   active;
  gboolean   is_using_listener;
  gulong     run_signal_handler_id;
};

struct _DasomServerClass
{
  GObjectClass parent_class;
};

GType        dasom_server_get_type           (void) G_GNUC_CONST;

DasomServer *dasom_server_new                (const gchar  *address,
                                              GError      **error);
void         dasom_server_start              (DasomServer  *server);
void         dasom_server_stop               (DasomServer  *server);
DasomEngine *dasom_server_get_default_engine (DasomServer  *server);
DasomEngine *dasom_server_get_next_instance  (DasomServer  *server,
                                              DasomEngine  *engine);
DasomEngine *dasom_server_get_instance       (DasomServer  *server,
                                              const gchar  *module_name);

G_END_DECLS

#endif /* __DASOM_SERVER_H__ */

