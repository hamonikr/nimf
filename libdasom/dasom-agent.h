/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-agent.h
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

#ifndef __DASOM_AGENT_H__
#define __DASOM_AGENT_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include "dasom-message.h"

G_BEGIN_DECLS

#define DASOM_TYPE_AGENT             (dasom_agent_get_type ())
#define DASOM_AGENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_AGENT, DasomAgent))
#define DASOM_AGENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DASOM_TYPE_AGENT, DasomAgentClass))
#define DASOM_IS_AGENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_AGENT))
#define DASOM_IS_AGENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DASOM_TYPE_AGENT))
#define DASOM_AGENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_AGENT, DasomAgentClass))

typedef struct _DasomAgent      DasomAgent;
typedef struct _DasomAgentClass DasomAgentClass;

struct _DasomAgent
{
  GObject parent_instance;

  /*< private >*/
  GSocketConnection *connection;
  DasomMessage      *reply;
  GSource           *source;
};

struct _DasomAgentClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Signals */
  void (*engine_changed) (DasomAgent  *context,
                          const gchar *str);
  void (*disconnected)   (DasomAgent  *context);
};

GType       dasom_agent_get_type (void) G_GNUC_CONST;
DasomAgent *dasom_agent_new      (void);

G_END_DECLS

#endif /* __DASOM_AGENT_H__ */

