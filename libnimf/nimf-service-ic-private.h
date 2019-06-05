/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-service-ic-private.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2019 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_SERVICE_IC_PRIVATE_H__
#define __NIMF_SERVICE_IC_PRIVATE_H__

#include "nimf-service-ic.h"

G_BEGIN_DECLS

extern void
nimf_service_ic_load_engine (NimfServiceIC *ic,
                             const gchar   *engine_id,
                             NimfServer    *server);
extern void
nimf_service_ic_unload_engine (NimfServiceIC *ic,
                               const gchar   *engine_id,
                               NimfEngine    *signleton_engine_to_be_deleted,
                               NimfServer    *server);

G_END_DECLS

#endif /* __NIMF_SERVICE_IC_PRIVATE_H__ */
