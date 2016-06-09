/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-context.h
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

#ifndef __NIMF_CONTEXT_H__
#define __NIMF_CONTEXT_H__

#include <glib.h>
#include "nimf-engine.h"
#include "nimf-connection.h"
#include "nimf-server.h"
#include <X11/Xlib.h>

G_BEGIN_DECLS

typedef struct _NimfEngine     NimfEngine;
typedef struct _NimfConnection NimfConnection;

typedef struct _NimfContext NimfContext;

struct _NimfContext
{
  NimfContextType  type;
  NimfEngine      *engine;
  guint16          icid;
  NimfConnection  *connection;
  NimfServer      *server;
  gboolean         use_preedit;
  NimfRectangle    cursor_area;
  /* XIM */
  guint16          xim_connect_id;
  gint             xim_preedit_length;
  NimfPreeditState preedit_state;
  gpointer         cb_user_data;
  Window           client_window;
  Window           focus_window;
};

NimfContext *nimf_context_new  (NimfContextType  type,
                                NimfConnection  *connection,
                                NimfServer      *server,
                                gpointer         cb_user_data);
void         nimf_context_free (NimfContext     *context);

void         nimf_context_focus_in           (NimfContext  *context);
void         nimf_context_focus_out          (NimfContext  *context);
gboolean     nimf_context_filter_event       (NimfContext  *context,
                                              NimfEvent    *event);
void         nimf_context_get_preedit_string (NimfContext  *context,
                                              gchar       **str,
                                              gint         *cursor_pos);
void         nimf_context_set_surrounding         (NimfContext         *context,
                                                   const char          *text,
                                                   gint                 len,
                                                   gint                 cursor_index);
gboolean     nimf_context_get_surrounding         (NimfContext         *context,
                                                   gchar              **text,
                                                   gint                *cursor_index);
void         nimf_context_set_use_preedit         (NimfContext         *context,
                                                   gboolean             use_preedit);
void         nimf_context_set_cursor_location     (NimfContext         *context,
                                                   const NimfRectangle *area);
void         nimf_context_xim_set_cursor_location (NimfContext         *context,
                                                   Display             *display);
void         nimf_context_reset              (NimfContext  *context);
void         nimf_context_set_engine_by_id   (NimfContext  *context,
                                              const gchar  *engine_id);
/* signals */
void     nimf_context_emit_preedit_start        (NimfContext *context);
void     nimf_context_emit_preedit_changed      (NimfContext *context,
                                                 const gchar *preedit_string,
                                                 gint         cursor_pos);
void     nimf_context_emit_preedit_end          (NimfContext *context);
void     nimf_context_emit_commit               (NimfContext *context,
                                                 const gchar *text);
gboolean nimf_context_emit_retrieve_surrounding (NimfContext *context);
gboolean nimf_context_emit_delete_surrounding   (NimfContext *context,
                                                 gint         offset,
                                                 gint         n_chars);
void     nimf_context_emit_engine_changed       (NimfContext *context,
                                                 const gchar *name);
G_END_DECLS

#endif /* __NIMF_CONTEXT_H__ */
