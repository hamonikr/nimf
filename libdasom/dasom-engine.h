/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-engine.h
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

#ifndef __DASOM_ENGINE_H__
#define __DASOM_ENGINE_H__

#include <glib-object.h>
#include "dasom-events.h"
#include "dasom-im.h"
#include "daemon/dasom-context.h"

G_BEGIN_DECLS

#define DASOM_TYPE_ENGINE             (dasom_engine_get_type ())
#define DASOM_ENGINE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_ENGINE, DasomEngine))
#define DASOM_ENGINE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DASOM_TYPE_ENGINE, DasomEngineClass))
#define DASOM_IS_ENGINE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_ENGINE))
#define DASOM_IS_ENGINE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DASOM_TYPE_ENGINE))
#define DASOM_ENGINE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_ENGINE, DasomEngineClass))

typedef struct _DasomEngineInfo DasomEngineInfo;

/* TODO */
struct _DasomEngineInfo
{
  const gchar *engine_name;
};

typedef struct _DasomEngine        DasomEngine;
typedef struct _DasomEngineClass   DasomEngineClass;
typedef struct _DasomEnginePrivate DasomEnginePrivate;

struct _DasomEngine
{
  GObject parent_instance;
  DasomEnginePrivate *priv;
};

struct _DasomEngineClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Virtual functions */
  gboolean (* filter_event)       (DasomEngine      *engine,
                                  const DasomEvent  *event);
  void     (* get_preedit_string) (DasomEngine      *engine,
                                   gchar           **str,
                                   gint             *cursor_pos);
  void     (* reset)              (DasomEngine      *engine);
  void     (* focus_in)           (DasomEngine      *engine);
  void     (* focus_out)          (DasomEngine      *engine);

  const gchar * (* get_name)      (DasomEngine      *engine);
};

GType    dasom_engine_get_type                  (void) G_GNUC_CONST;
gboolean dasom_engine_filter_event              (DasomEngine      *engine,
                                                 const DasomEvent *event);
void     dasom_engine_reset                     (DasomEngine      *engine);
void     dasom_engine_focus_in                  (DasomEngine      *engine);
void     dasom_engine_focus_out                 (DasomEngine      *engine);
void     dasom_engine_get_preedit_string        (DasomEngine      *engine,
                                                 gchar           **str,
                                                 gint             *cursor_pos);

void     dasom_engine_emit_preedit_start        (DasomEngine      *engine);
void     dasom_engine_emit_preedit_changed      (DasomEngine      *engine);
void     dasom_engine_emit_preedit_end          (DasomEngine      *engine);
void     dasom_engine_emit_commit               (DasomEngine      *engine,
                                                 gchar const      *text);

void     dasom_engine_update_candidate_window   (DasomEngine      *engine,
                                                 const gchar     **strv);
void     dasom_engine_show_candidate_window     (DasomEngine      *engine);
void     dasom_engine_hide_candidate_window     (DasomEngine      *engine);
void     dasom_engine_select_previous_candidate (DasomEngine      *engine);
void     dasom_engine_select_next_candidate     (DasomEngine      *engine);

const gchar *dasom_engine_get_name     (DasomEngine      *engine);

G_END_DECLS

#endif /* __DASOM_ENGINE_H__ */
