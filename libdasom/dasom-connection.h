/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-connection.h
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

#ifndef __DASOM_CONNECTION_H__
#define __DASOM_CONNECTION_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>
#include "dasom-engine.h"
#include "dasom-server.h"
#include "dasom-private.h"
#include <X11/Xlib.h>
#include "IMdkit/IMdkit.h"

G_BEGIN_DECLS

#define DASOM_TYPE_CONNECTION             (dasom_connection_get_type ())
#define DASOM_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_CONNECTION, DasomConnection))
#define DASOM_CONNECTION_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), DASOM_TYPE_CONNECTION, DasomConnectionClass))
#define DASOM_IS_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_CONNECTION))
#define DASOM_IS_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), DASOM_TYPE_CONNECTION))
#define DASOM_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_CONNECTION, DasomConnectionClass))

typedef struct _DasomServer DasomServer;
typedef struct _DasomEngine DasomEngine;
typedef struct _DasomResult DasomResult;

typedef struct _DasomConnection      DasomConnection;
typedef struct _DasomConnectionClass DasomConnectionClass;

struct _DasomConnection
{
  GObject parent_instance;

  DasomConnectionType  type;
  DasomEngine         *engine;
  guint16              id;
  gboolean             use_preedit;
  gboolean             is_english_mode;
  DasomRectangle       cursor_area;
  DasomServer         *server;
  GSocket             *socket;
  DasomResult         *result;
  GSource             *source;
  GSocketConnection   *socket_connection;
  /* XIM */
  guint16              xim_connect_id;
  gint                 xim_preedit_length;
  DasomPreeditState    preedit_state;
  gpointer             cb_user_data;
  Window               client_window;
  Window               focus_window;
};

struct _DasomConnectionClass
{
  GObjectClass parent_class;
  /*< public >*/
  /* Signals */
  void     (*preedit_start)        (DasomConnection *connection);
  void     (*preedit_end)          (DasomConnection *connection);
  void     (*preedit_changed)      (DasomConnection *connection);
  void     (*commit)               (DasomConnection *connection,
                                    const gchar     *str);
  gboolean (*retrieve_surrounding) (DasomConnection *connection);
  gboolean (*delete_surrounding)   (DasomConnection *connection,
                                    gint             offset,
                                    gint             n_chars);
  void     (*engine_changed)       (DasomConnection *connection,
                                    const gchar     *str);
};

GType            dasom_connection_get_type                  (void) G_GNUC_CONST;
DasomConnection *dasom_connection_new                       (DasomConnectionType   type,
                                                             DasomEngine          *engine,
                                                             gpointer              cb_user_data);
void             dasom_connection_set_engine                (DasomConnection      *connection,
                                                             DasomEngine          *engine);
guint16          dasom_connection_get_id                    (DasomConnection      *connection);
gboolean         dasom_connection_filter_event              (DasomConnection      *connection,
                                                             DasomEvent           *event);
void             dasom_connection_get_preedit_string        (DasomConnection      *connection,
                                                             gchar               **str,
                                                             gint                 *cursor_pos);
void             dasom_connection_reset                     (DasomConnection      *connection);
void             dasom_connection_focus_in                  (DasomConnection      *connection);
void             dasom_connection_focus_out                 (DasomConnection      *connection);
void             dasom_connection_set_surrounding           (DasomConnection      *connection,
                                                             const char           *text,
                                                             gint                  len,
                                                             gint                  cursor_index);
gboolean         dasom_connection_get_surrounding           (DasomConnection      *connection,
                                                             gchar               **text,
                                                             gint                 *cursor_index);
void             dasom_connection_xim_set_cursor_location   (DasomConnection      *connection,
                                                             XIMS                  xims);
void             dasom_connection_set_cursor_location       (DasomConnection      *connection,
                                                             const DasomRectangle *area);
void             dasom_connection_set_use_preedit           (DasomConnection      *connection,
                                                             gboolean              use_preedit);

void             dasom_connection_emit_preedit_start        (DasomConnection      *connection);
void             dasom_connection_emit_preedit_changed      (DasomConnection      *connection);
void             dasom_connection_emit_preedit_end          (DasomConnection      *connection);
void             dasom_connection_emit_commit               (DasomConnection      *connection,
                                                             const gchar          *text);
gboolean         dasom_connection_emit_retrieve_surrounding (DasomConnection      *connection);
gboolean         dasom_connection_emit_delete_surrounding   (DasomConnection      *connection,
                                                             gint                  offset,
                                                             gint                  n_chars);
G_END_DECLS

#endif /* __DASOM_CONNECTION_H__ */
