/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-server-private.h
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

#ifndef __NIMF_SERVER_PRIVATE_H__
#define __NIMF_SERVER_PRIVATE_H__

#include "nimf-server.h"
#include "nimf-service-ic.h"

G_BEGIN_DECLS

struct _NimfServerPrivate
{
  GHashTable    *modules;
  GHashTable    *services;
  GList         *engines;
  NimfServiceIC *last_focused_im;
  const gchar   *last_focused_service;
  GSettings     *settings;
  NimfKey      **hotkeys;
  GHashTable    *shortcuts;
  gboolean       use_singleton;
  GPtrArray     *ics;
  /* facilities */
  NimfCandidatable *candidatable;
  NimfPreeditable  *preeditable;
};

typedef struct {
  GSettings  *settings;
  NimfKey   **to_lang;
  NimfKey   **to_sys;
} NimfShortcut;

G_END_DECLS

NimfEngine *nimf_server_get_default_engine (NimfServer  *server);
NimfEngine *nimf_server_get_next_engine    (NimfServer  *server,
                                            NimfEngine  *engine);
NimfEngine *nimf_server_get_engine_by_id   (NimfServer  *server,
                                            const gchar *engine_id);
gboolean    nimf_server_start              (NimfServer  *server);

#endif /* __NIMF_SERVER_PRIVATE_H__ */

