/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-im.c
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

#include "dasom-im.h"
#include "dasom-events.h"
#include "dasom-types.h"
#include "dasom-module-manager.h"
#include "dasom-key-syms.h"
#include "dasom-marshalers.h"
#include <gio/gunixsocketaddress.h>
#include "dasom-message.h"
#include "dasom-private.h"

enum {
  PREEDIT_START,
  PREEDIT_END,
  PREEDIT_CHANGED,
  COMMIT,
  RETRIEVE_SURROUNDING,
  DELETE_SURROUNDING,
  LAST_SIGNAL
};

static guint im_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (DasomIM, dasom_im, G_TYPE_OBJECT);

static gboolean
on_response (GSocket      *socket,
             GIOCondition  condition,
             gpointer      user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomIM *im = DASOM_IM (user_data);

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    g_socket_close (socket, NULL);
    dasom_message_free (im->reply);
    im->reply = NULL;
    /* FIXME */
    g_error (G_STRLOC ": %s", G_STRFUNC);

    return G_SOURCE_REMOVE;
  }

  DasomMessage *message;
  message = dasom_recv_message (socket);
  dasom_message_free (im->reply);
  im->reply = message;

  switch (message->type)
  {
    /* reply */
    case DASOM_MESSAGE_FILTER_EVENT_REPLY:
    case DASOM_MESSAGE_GET_PREEDIT_STRING_REPLY:
    case DASOM_MESSAGE_RESET_REPLY:
    case DASOM_MESSAGE_FOCUS_IN_REPLY:
    case DASOM_MESSAGE_FOCUS_OUT_REPLY:
      break;
    /* signals */
    case DASOM_MESSAGE_PREEDIT_START:
      g_signal_emit_by_name (im, "preedit-start");
      dasom_send_message (socket, DASOM_MESSAGE_PREEDIT_START_REPLY, NULL, NULL);
      break;
    case DASOM_MESSAGE_PREEDIT_END:
      g_signal_emit_by_name (im, "preedit-end");
      dasom_send_message (socket, DASOM_MESSAGE_PREEDIT_END_REPLY, NULL, NULL);
      break;
    case DASOM_MESSAGE_PREEDIT_CHANGED:
      g_signal_emit_by_name (im, "preedit-changed");
      dasom_send_message (socket, DASOM_MESSAGE_PREEDIT_CHANGED_REPLY, NULL, NULL);
      break;
    case DASOM_MESSAGE_COMMIT:
      g_print ("g_signal_emit_by_name:commit:%s\n", (const gchar *) message->body.data);
      g_signal_emit_by_name (im, "commit", (const gchar *) message->body.data);
      dasom_send_message (socket, DASOM_MESSAGE_COMMIT_REPLY, NULL, NULL);
      break;
    case DASOM_MESSAGE_RETRIEVE_SURROUNDING:
      g_signal_emit_by_name (im, "retrieve-surrounding");
      dasom_send_message (socket, DASOM_MESSAGE_RETRIEVE_SURROUNDING_REPLY, NULL, NULL);
      break;
    case DASOM_MESSAGE_DELETE_SURROUNDING:
      g_signal_emit_by_name (im, "delete-surrounding");
      dasom_send_message (socket, DASOM_MESSAGE_DELETE_SURROUNDING_REPLY, NULL, NULL);
      break;
    default:
      g_warning (G_STRLOC ": %s: Unknown message type: %d", G_STRFUNC, message->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

void
dasom_im_loop_until (DasomIM          *im,
                     DasomMessageType  type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  /* FIXME: socket 변수가 im 스트럭에 들어가는건 어떤지 고려할 것 */
  GSocket *socket = g_socket_connection_get_socket (im->connection);

  do {
    g_socket_condition_wait (socket, G_IO_IN, NULL, NULL);
    GIOCondition condition = g_socket_condition_check (socket, G_IO_IN | G_IO_HUP | G_IO_ERR);

    if (!on_response (socket, condition, im))
      break; /* TODO: error handling */

  } while (im->reply->type != type); /* <<< 요런 부분이 NULL 때문에 에러 발생 가능성이 높은 부분 */

  /* 연결이 끊어지면 아무 것도 받지 못하여 on_response에서 NULL로 설정합니다. */
  /* FIXME: 추후 이에 대한 처리가 있어야 합니다. */
  /* 클라이언트 부분과 서버 부분에도 이와 비슷한 코드를 사용하므로
   * 통합적인 코드가 필요하겠습니다.
   * DasomConnection 의 필요성이 느껴집니다 */

  g_assert (im->reply != NULL);

  if (im->reply->type != type)
  {
    g_print ("error NOT MATCH\n");

    const gchar *name = dasom_message_get_name (im->reply);
    if (name)
      g_print ("reply_type: %s\n", name);
    else
      g_error ("unknown reply_type type");
  }

  g_assert (im->reply->type == type);
}

void dasom_im_focus_out (DasomIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  dasom_send_message (socket, DASOM_MESSAGE_FOCUS_OUT, NULL, NULL);
  dasom_im_loop_until (im, DASOM_MESSAGE_FOCUS_OUT_REPLY);

  g_assert (im->reply->type == DASOM_MESSAGE_FOCUS_OUT_REPLY);
}

void dasom_im_set_cursor_location (DasomIM        *im,
                                   DasomRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void dasom_im_set_use_preedit (DasomIM  *im,
                               gboolean  use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

gboolean dasom_im_get_surrounding (DasomIM  *im,
                                   gchar   **text,
                                   gint     *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return FALSE;
}

void dasom_im_set_surrounding (DasomIM   *im,
                              const char *text,
                              gint        len,
                              gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void dasom_im_focus_in (DasomIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  dasom_send_message (socket, DASOM_MESSAGE_FOCUS_IN, NULL, NULL);
  dasom_im_loop_until (im, DASOM_MESSAGE_FOCUS_IN_REPLY);

  g_assert (im->reply->type == DASOM_MESSAGE_FOCUS_IN_REPLY);
}

void
dasom_im_get_preedit_string (DasomIM  *im,
                             gchar   **str,
                             gint     *cursor_pos)
{
  g_debug (G_STRLOC ":REQ %s", G_STRFUNC);

  g_return_val_if_fail (DASOM_IS_IM (im), FALSE);

  GSocket *socket = g_socket_connection_get_socket (im->connection);

  if (!socket || g_socket_is_closed (socket))
  {
    if (str)
      *str = g_strdup ("");

    if (cursor_pos)
      *cursor_pos = 0;

    g_warning ("socket is closed");

    return;
  }

  dasom_send_message (socket, DASOM_MESSAGE_GET_PREEDIT_STRING, NULL, NULL);
  dasom_im_loop_until (im, DASOM_MESSAGE_GET_PREEDIT_STRING_REPLY);

  g_assert (im->reply->type == DASOM_MESSAGE_GET_PREEDIT_STRING_REPLY);

  gchar *preedit_str; /* do not free */
  gint   pos;
  gint   str_len = im->reply->body.data_len - 1 - sizeof (gint);

  preedit_str = g_strndup (im->reply->body.data, str_len);
  pos = *(gint *) (im->reply->body.data + str_len + 1);

  if (str)
  {
    *str = preedit_str;
    g_print ("preedit:%s\n", *str);
  }
  else
    g_free (preedit_str);

  if (cursor_pos)
  {
    *cursor_pos = pos;
    g_print ("cursor_pos:%d\n", *cursor_pos);
  }

  /* TODO: g_source_add */

  g_return_if_fail (str == NULL || g_utf8_validate (*str, -1, NULL));

  return;
}

void dasom_im_reset (DasomIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  dasom_send_message (socket, DASOM_MESSAGE_RESET, NULL, NULL);
  dasom_im_loop_until (im, DASOM_MESSAGE_RESET_REPLY);

  g_assert (im->reply->type == DASOM_MESSAGE_RESET_REPLY);
}

gboolean dasom_im_filter_event (DasomIM *im, DasomEvent *event)
{
  g_debug (G_STRLOC ":REQ %s", G_STRFUNC);

  g_return_val_if_fail (DASOM_IS_IM (im), FALSE);

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return FALSE;
  }

  dasom_send_message (socket, DASOM_MESSAGE_FILTER_EVENT, event, (GDestroyNotify) dasom_event_free);
  dasom_im_loop_until (im, DASOM_MESSAGE_FILTER_EVENT_REPLY);

  gboolean retval = FALSE;

  if (im->reply->type == DASOM_MESSAGE_FILTER_EVENT_REPLY)
    retval = *(gboolean *) (im->reply->body.data);

  return retval;
}

DasomIM *
dasom_im_new (void)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (DASOM_TYPE_IM, NULL);
}

/* TODO: 어떠한 이유로 서버와 접속이 불가할 경우, slave 또는 fallback 모드가 필요합니다 */
static void
dasom_im_init (DasomIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSocketClient  *client;
  GSocketAddress *address;
  GSocket        *socket;

  address = g_unix_socket_address_new_with_type ("unix:abstract=dasom",
                                                  -1,
                                                  G_UNIX_SOCKET_ADDRESS_ABSTRACT);

  client = g_socket_client_new ();
  im->connection = g_socket_client_connect (client,
                                            G_SOCKET_CONNECTABLE (address),
                                            NULL,
                                            NULL);
  g_object_unref (address);
  if (im->connection == NULL)
    return; /* 에러 메시지 있어야 한다 */

  socket = g_socket_connection_get_socket (im->connection);
  /* FALSE 이면 im 이 생성되지 않으므로 initable 을 포기하고 디폴트로 영어가
   * 입력될 수 있도록 해야 한다. */
  if (!socket)
    return; /* 에러 메시지 있어야 한다 */

  DasomMessage *message;

/* FIXME: 우아하게 처리할 필요가 있음 */
  DasomConnectionType *type = g_malloc0 (sizeof (DasomConnectionType));
  *type = DASOM_CONNECTION_DASOM_IM;

  dasom_send_message (socket, DASOM_MESSAGE_CONNECT, type, g_free);
  g_socket_condition_wait (socket, G_IO_IN, NULL, NULL);
  message = dasom_recv_message (socket);

  if (message->type != DASOM_MESSAGE_CONNECT_REPLY)
    g_error ("FIXME: error handling");

  dasom_message_free (message);

  GSource *source = g_socket_create_source (socket, G_IO_IN | G_IO_HUP | G_IO_ERR, NULL);
  g_source_attach (source, NULL);
  g_source_set_callback (source,
                         (GSourceFunc) on_response,
                         im,
                         NULL);
}

static void
dasom_im_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomIM *im = DASOM_IM (object);

  if (im->connection)
    g_object_unref (im->connection);

  G_OBJECT_CLASS (dasom_im_parent_class)->finalize (object);
}

static void
dasom_im_class_init (DasomIMClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dasom_im_finalize;

  im_signals[PREEDIT_START] =
    g_signal_new (g_intern_static_string ("preedit-start"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomIMClass, preedit_start),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  im_signals[PREEDIT_END] =
    g_signal_new (g_intern_static_string ("preedit-end"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomIMClass, preedit_end),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  im_signals[PREEDIT_CHANGED] =
    g_signal_new (g_intern_static_string ("preedit-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomIMClass, preedit_changed),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  im_signals[COMMIT] =
    g_signal_new (g_intern_static_string ("commit"),
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomIMClass, commit),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  im_signals[RETRIEVE_SURROUNDING] =
    g_signal_new (g_intern_static_string ("retrieve-surrounding"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomIMClass, retrieve_surrounding),
                  g_signal_accumulator_true_handled, NULL,
                  dasom_cclosure_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);

  im_signals[DELETE_SURROUNDING] =
    g_signal_new (g_intern_static_string ("delete-surrounding"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomIMClass, delete_surrounding),
                  g_signal_accumulator_true_handled, NULL,
                  dasom_cclosure_marshal_BOOLEAN__INT_INT,
                  G_TYPE_BOOLEAN, 2,
                  G_TYPE_INT,
                  G_TYPE_INT);
}
