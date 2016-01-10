/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-connection.h
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

#ifndef __NIMF_CONNECTION_H__
#define __NIMF_CONNECTION_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-engine.h"
#include "nimf-server.h"
#include "nimf-private.h"
#include <X11/Xlib.h>

G_BEGIN_DECLS

#define NIMF_TYPE_CONNECTION             (nimf_connection_get_type ())
#define NIMF_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_CONNECTION, NimfConnection))
#define NIMF_CONNECTION_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_CONNECTION, NimfConnectionClass))
#define NIMF_IS_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_CONNECTION))
#define NIMF_IS_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_CONNECTION))
#define NIMF_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_CONNECTION, NimfConnectionClass))

typedef struct _NimfServer NimfServer;
typedef struct _NimfEngine NimfEngine;
typedef struct _NimfResult NimfResult;

typedef struct _NimfConnection      NimfConnection;
typedef struct _NimfConnectionClass NimfConnectionClass;

struct _NimfConnection
{
  GObject parent_instance;

  NimfConnectionType  type;
  NimfEngine         *engine;
  guint16             id;
  gboolean            use_preedit;
  gboolean            is_english_mode;
  NimfRectangle       cursor_area;
  NimfServer         *server;
  GSocket            *socket;
  NimfResult         *result;
  GSource            *source;
  GSocketConnection  *socket_connection;
  /* XIM */
  guint16             xim_connect_id;
  gint                xim_preedit_length;
  NimfPreeditState    preedit_state;
  gpointer            cb_user_data;
  Window              client_window;
  Window              focus_window;
};

struct _NimfConnectionClass
{
  GObjectClass parent_class;
  /*< public >*/
  /* Signals */
  void     (*preedit_start)        (NimfConnection *connection);
  void     (*preedit_end)          (NimfConnection *connection);
  void     (*preedit_changed)      (NimfConnection *connection);
  void     (*commit)               (NimfConnection *connection,
                                    const gchar    *str);
  gboolean (*retrieve_surrounding) (NimfConnection *connection);
  gboolean (*delete_surrounding)   (NimfConnection *connection,
                                    gint            offset,
                                    gint            n_chars);
  void     (*engine_changed)       (NimfConnection *connection,
                                    const gchar    *str);
};

GType           nimf_connection_get_type                  (void) G_GNUC_CONST;
NimfConnection *nimf_connection_new                       (NimfConnectionType   type,
                                                           NimfEngine          *engine,
                                                           gpointer             cb_user_data);
void           nimf_connection_set_engine_by_id           (NimfConnection    *connection,
                                                           const gchar       *id,
                                                           gboolean           is_english_mode);
guint16         nimf_connection_get_id                    (NimfConnection      *connection);
gboolean        nimf_connection_filter_event              (NimfConnection      *connection,
                                                           NimfEvent           *event);
void            nimf_connection_get_preedit_string        (NimfConnection      *connection,
                                                           gchar              **str,
                                                           gint                *cursor_pos);
void            nimf_connection_reset                     (NimfConnection      *connection);
void            nimf_connection_focus_in                  (NimfConnection      *connection);
void            nimf_connection_focus_out                 (NimfConnection      *connection);
void            nimf_connection_set_surrounding           (NimfConnection      *connection,
                                                           const char          *text,
                                                           gint                 len,
                                                           gint                 cursor_index);
gboolean        nimf_connection_get_surrounding           (NimfConnection      *connection,
                                                           gchar              **text,
                                                           gint                *cursor_index);
void            nimf_connection_xim_set_cursor_location   (NimfConnection      *connection,
                                                           Display             *display);
void            nimf_connection_set_cursor_location       (NimfConnection      *connection,
                                                           const NimfRectangle *area);
void            nimf_connection_set_use_preedit           (NimfConnection      *connection,
                                                           gboolean             use_preedit);
/* signals */
void            nimf_connection_emit_preedit_start        (NimfConnection *connection);
void            nimf_connection_emit_preedit_changed      (NimfConnection *connection,
                                                           const gchar    *preedit_string,
                                                           gint            cursor_pos);
void            nimf_connection_emit_preedit_end          (NimfConnection *connection);
void            nimf_connection_emit_commit               (NimfConnection *connection,
                                                           const gchar    *text);
gboolean        nimf_connection_emit_retrieve_surrounding (NimfConnection *connection);
gboolean        nimf_connection_emit_delete_surrounding   (NimfConnection *connection,
                                                           gint            offset,
                                                           gint            n_chars);
G_END_DECLS

#endif /* __NIMF_CONNECTION_H__ */
