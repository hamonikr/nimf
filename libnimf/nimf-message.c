/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-message.c
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

#include "nimf-message-private.h"
#include "nimf-types.h"
#include "nimf-message-enum-types-private.h"
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

NimfResult *
nimf_result_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfResult *result;

  result = g_slice_new0 (NimfResult);
  result->ref_count = 1;

  return result;
}

NimfResult *
nimf_result_ref (NimfResult *result)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (result != NULL, NULL);

  g_atomic_int_inc (&result->ref_count);

  return result;
}

void
nimf_result_unref (NimfResult *result)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (result == NULL))
    return;

  if (g_atomic_int_dec_and_test (&result->ref_count))
  {
    if (result->reply)
      g_slice_free (NimfMessage, result->reply);

    g_slice_free (NimfResult, result);
  }
}

void
nimf_result_iteration_until (NimfResult      *result,
                             GMainContext    *main_context,
                             guint16          icid,
                             NimfMessageType  type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_result_ref (result);

  do {
    result->is_dispatched = FALSE;
    g_main_context_iteration (main_context, TRUE);
  } while ((result->is_dispatched == FALSE) ||
           (result->reply && ((result->reply->header->type != type) ||
                              (result->reply->header->icid != icid))));

  if (G_UNLIKELY (result->is_dispatched == TRUE && result->reply == NULL))
    g_debug (G_STRLOC ": %s: Can't receive %s",
             G_STRFUNC, nimf_message_get_name_by_type (type));

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

  nimf_result_unref (result);
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
    g_debug (G_STRLOC ": %s: n_written %"G_GSSIZE_FORMAT" differs from %d",
             G_STRFUNC, n_written, nimf_message_get_header_size () + message->header->data_len);

    if (error)
    {
      g_debug (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
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
    g_debug (G_STRLOC ": %s: received %"G_GSSIZE_FORMAT" less than %d",
             G_STRFUNC, n_read, nimf_message_get_header_size ());

    if (error)
    {
      g_debug (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
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
      g_debug (G_STRLOC ": %s: received %"G_GSSIZE_FORMAT" less than %d",
               G_STRFUNC, n_read, message->header->data_len);

      if (error)
      {
        g_debug (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
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
