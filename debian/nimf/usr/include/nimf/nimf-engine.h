/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-engine.h
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

#ifndef __NIMF_ENGINE_H__
#define __NIMF_ENGINE_H__

#include <glib-object.h>
#include "nimf-events.h"
#include "nimf-service-ic.h"
#include "nimf-candidatable.h"

G_BEGIN_DECLS

#define NIMF_TYPE_ENGINE            (nimf_engine_get_type ())
#define NIMF_ENGINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_ENGINE, NimfEngine))
#define NIMF_ENGINE_CLASS(class)    (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_ENGINE, NimfEngineClass))
#define NIMF_IS_ENGINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_ENGINE))
#define NIMF_ENGINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_ENGINE, NimfEngineClass))

#ifndef __GTK_DOC_IGNORE__
typedef struct _NimfServiceIC    NimfServiceIC;
typedef struct _NimfCandidatable NimfCandidatable;
#endif
typedef struct _NimfEngine        NimfEngine;
typedef struct _NimfEngineClass   NimfEngineClass;
typedef struct _NimfEnginePrivate NimfEnginePrivate;

struct _NimfEngine
{
  GObject parent_instance;
  NimfEnginePrivate *priv;
};

/**
 * NimfEngineClass:
 * @filter_event: If the language engine consumes the event, returns %TRUE.
 * @reset: Resets the language engine.
 * @focus_in: Called via nimf_im_focus_in() when the caller has gained focus.
 * @focus_out: Called via nimf_im_focus_out() when the caller has lost focus.
 * @set_method: Sets the method in the language engine.
 * @candidate_page_up: handler when candidate page up
 * @candidate_page_down: handler when candidate page down
 * @candidate_clicked: handler when candidate clicked
 * @candidate_scrolled: handler when candidate scrolled
 * @get_id: Gets the language engine id.
 * @get_icon_name: Gets the language engine name.
 */
struct _NimfEngineClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Virtual functions */
  gboolean (* filter_event)       (NimfEngine          *engine,
                                   NimfServiceIC       *ic,
                                   NimfEvent           *event);
  void     (* reset)              (NimfEngine          *engine,
                                   NimfServiceIC       *ic);
  void     (* focus_in)           (NimfEngine          *engine,
                                   NimfServiceIC       *ic);
  void     (* focus_out)          (NimfEngine          *engine,
                                   NimfServiceIC       *ic);
  void     (* set_method)         (NimfEngine          *engine,
                                   const gchar         *method_id);
  /* candidate */
  gboolean (* candidate_page_up)   (NimfEngine         *engine,
                                    NimfServiceIC      *ic);
  gboolean (* candidate_page_down) (NimfEngine         *engine,
                                    NimfServiceIC      *ic);
  void     (* candidate_clicked)   (NimfEngine         *engine,
                                    NimfServiceIC      *ic,
                                    gchar              *text,
                                    gint                index);
  void     (* candidate_scrolled)  (NimfEngine         *engine,
                                    NimfServiceIC      *ic,
                                    gdouble             value);
  /* info */
  const gchar * (* get_id)        (NimfEngine          *engine);
  const gchar * (* get_icon_name) (NimfEngine          *engine);
};

GType    nimf_engine_get_type                  (void) G_GNUC_CONST;
gboolean nimf_engine_filter_event              (NimfEngine          *engine,
                                                NimfServiceIC       *ic,
                                                NimfEvent           *event);
void     nimf_engine_reset                     (NimfEngine          *engine,
                                                NimfServiceIC       *ic);
void     nimf_engine_focus_in                  (NimfEngine          *engine,
                                                NimfServiceIC       *ic);
void     nimf_engine_focus_out                 (NimfEngine          *engine,
                                                NimfServiceIC       *ic);
void     nimf_engine_set_surrounding           (NimfEngine          *engine,
                                                const char          *text,
                                                gint                 len,
                                                gint                 cursor_index);
gboolean nimf_engine_get_surrounding           (NimfEngine          *engine,
                                                NimfServiceIC       *ic,
                                                gchar              **text,
                                                gint                *cursor_index);
void     nimf_engine_status_changed            (NimfEngine          *engine);
void     nimf_engine_set_method                (NimfEngine          *engine,
                                                const gchar         *method_id);
/* signals */
void     nimf_engine_emit_preedit_start        (NimfEngine       *engine,
                                                NimfServiceIC    *ic);
void     nimf_engine_emit_preedit_changed      (NimfEngine       *engine,
                                                NimfServiceIC    *ic,
                                                const gchar      *preedit_string,
                                                NimfPreeditAttr **attrs,
                                                gint              cursor_pos);
void     nimf_engine_emit_preedit_end          (NimfEngine       *engine,
                                                NimfServiceIC    *ic);
void     nimf_engine_emit_commit               (NimfEngine       *engine,
                                                NimfServiceIC    *ic,
                                                gchar const      *text);
gboolean nimf_engine_emit_retrieve_surrounding (NimfEngine       *engine,
                                                NimfServiceIC    *ic);
gboolean nimf_engine_emit_delete_surrounding   (NimfEngine       *engine,
                                                NimfServiceIC    *ic,
                                                gint              offset,
                                                gint              n_chars);
void     nimf_engine_emit_beep                 (NimfEngine       *engine,
                                                NimfServiceIC    *ic);
/* info */
const gchar      *nimf_engine_get_id           (NimfEngine *engine);
const gchar      *nimf_engine_get_icon_name    (NimfEngine *engine);
/* candidate */
NimfCandidatable *nimf_engine_get_candidatable (NimfEngine *engine);

G_END_DECLS

#endif /* __NIMF_ENGINE_H__ */
