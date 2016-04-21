/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-agent.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2015,2016 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_AGENT_H__
#define __NIMF_AGENT_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-client.h"

G_BEGIN_DECLS

#define NIMF_TYPE_AGENT             (nimf_agent_get_type ())
#define NIMF_AGENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_AGENT, NimfAgent))
#define NIMF_AGENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_AGENT, NimfAgentClass))
#define NIMF_IS_AGENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_AGENT))
#define NIMF_IS_AGENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_AGENT))
#define NIMF_AGENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_AGENT, NimfAgentClass))

typedef struct _NimfAgent      NimfAgent;
typedef struct _NimfAgentClass NimfAgentClass;

struct _NimfAgent
{
  NimfClient parent_instance;
};

struct _NimfAgentClass
{
  NimfClientClass parent_class;
};

GType       nimf_agent_get_type              (void) G_GNUC_CONST;
NimfAgent  *nimf_agent_new                   (void);
gchar     **nimf_agent_get_loaded_engine_ids (NimfAgent   *agent);
void        nimf_agent_set_engine_by_id      (NimfAgent   *agent,
                                              const gchar *id,
                                              gboolean     is_english_mode);
G_END_DECLS

#endif /* __NIMF_AGENT_H__ */

