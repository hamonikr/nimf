/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-service-ic.h
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

#ifndef __NIMF_SERVICE_IC_H__
#define __NIMF_SERVICE_IC_H__

#include <glib.h>
#include "nimf-types.h"
#include "nimf-events.h"
#include "nimf-engine.h"

G_BEGIN_DECLS

#define NIMF_TYPE_SERVICE_IC             (nimf_service_ic_get_type ())
#define NIMF_SERVICE_IC(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_SERVICE_IC, NimfServiceIC))
#define NIMF_SERVICE_IC_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_SERVICE_IC, NimfServiceICClass))
#define NIMF_IS_SERVICE_IC(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_SERVICE_IC))
#define NIMF_SERVICE_IC_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_SERVICE_IC, NimfServiceICClass))

#ifndef __GTK_DOC_IGNORE__
typedef struct _NimfEngine NimfEngine;
#endif
typedef struct _NimfServiceIC        NimfServiceIC;
typedef struct _NimfServiceICClass   NimfServiceICClass;
typedef struct _NimfServiceICPrivate NimfServiceICPrivate;

struct _NimfServiceICClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Virtual functions */
  const gchar * (* get_service_id) (NimfServiceIC *ic);

  void     (* emit_commit)               (NimfServiceIC    *ic,
                                          const gchar      *text);
  void     (* emit_preedit_start)        (NimfServiceIC    *ic);
  void     (* emit_preedit_changed)      (NimfServiceIC    *ic,
                                          const gchar      *preedit_string,
                                          NimfPreeditAttr **attrs,
                                          gint              cursor_pos);
  void     (* emit_preedit_end)          (NimfServiceIC    *ic);
  gboolean (* emit_retrieve_surrounding) (NimfServiceIC    *ic);
  gboolean (* emit_delete_surrounding)   (NimfServiceIC    *ic,
                                          gint              offset,
                                          gint              n_chars);
  void     (* emit_beep)                 (NimfServiceIC    *ic);
};

struct _NimfServiceIC
{
  GObject parent_instance;
  NimfServiceICPrivate *priv;
};

GType        nimf_service_ic_get_type           (void) G_GNUC_CONST;
void         nimf_service_ic_focus_in           (NimfServiceIC *ic);
void         nimf_service_ic_focus_out          (NimfServiceIC *ic);
gboolean     nimf_service_ic_filter_event       (NimfServiceIC *ic,
                                                 NimfEvent     *event);
void         nimf_service_ic_set_surrounding     (NimfServiceIC  *ic,
                                                  const char     *text,
                                                  gint            len,
                                                  gint            cursor_index);
void         nimf_service_ic_set_use_preedit     (NimfServiceIC  *ic,
                                                  gboolean        use_preedit);
gboolean     nimf_service_ic_get_use_preedit     (NimfServiceIC  *ic);
void         nimf_service_ic_set_cursor_location (NimfServiceIC  *ic,
                                                  const NimfRectangle *area);
const NimfRectangle *
             nimf_service_ic_get_cursor_location (NimfServiceIC  *ic);
void         nimf_service_ic_reset               (NimfServiceIC  *ic);
void         nimf_service_ic_change_engine_by_id (NimfServiceIC  *ic,
                                                  const gchar    *engine_id);
void         nimf_service_ic_change_engine       (NimfServiceIC  *ic,
                                                  const gchar    *engine_id,
                                                  const gchar    *method_id);
void         nimf_service_ic_engine_changed      (NimfServiceIC  *ic,
                                                  const gchar    *engine_id,
                                                  const gchar    *name);
NimfEngine  *nimf_service_ic_get_engine          (NimfServiceIC  *ic);
const gchar *nimf_service_ic_get_service_id      (NimfServiceIC  *ic);
/* signals */
void     nimf_service_ic_emit_preedit_start        (NimfServiceIC    *ic);
void     nimf_service_ic_emit_preedit_changed      (NimfServiceIC    *ic,
                                                    const gchar      *preedit_string,
                                                    NimfPreeditAttr **attrs,
                                                    gint              cursor_pos);
void     nimf_service_ic_emit_preedit_end          (NimfServiceIC    *ic);
void     nimf_service_ic_emit_commit               (NimfServiceIC    *ic,
                                                    const gchar      *text);
gboolean nimf_service_ic_emit_retrieve_surrounding (NimfServiceIC    *ic);
gboolean nimf_service_ic_emit_delete_surrounding   (NimfServiceIC    *ic,
                                                    gint              offset,
                                                    gint              n_chars);
void     nimf_service_ic_emit_beep                 (NimfServiceIC    *ic);

G_END_DECLS

#endif /* __NIMF_SERVICE_IC_H__ */
