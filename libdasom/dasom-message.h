/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-message.h
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

#ifndef __DASOM_MESSAGE_H__
#define __DASOM_MESSAGE_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>
#include "dasom-events.h"

G_BEGIN_DECLS

typedef struct _DasomMessage       DasomMessage;
typedef struct _DasomMessageHeader DasomMessageHeader;

typedef enum
{
  DASOM_MESSAGE_NONE = 0,
  /* im methods */
  DASOM_MESSAGE_CONNECT,
  DASOM_MESSAGE_CONNECT_REPLY,
  DASOM_MESSAGE_FILTER_EVENT,
  DASOM_MESSAGE_FILTER_EVENT_REPLY,
  DASOM_MESSAGE_RESET,
  DASOM_MESSAGE_RESET_REPLY,
  DASOM_MESSAGE_FOCUS_IN,
  DASOM_MESSAGE_FOCUS_IN_REPLY,
  DASOM_MESSAGE_FOCUS_OUT,
  DASOM_MESSAGE_FOCUS_OUT_REPLY,
  DASOM_MESSAGE_SET_SURROUNDING,
  DASOM_MESSAGE_SET_SURROUNDING_REPLY,
  DASOM_MESSAGE_GET_SURROUNDING,
  DASOM_MESSAGE_GET_SURROUNDING_REPLY,
  DASOM_MESSAGE_SET_CURSOR_LOCATION,
  DASOM_MESSAGE_SET_CURSOR_LOCATION_REPLY,
  DASOM_MESSAGE_SET_USE_PREEDIT,
  DASOM_MESSAGE_SET_USE_PREEDIT_REPLY,
  /* context signals */
  DASOM_MESSAGE_PREEDIT_START,
  DASOM_MESSAGE_PREEDIT_START_REPLY,
  DASOM_MESSAGE_PREEDIT_END,
  DASOM_MESSAGE_PREEDIT_END_REPLY,
  DASOM_MESSAGE_PREEDIT_CHANGED,
  DASOM_MESSAGE_PREEDIT_CHANGED_REPLY,
  DASOM_MESSAGE_COMMIT,
  DASOM_MESSAGE_COMMIT_REPLY,
  DASOM_MESSAGE_RETRIEVE_SURROUNDING,
  DASOM_MESSAGE_RETRIEVE_SURROUNDING_REPLY,
  DASOM_MESSAGE_DELETE_SURROUNDING,
  DASOM_MESSAGE_DELETE_SURROUNDING_REPLY,
  DASOM_MESSAGE_ENGINE_CHANGED,

  DASOM_MESSAGE_ERROR
} DasomMessageType;

struct _DasomMessageHeader
{
  DasomMessageType type;
  guint16          data_len;
};

struct _DasomMessage
{
  DasomMessageHeader *header;
  gchar              *data;
  GDestroyNotify      data_destroy_func;
  gint                ref_count;
};

DasomMessage *dasom_message_new          (void);
DasomMessage *dasom_message_new_full     (DasomMessageType  type,
                                          gpointer          data,
                                          guint16           data_len,
                                          GDestroyNotify    data_destroy_func);
DasomMessage *dasom_message_ref          (DasomMessage     *message);
void          dasom_message_unref        (DasomMessage     *message);
const DasomMessageHeader *
              dasom_message_get_header       (DasomMessage     *message);
guint16       dasom_message_get_header_size  (void);
void          dasom_message_set_body         (DasomMessage     *message,
                                              gchar            *data,
                                              guint16           data_len,
                                              GDestroyNotify    data_destroy_func);
const gchar  *dasom_message_get_body         (DasomMessage     *message);
guint16       dasom_message_get_body_size    (DasomMessage     *message);
const gchar  *dasom_message_get_name         (DasomMessage     *message);
const gchar  *dasom_message_get_name_by_type (DasomMessageType  type);

G_END_DECLS

#endif /* __DASOM_MESSAGE_H__ */
