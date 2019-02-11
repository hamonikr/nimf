/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-message.h
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
