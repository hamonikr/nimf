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
#define NIMF_IS_SERVICE_IC_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_SERVICE_IC))
#define NIMF_SERVICE_IC_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_SERVICE_IC, NimfServiceICClass))

typedef struct _NimfEngine NimfEngine;

typedef struct _NimfServiceIC        NimfServiceIC;
typedef struct _NimfServiceICClass   NimfServiceICClass;
typedef struct _NimfServiceICPrivate NimfServiceICPrivate;

struct _NimfServiceICClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Virtual functions */
  const gchar * (* get_service_id) (NimfServiceIC *im);

  void     (* emit_commit)               (NimfServiceIC    *im,
                                          const gchar      *text);
  void     (* emit_preedit_start)        (NimfServiceIC    *im);
  void     (* emit_preedit_changed)      (NimfServiceIC    *im,
                                          const gchar      *preedit_string,
                                          NimfPreeditAttr **attrs,
                                          gint              cursor_pos);
  void     (* emit_preedit_end)          (NimfServiceIC    *im);
  gboolean (* emit_retrieve_surrounding) (NimfServiceIC    *im);
  gboolean (* emit_delete_surrounding)   (NimfServiceIC    *im,
                                          gint              offset,
                                          gint              n_chars);
  void     (* emit_beep)                 (NimfServiceIC    *im);
};

struct _NimfServiceIC
{
  GObject parent_instance;
  NimfServiceICPrivate *priv;
};

GType        nimf_service_ic_get_type           (void) G_GNUC_CONST;
void         nimf_service_ic_focus_in           (NimfServiceIC *im);
void         nimf_service_ic_focus_out          (NimfServiceIC *im);
gboolean     nimf_service_ic_filter_event       (NimfServiceIC *im,
                                                 NimfEvent     *event);
void         nimf_service_ic_set_surrounding     (NimfServiceIC  *im,
                                                  const char     *text,
                                                  gint            len,
                                                  gint            cursor_index);
void         nimf_service_ic_set_use_preedit     (NimfServiceIC  *im,
                                                  gboolean        use_preedit);
gboolean     nimf_service_ic_get_use_preedit     (NimfServiceIC  *im);
void         nimf_service_ic_set_cursor_location (NimfServiceIC  *im,
                                                  const NimfRectangle *area);
const NimfRectangle *
             nimf_service_ic_get_cursor_location (NimfServiceIC  *im);
void         nimf_service_ic_reset               (NimfServiceIC  *im);
void         nimf_service_ic_set_engine_by_id    (NimfServiceIC  *im,
                                                  const gchar    *engine_id);
void         nimf_service_ic_set_engine          (NimfServiceIC  *im,
                                                  const gchar    *engine_id,
                                                  const gchar    *method_id);
void         nimf_service_ic_engine_changed      (NimfServiceIC  *im,
                                                  const gchar    *engine_id,
                                                  const gchar    *name);
NimfEngine  *nimf_service_ic_get_engine          (NimfServiceIC  *im);
const gchar *nimf_service_ic_get_service_id      (NimfServiceIC  *im);
/* signals */
void     nimf_service_ic_emit_preedit_start        (NimfServiceIC    *im);
void     nimf_service_ic_emit_preedit_changed      (NimfServiceIC    *im,
                                                    const gchar      *preedit_string,
                                                    NimfPreeditAttr **attrs,
                                                    gint              cursor_pos);
void     nimf_service_ic_emit_preedit_end          (NimfServiceIC    *im);
void     nimf_service_ic_emit_commit               (NimfServiceIC    *im,
                                                    const gchar      *text);
gboolean nimf_service_ic_emit_retrieve_surrounding (NimfServiceIC    *im);
gboolean nimf_service_ic_emit_delete_surrounding   (NimfServiceIC    *im,
                                                    gint              offset,
                                                    gint              n_chars);
void     nimf_service_ic_emit_beep                 (NimfServiceIC    *im);

G_END_DECLS

#endif /* __NIMF_SERVICE_IC_H__ */
