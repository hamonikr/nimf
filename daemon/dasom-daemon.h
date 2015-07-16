/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-daemon.h
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

#ifndef __DASOM_DAEMON_H__
#define __DASOM_DAEMON_H__

#include <glib-object.h>
#include "dasom-module-manager.h"
#include "dasom-candidate.h"
#include "dasom-engine.h"
#include "dasom-context.h"

G_BEGIN_DECLS

#define DASOM_TYPE_DAEMON             (dasom_daemon_get_type ())
#define DASOM_DAEMON(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_DAEMON, DasomDaemon))
#define DASOM_DAEMON_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DASOM_TYPE_DAEMON, DasomDaemonClass))
#define DASOM_IS_DAEMON(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_DAEMON))
#define DASOM_IS_DAEMON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DASOM_TYPE_DAEMON))
#define DASOM_DAEMON_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_DAEMON, DasomDaemonClass))

typedef struct _DasomEngine  DasomEngine;
typedef struct _DasomContext DasomContext;

typedef struct _DasomDaemon      DasomDaemon;
typedef struct _DasomDaemonClass DasomDaemonClass;

struct _DasomDaemon
{
  GObject parent_instance;

  GMainLoop           *loop;
  int                  status;
  DasomModuleManager  *module_manager;
  GList               *instances;
  GSocketService      *service;
  GHashTable          *contexts;
  GList               *agents_list;
  gchar              **hotkey_names;
  DasomCandidate      *candidate;
  DasomContext        *target;
};

struct _DasomDaemonClass
{
  GObjectClass parent_class;
};

GType        dasom_daemon_get_type           (void) G_GNUC_CONST;
DasomEngine *dasom_daemon_get_default_engine (DasomDaemon *daemon);
DasomEngine *dasom_daemon_get_next_instance  (DasomDaemon *daemon,
                                              DasomEngine *engine);
DasomEngine *dasom_daemon_get_instance       (DasomDaemon *daemon,
                                              const gchar *module_name);

G_END_DECLS

#endif /* __DASOM_DAEMON_H__ */

