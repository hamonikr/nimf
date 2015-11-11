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

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>
#include "dasom-events.h"
#include "dasom-types.h"
#include "dasom-connection.h"

G_BEGIN_DECLS

#define DASOM_TYPE_ENGINE             (dasom_engine_get_type ())
#define DASOM_ENGINE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_ENGINE, DasomEngine))
#define DASOM_ENGINE_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), DASOM_TYPE_ENGINE, DasomEngineClass))
#define DASOM_IS_ENGINE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_ENGINE))
#define DASOM_IS_ENGINE_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), DASOM_TYPE_ENGINE))
#define DASOM_ENGINE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_ENGINE, DasomEngineClass))

typedef struct _DasomEngineInfo DasomEngineInfo;

/* TODO */
struct _DasomEngineInfo
{
  const gchar *engine_name;
};

typedef struct _DasomConnection    DasomConnection;

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
                                   DasomConnection  *connection,
                                   DasomEvent       *event);
  void     (* get_preedit_string) (DasomEngine      *engine,
                                   gchar           **str,
                                   gint             *cursor_pos);
  void     (* reset)              (DasomEngine      *engine,
                                   DasomConnection  *target);
  void     (* focus_in)           (DasomEngine      *engine);
  void     (* focus_out)          (DasomEngine      *engine,
                                   DasomConnection  *target);
  void     (* set_surrounding)    (DasomEngine      *engine,
                                   const char       *text,
                                   gint              len,
                                   gint              cursor_index);
  gboolean (* get_surrounding)    (DasomEngine      *engine,
                                   DasomConnection  *target,
                                   gchar           **text,
                                   gint             *cursor_index);
  void     (* set_cursor_location)(DasomEngine      *engine,
                                   const DasomRectangle *area);
  void     (* set_english_mode)   (DasomEngine      *engine,
                                   gboolean          is_english_mode);
  gboolean (* get_english_mode)   (DasomEngine      *engine);

  void     (* candidate_clicked)  (DasomEngine      *engine,
                                   DasomConnection  *target,
                                   gchar            *text);

  const gchar * (* get_name)      (DasomEngine      *engine);
};

GType    dasom_engine_get_type                  (void) G_GNUC_CONST;
gboolean dasom_engine_filter_event              (DasomEngine      *engine,
                                                 DasomConnection  *target,
                                                 DasomEvent       *event);
void     dasom_engine_reset                     (DasomEngine      *engine,
                                                 DasomConnection  *target);
void     dasom_engine_focus_in                  (DasomEngine      *engine);
void     dasom_engine_focus_out                 (DasomEngine      *engine,
                                                 DasomConnection  *target);
void     dasom_engine_get_preedit_string        (DasomEngine      *engine,
                                                 gchar           **str,
                                                 gint             *cursor_pos);
void     dasom_engine_set_surrounding           (DasomEngine      *engine,
                                                 const char       *text,
                                                 gint              len,
                                                 gint              cursor_index);
gboolean dasom_engine_get_surrounding           (DasomEngine      *engine,
                                                 DasomConnection  *target,
                                                 gchar           **text,
                                                 gint             *cursor_index);
void     dasom_engine_set_cursor_location       (DasomEngine      *engine,
                                                 const DasomRectangle *area);
void     dasom_engine_set_english_mode          (DasomEngine      *engine,
                                                 gboolean          is_english_mode);
gboolean dasom_engine_get_english_mode          (DasomEngine      *engine);

void     dasom_engine_emit_preedit_start        (DasomEngine      *engine,
                                                 DasomConnection  *target);
void     dasom_engine_emit_preedit_changed      (DasomEngine      *engine,
                                                 DasomConnection  *target);
void     dasom_engine_emit_preedit_end          (DasomEngine      *engine,
                                                 DasomConnection  *target);
void     dasom_engine_emit_commit               (DasomEngine      *engine,
                                                 DasomConnection  *target,
                                                 gchar const      *text);
gboolean dasom_engine_emit_retrieve_surrounding (DasomEngine      *engine,
                                                 DasomConnection  *target);
gboolean dasom_engine_emit_delete_surrounding   (DasomEngine      *engine,
                                                 DasomConnection  *target,
                                                 gint              offset,
                                                 gint              n_chars);
void     dasom_engine_emit_engine_changed       (DasomEngine      *engine,
                                                 DasomConnection  *target);
/* candidate */
void     dasom_engine_update_candidate_window         (DasomEngine  *engine,
                                                       const gchar **strv);
void     dasom_engine_show_candidate_window           (DasomEngine  *engine,
                                                       DasomConnection *target);
void     dasom_engine_hide_candidate_window           (DasomEngine  *engine);
void     dasom_engine_select_previous_candidate_item  (DasomEngine  *engine);
void     dasom_engine_select_next_candidate_item      (DasomEngine  *engine);
void     dasom_engine_select_page_up_candidate_item   (DasomEngine  *engine);
void     dasom_engine_select_page_down_candidate_item (DasomEngine  *engine);
gchar   *dasom_engine_get_selected_candidate_text     (DasomEngine  *engine);
const gchar *dasom_engine_get_name                    (DasomEngine  *engine);

G_END_DECLS

#endif /* __DASOM_ENGINE_H__ */
