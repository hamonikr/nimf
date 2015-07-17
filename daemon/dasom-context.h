/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-context.h
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

#ifndef __DASOM_CONTEXT_H__
#define __DASOM_CONTEXT_H__

#include <glib-object.h>
#include "dasom-engine.h"
#include "dasom-daemon.h"
#include "dasom-message.h"
#include "dasom-types.h"

G_BEGIN_DECLS

#define DASOM_TYPE_CONTEXT             (dasom_context_get_type ())
#define DASOM_CONTEXT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_CONTEXT, DasomContext))
#define DASOM_CONTEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DASOM_TYPE_CONTEXT, DasomContextClass))
#define DASOM_IS_CONTEXT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_CONTEXT))
#define DASOM_IS_CONTEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DASOM_TYPE_CONTEXT))
#define DASOM_CONTEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_CONTEXT, DasomContextClass))

typedef struct _DasomDaemon       DasomDaemon;

typedef struct _DasomContext      DasomContext;
typedef struct _DasomContextClass DasomContextClass;

struct _DasomContext
{
  GObject parent_instance;

  DasomConnectionType  type;
  DasomEngine         *engine;
  guint16              id;
  guint16              xim_connect_id;

  DasomDaemon         *daemon;
  GSocket             *socket;
  DasomMessage        *reply;
  GSource             *source;
  GSocketConnection   *connection;
};

struct _DasomContextClass
{
  GObjectClass parent_class;
  /*< public >*/
  /* Signals */
  void     (*preedit_start)        (DasomContext *context);
  void     (*preedit_end)          (DasomContext *context);
  void     (*preedit_changed)      (DasomContext *context);
  void     (*commit)               (DasomContext *context,
                                    const gchar  *str);
  gboolean (*retrieve_surrounding) (DasomContext *context);
  gboolean (*delete_surrounding)   (DasomContext *context,
                                    gint          offset,
                                    gint          n_chars);
  void     (*engine_changed)       (DasomContext *context,
                                    const gchar  *str);
};

GType         dasom_context_get_type           (void) G_GNUC_CONST;
DasomContext *dasom_context_new                (DasomConnectionType  type,
                                                DasomEngine         *engine);
void          dasom_context_set_engine         (DasomContext        *context,
                                                DasomEngine         *engine);
guint16       dasom_context_get_id             (DasomContext        *context);
gboolean      dasom_context_filter_event       (DasomContext        *context,
                                                const DasomEvent    *event);
void          dasom_context_get_preedit_string (DasomContext        *context,
                                                gchar              **str,
                                                gint                *cursor_pos);
void          dasom_context_reset              (DasomContext        *context);
void          dasom_context_focus_in           (DasomContext        *context);
void          dasom_context_focus_out          (DasomContext        *context);
void          dasom_context_set_surrounding    (DasomContext        *context,
                                                const char          *text,
                                                gint                 len,
                                                gint                 cursor_index);
gboolean      dasom_context_get_surrounding    (DasomContext        *context,
                                                gchar              **text,
                                                gint                *cursor_index);
void          dasom_context_set_cursor_location (DasomContext         *context,
                                                 const DasomRectangle *area);

G_END_DECLS

#endif /* __DASOM_CONTEXT_H__ */
