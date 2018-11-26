/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-private.h
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

#ifndef __NIMF_PRIVATE_H__
#define __NIMF_PRIVATE_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-message.h"
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _NimfResult NimfResult;

struct _NimfResult
{
  gboolean     is_dispatched;
  NimfMessage *reply;
};

void         nimf_send_message           (GSocket         *socket,
                                          guint16          im_id,
                                          NimfMessageType  type,
                                          gpointer         data,
                                          guint16          data_len,
                                          GDestroyNotify   data_destroy_func);
NimfMessage *nimf_recv_message           (GSocket         *socket);
void         nimf_log_default_handler    (const gchar     *log_domain,
                                          GLogLevelFlags   log_level,
                                          const gchar     *message,
                                          gboolean        *debug);
void         nimf_result_iteration_until (NimfResult      *result,
                                          GMainContext    *main_context,
                                          guint16          icid,
                                          NimfMessageType  type);
gchar       *nimf_get_socket_path (void);
gchar       *nimf_get_lock_path   (void);
gchar       *nimf_get_nimf_path   (void);

G_END_DECLS

#endif /* __NIMF_PRIVATE_H__ */
