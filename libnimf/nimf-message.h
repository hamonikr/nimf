/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-message.h
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

#ifndef __NIMF_MESSAGE_H__
#define __NIMF_MESSAGE_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-events.h"

G_BEGIN_DECLS

typedef struct _NimfMessage       NimfMessage;
typedef struct _NimfMessageHeader NimfMessageHeader;

typedef enum
{
  NIMF_MESSAGE_NONE = 0,
  /* im methods */
  NIMF_MESSAGE_CREATE_CONTEXT,
  NIMF_MESSAGE_CREATE_CONTEXT_REPLY,
  NIMF_MESSAGE_DESTROY_CONTEXT,
  NIMF_MESSAGE_DESTROY_CONTEXT_REPLY,
  NIMF_MESSAGE_FILTER_EVENT,
  NIMF_MESSAGE_FILTER_EVENT_REPLY,
  NIMF_MESSAGE_RESET,
  NIMF_MESSAGE_RESET_REPLY,
  NIMF_MESSAGE_FOCUS_IN,
  NIMF_MESSAGE_FOCUS_IN_REPLY,
  NIMF_MESSAGE_FOCUS_OUT,
  NIMF_MESSAGE_FOCUS_OUT_REPLY,
  NIMF_MESSAGE_SET_SURROUNDING,
  NIMF_MESSAGE_SET_SURROUNDING_REPLY,
  NIMF_MESSAGE_GET_SURROUNDING,
  NIMF_MESSAGE_GET_SURROUNDING_REPLY,
  NIMF_MESSAGE_SET_CURSOR_LOCATION,
  NIMF_MESSAGE_SET_CURSOR_LOCATION_REPLY,
  NIMF_MESSAGE_SET_USE_PREEDIT,
  NIMF_MESSAGE_SET_USE_PREEDIT_REPLY,
  /* context signals */
  NIMF_MESSAGE_PREEDIT_START,
  NIMF_MESSAGE_PREEDIT_START_REPLY,
  NIMF_MESSAGE_PREEDIT_END,
  NIMF_MESSAGE_PREEDIT_END_REPLY,
  NIMF_MESSAGE_PREEDIT_CHANGED,
  NIMF_MESSAGE_PREEDIT_CHANGED_REPLY,
  NIMF_MESSAGE_COMMIT,
  NIMF_MESSAGE_COMMIT_REPLY,
  NIMF_MESSAGE_RETRIEVE_SURROUNDING,
  NIMF_MESSAGE_RETRIEVE_SURROUNDING_REPLY,
  NIMF_MESSAGE_DELETE_SURROUNDING,
  NIMF_MESSAGE_DELETE_SURROUNDING_REPLY,
  /* misc */
  NIMF_MESSAGE_BEEP,
  NIMF_MESSAGE_BEEP_REPLY,
  /* command */
  NIMF_MESSAGE_START_INDICATOR,
  NIMF_MESSAGE_START_INDICATOR_REPLY
} NimfMessageType;

struct _NimfMessageHeader
{
  guint16         icid;
  NimfMessageType type;
  guint16         data_len;
};

struct _NimfMessage
{
  NimfMessageHeader *header;
  gchar             *data;
  GDestroyNotify     data_destroy_func;
  gint               ref_count;
};

NimfMessage  *nimf_message_new              (void);
NimfMessage  *nimf_message_new_full         (NimfMessageType  type,
                                             guint16          im_id,
                                             gpointer         data,
                                             guint16          data_len,
                                             GDestroyNotify   data_destroy_func);
NimfMessage  *nimf_message_ref              (NimfMessage     *message);
void          nimf_message_unref            (NimfMessage     *message);
const NimfMessageHeader *
              nimf_message_get_header       (NimfMessage     *message);
guint16       nimf_message_get_header_size  (void);
void          nimf_message_set_body         (NimfMessage     *message,
                                             gchar           *data,
                                             guint16          data_len,
                                             GDestroyNotify   data_destroy_func);
const gchar  *nimf_message_get_body         (NimfMessage     *message);
guint16       nimf_message_get_body_size    (NimfMessage     *message);
const gchar  *nimf_message_get_name         (NimfMessage     *message);
const gchar  *nimf_message_get_name_by_type (NimfMessageType  type);

G_END_DECLS

#endif /* __NIMF_MESSAGE_H__ */
