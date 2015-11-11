/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-private.h
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

#ifndef __DASOM_PRIVATE_H__
#define __DASOM_PRIVATE_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>
#include "dasom-server.h"

G_BEGIN_DECLS

typedef struct _DasomEnginePrivate DasomEnginePrivate;

struct _DasomEnginePrivate
{
  DasomServer  *server;
  gchar        *surrounding_text;
  gint          surrounding_cursor_index;
};

void          dasom_send_message        (GSocket          *socket,
                                         DasomMessageType  type,
                                         gpointer          data,
                                         guint16           data_len,
                                         GDestroyNotify    data_destroy_func);
DasomMessage *dasom_recv_message        (GSocket          *socket);
void          dasom_log_default_handler (const gchar      *log_domain,
                                         GLogLevelFlags    log_level,
                                         const gchar      *message,
                                         gboolean         *debug);

G_END_DECLS

#endif /* __DASOM_PRIVATE_H__ */
