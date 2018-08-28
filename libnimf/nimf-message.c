/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-message.c
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

#include "nimf-message.h"
#include "nimf-types.h"
#include "nimf-enum-types.h"
#include <string.h>

NimfMessage *
nimf_message_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_message_new_full (NIMF_MESSAGE_NONE, 0, NULL, 0, NULL);
}

NimfMessage *
nimf_message_new_full (NimfMessageType type,
                       guint16         icid,
                       gpointer        data,
                       guint16         data_len,
                       GDestroyNotify  data_destroy_func)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMessage *message;

  message                    = g_slice_new0 (NimfMessage);
  message->header            = g_slice_new0 (NimfMessageHeader);
  message->header->icid      = icid;
  message->header->type      = type;
  message->header->data_len  = data_len;
  message->data              = data;
  message->data_destroy_func = data_destroy_func;
  message->ref_count = 1;

  return message;
}

NimfMessage *
nimf_message_ref (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (message != NULL, NULL);

  g_atomic_int_inc (&message->ref_count);

  return message;
}

void
nimf_message_unref (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (message == NULL))
    return;

  if (g_atomic_int_dec_and_test (&message->ref_count))
  {
    g_slice_free (NimfMessageHeader, message->header);

    if (message->data_destroy_func)
      message->data_destroy_func (message->data);

    g_slice_free (NimfMessage, message);
  }
}

const NimfMessageHeader *
nimf_message_get_header (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->header;
}

guint16
nimf_message_get_header_size ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return sizeof (NimfMessageHeader);
}

void
nimf_message_set_body (NimfMessage    *message,
                       gchar          *data,
                       guint16         data_len,
                       GDestroyNotify  data_destroy_func)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  message->data              = data;
  message->header->data_len  = data_len;
  message->data_destroy_func = data_destroy_func;
}

const gchar *
nimf_message_get_body (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->data;
}

guint16
nimf_message_get_body_size (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->header->data_len;
}

const gchar *nimf_message_get_name (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GEnumClass *enum_class = (GEnumClass *) g_type_class_ref (NIMF_TYPE_MESSAGE_TYPE);
  GEnumValue *enum_value = g_enum_get_value (enum_class, message->header->type);
  g_type_class_unref (enum_class);

  return enum_value ? enum_value->value_name : NULL;
}

const gchar *nimf_message_get_name_by_type (NimfMessageType type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GEnumClass *enum_class = (GEnumClass *) g_type_class_ref (NIMF_TYPE_MESSAGE_TYPE);
  GEnumValue *enum_value = g_enum_get_value (enum_class, type);
  g_type_class_unref (enum_class);

  return enum_value ? enum_value->value_name : NULL;
}
