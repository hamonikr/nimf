/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-service-im.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2018 Hodong Kim <cogniti@gmail.com>
 *
 * # 법적 고지
 *
 * Nimf 소프트웨어는 대한민국 저작권법과 국제 조약의 보호를 받습니다.
 * Nimf 개발자는 대한민국 법률의 보호를 받습니다.
 * 커뮤니티의 위력을 이용하여 개발자의 시간과 노동력을 약탈하려는 행위를 금하시기 바랍니다.
 *
 * * 커뮤니티 게시판에 개발자를 욕(비난)하거나
 * * 욕보이는(음해하는) 글을 작성하거나
 * * 허위 사실을 공표하거나
 * * 명예를 훼손하는
 *
 * 등의 행위는 정보통신망 이용촉진 및 정보보호 등에 관한 법률의 제재를 받습니다.
 *
 * # 면책 조항
 *
 * Nimf 는 무료로 배포되는 오픈소스 소프트웨어입니다.
 * Nimf 개발자는 개발 및 유지보수에 대해 어떠한 의무도 없고 어떠한 책임도 없습니다.
 * 어떠한 경우에도 보증하지 않습니다. 도덕적 보증 책임도 없고, 도의적 보증 책임도 없습니다.
 * Nimf 개발자는 리브레오피스, 이클립스 등 귀하가 사용하시는 소프트웨어의 버그를 해결해야 할 의무가 없습니다.
 * Nimf 개발자는 귀하가 사용하시는 배포판에 대해 기술 지원을 해드려야 할 의무가 없습니다.
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

#ifndef __NIMF_SERVICE_IM_H__
#define __NIMF_SERVICE_IM_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib.h>
#include "nimf-engine.h"
#include "nimf-connection.h"
#include "nimf-server.h"
#include "nimf-enum-types.h"

G_BEGIN_DECLS

#define NIMF_TYPE_SERVICE_IM             (nimf_service_im_get_type ())
#define NIMF_SERVICE_IM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_SERVICE_IM, NimfServiceIM))
#define NIMF_SERVICE_IM_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_SERVICE_IM, NimfServiceIMClass))
#define NIMF_IS_SERVICE_IM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_SERVICE_IM))
#define NIMF_IS_SERVICE_IM_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_SERVICE_IM))
#define NIMF_SERVICE_IM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_SERVICE_IM, NimfServiceIMClass))

typedef struct _NimfEngine     NimfEngine;
typedef struct _NimfConnection NimfConnection;

typedef struct _NimfServiceIM      NimfServiceIM;
typedef struct _NimfServiceIMClass NimfServiceIMClass;

struct _NimfServiceIMClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Virtual functions */
  void     (* emit_commit)               (NimfServiceIM    *im,
                                          const gchar      *text);
  void     (* emit_preedit_start)        (NimfServiceIM    *im);
  void     (* emit_preedit_changed)      (NimfServiceIM    *im,
                                          const gchar      *preedit_string,
                                          NimfPreeditAttr **attrs,
                                          gint              cursor_pos);
  void     (* emit_preedit_end)          (NimfServiceIM    *im);
  gboolean (* emit_retrieve_surrounding) (NimfServiceIM    *im);
  gboolean (* emit_delete_surrounding)   (NimfServiceIM    *im,
                                          gint              offset,
                                          gint              n_chars);
  void     (* emit_beep)                 (NimfServiceIM    *im);
};

struct _NimfServiceIM
{
  GObject parent_instance;

   /*< private >*/
  GObjectClass parent_class;

  NimfEngine       *engine;
  guint16           icid;
  NimfServer       *server; /* prop */
  gboolean          use_preedit;
  NimfRectangle     cursor_area;
  GList            *engines;
  /* preedit */
  NimfPreeditState  preedit_state;
  gchar            *preedit_string;
  NimfPreeditAttr **preedit_attrs;
  gint              preedit_cursor_pos;
};

GType nimf_service_im_get_type    (void) G_GNUC_CONST;
void         nimf_service_im_focus_in           (NimfServiceIM *im);
void         nimf_service_im_focus_out          (NimfServiceIM *im);
gboolean     nimf_service_im_filter_event       (NimfServiceIM *im,
                                                 NimfEvent     *event);
void         nimf_service_im_set_surrounding     (NimfServiceIM  *im,
                                                  const char     *text,
                                                  gint            len,
                                                  gint            cursor_index);
gboolean     nimf_service_im_get_surrounding     (NimfServiceIM  *im,
                                                  gchar         **text,
                                                  gint           *cursor_index);
void         nimf_service_im_set_use_preedit     (NimfServiceIM  *im,
                                                  gboolean        use_preedit);
void         nimf_service_im_set_cursor_location (NimfServiceIM  *im,
                                                  const NimfRectangle *area);
void         nimf_service_im_reset               (NimfServiceIM  *im);
void         nimf_service_im_set_engine_by_id    (NimfServiceIM  *im,
                                                  const gchar    *engine_id);
void         nimf_service_im_engine_changed      (NimfServiceIM  *im,
                                                  const gchar    *engine_id,
                                                  const gchar    *name);
/* signals */
void     nimf_service_im_emit_preedit_start        (NimfServiceIM    *im);
void     nimf_service_im_emit_preedit_changed      (NimfServiceIM    *im,
                                                    const gchar      *preedit_string,
                                                    NimfPreeditAttr **attrs,
                                                    gint              cursor_pos);
void     nimf_service_im_emit_preedit_end          (NimfServiceIM    *im);
void     nimf_service_im_emit_commit               (NimfServiceIM    *im,
                                                    const gchar      *text);
gboolean nimf_service_im_emit_retrieve_surrounding (NimfServiceIM    *im);
gboolean nimf_service_im_emit_delete_surrounding   (NimfServiceIM    *im,
                                                    gint              offset,
                                                    gint              n_chars);
void     nimf_service_im_emit_beep                 (NimfServiceIM    *im);

G_END_DECLS

#endif /* __NIMF_SERVICE_IM_H__ */
