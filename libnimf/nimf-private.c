/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-private.c
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

#include "nimf-private.h"
#include <syslog.h>

gchar *
nimf_get_socket_path ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_strconcat (g_get_user_runtime_dir (), "/nimf/socket", NULL);
}

void
nimf_send_message (GSocket         *socket,
                   guint16          icid,
                   NimfMessageType  type,
                   gpointer         data,
                   guint16          data_len,
                   GDestroyNotify   data_destroy_func)
{
  g_debug (G_STRLOC ": %s: fd = %d", G_STRFUNC, g_socket_get_fd (socket));

  NimfMessage   *message;
  GError        *error = NULL;
  gssize         n_written;
  GOutputVector  vectors[2] = { { NULL, }, };

  message = nimf_message_new_full (type, icid,
                                   data, data_len, data_destroy_func);

  vectors[0].buffer = nimf_message_get_header (message);
  vectors[0].size   = nimf_message_get_header_size ();
  vectors[1].buffer = message->data;
  vectors[1].size   = message->header->data_len;

  n_written = g_socket_send_message (socket, NULL, vectors,
                                     message->header->data_len > 0 ? 2 : 1,
                                     NULL, 0, 0, NULL, &error);

  if (G_UNLIKELY (n_written != nimf_message_get_header_size () + message->header->data_len))
  {
    g_critical (G_STRLOC ": %s: n_written %"G_GSSIZE_FORMAT" differs from %d",
                G_STRFUNC, n_written, nimf_message_get_header_size () + message->header->data_len);

    if (error)
    {
      g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
      g_error_free (error);
    }

    nimf_message_unref (message);

    return;
  }

  /* debug message */
  const gchar *name = nimf_message_get_name (message);
  if (name)
    g_debug ("send: %s, icid: %d, fd: %d", name, icid, g_socket_get_fd(socket));
  else
    g_error (G_STRLOC ": unknown message type");

  nimf_message_unref (message);
}

NimfMessage *nimf_recv_message (GSocket *socket)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMessage *message = nimf_message_new ();
  GError *error = NULL;
  gssize n_read = 0;

  n_read = g_socket_receive (socket,
                             (gchar *) message->header,
                             nimf_message_get_header_size (),
                             NULL, &error);

  if (G_UNLIKELY (n_read < nimf_message_get_header_size ()))
  {
    g_critical (G_STRLOC ": %s: received %"G_GSSIZE_FORMAT" less than %d",
                G_STRFUNC, n_read, nimf_message_get_header_size ());

    if (error)
    {
      g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
      g_error_free (error);
    }

    nimf_message_unref (message);

    return NULL;
  }

  if (message->header->data_len > 0)
  {
    nimf_message_set_body (message,
                           g_malloc0 (message->header->data_len),
                           message->header->data_len,
                           g_free);

    n_read = g_socket_receive (socket,
                               message->data,
                               message->header->data_len,
                               NULL, &error);

    if (G_UNLIKELY (n_read < message->header->data_len))
    {
      g_critical (G_STRLOC ": %s: received %"G_GSSIZE_FORMAT" less than %d",
                  G_STRFUNC, n_read, message->header->data_len);

      if (error)
      {
        g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
        g_error_free (error);
      }

      nimf_message_unref (message);

      return NULL;
    }
  }

  /* debug message */
  const gchar *name = nimf_message_get_name (message);
  if (name)
    g_debug ("recv: %s, icid: %d, fd: %d", name, message->header->icid, g_socket_get_fd (socket));
  else
    g_error (G_STRLOC ": unknown message type");

  return message;
}

void nimf_log_default_handler (const gchar    *log_domain,
                               GLogLevelFlags  log_level,
                               const gchar    *message,
                               gboolean       *debug)
{
  int priority;
  const gchar *prefix;

  switch (log_level & G_LOG_LEVEL_MASK)
  {
    case G_LOG_LEVEL_ERROR:
      priority = LOG_ERR;
      prefix = "ERROR **";
      break;
    case G_LOG_LEVEL_CRITICAL:
      priority = LOG_CRIT;
      prefix = "CRITICAL **";
      break;
    case G_LOG_LEVEL_WARNING:
      priority = LOG_WARNING;
      prefix = "WARNING **";
      break;
    case G_LOG_LEVEL_MESSAGE:
      priority = LOG_NOTICE;
      prefix = "Message";
      break;
    case G_LOG_LEVEL_INFO:
      priority = LOG_INFO;
      prefix = "INFO";
      break;
    case G_LOG_LEVEL_DEBUG:
      priority = LOG_DEBUG;
      prefix = "DEBUG";
      break;
    default:
      priority = LOG_NOTICE;
      prefix = "LOG";
      break;
  }

  if (priority == LOG_DEBUG && (debug == NULL || *debug == FALSE))
    return;

  syslog (priority, "%s-%s: %s", log_domain, prefix, message ? message : "(NULL) message");
}

void
nimf_result_iteration_until (NimfResult      *result,
                             GMainContext    *main_context,
                             guint16          icid,
                             NimfMessageType  type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  do {
    result->is_dispatched = FALSE;
    g_main_context_iteration (main_context, TRUE);
  } while ((result->is_dispatched == FALSE) ||
           (result->reply && ((result->reply->header->type != type) ||
                              (result->reply->header->icid != icid))));

  if (G_UNLIKELY (result->is_dispatched == TRUE && result->reply == NULL))
    g_critical (G_STRLOC ": %s:Can't receive %s", G_STRFUNC,
                nimf_message_get_name_by_type (type));

  /* This prevents not checking reply in the following iteration
   *                               send commit (wait reply)
   *                               recv   reset
   *                               send     commit (wait reply)
   *                               recv     commit-reply (is_dispatched: TRUE)
   * `result->is_dispatched = FALSE' prevents breaking loop
   *                               send   reset-reply
   *                               recv commit-reply
   */
  result->is_dispatched = FALSE;
}
