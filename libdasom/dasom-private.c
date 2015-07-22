/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-private.c
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

#include "dasom-private.h"

void
dasom_send_message (GSocket          *socket,
                    DasomMessageType  type,
                    gpointer          data,
                    guint16           data_len,
                    GDestroyNotify    data_destroy_func)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomMessage *message;
  const DasomMessageHeader *header;
  GError *error = NULL;
  gssize n_written;

  message = dasom_message_new_full (type, data, data_len, data_destroy_func);
  header  = dasom_message_get_header (message);

  n_written = g_socket_send (socket,
                             (gchar *) header,
                             dasom_message_get_header_size (),
                             NULL, &error);

  if (G_UNLIKELY (n_written < dasom_message_get_header_size ()))
  {
    g_critical (G_STRLOC ": %s: sent %"G_GSSIZE_FORMAT" less than %d",
                G_STRFUNC, n_written, dasom_message_get_header_size ());
    if (error)
    {
      g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
      g_error_free (error);
    }

    dasom_message_unref (message);

    return;
  }

  if (G_LIKELY (message->header->data_len > 0))
  {
    n_written = g_socket_send (socket,
                               message->data,
                               message->header->data_len,
                               NULL, &error);

    if (G_UNLIKELY (n_written < message->header->data_len))
    {
      g_critical (G_STRLOC ": %s: sent %"G_GSSIZE_FORMAT" less than %d",
                  G_STRFUNC, n_written, message->header->data_len);

      if (error)
      {
        g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
        g_error_free (error);
      }

      dasom_message_unref (message);

      return;
    }
  }

  /* debug message */
  const gchar *name = dasom_message_get_name (message);
  if (name)
    g_debug ("send: %s, fd: %d", name, g_socket_get_fd(socket));
  else
    g_error ("unknown message type");

  dasom_message_unref (message);
}

DasomMessage *dasom_recv_message (GSocket *socket)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomMessage *message = dasom_message_new ();
  GError *error = NULL;
  gssize n_read = 0;

  n_read = g_socket_receive (socket,
                             (gchar *) message->header,
                             dasom_message_get_header_size (),
                             NULL, &error);

  if (G_UNLIKELY (n_read < dasom_message_get_header_size ()))
  {
    g_critical (G_STRLOC ": %s: received %"G_GSSIZE_FORMAT" less than %d",
                G_STRFUNC, n_read, message->header->data_len);

    if (error)
    {
      g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
      g_error_free (error);
    }

    dasom_message_unref (message);

    return NULL;
  }

  if (message->header->data_len > 1)
  {
    dasom_message_set_body (message,
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

      dasom_message_unref (message);

      return NULL;
    }
  }

  /* debug message */
  const gchar *name = dasom_message_get_name (message);
  if (name)
    g_debug ("recv: %s, fd: %d", name, g_socket_get_fd (socket));
  else
    g_error ("unknown message type");

  return message;
}
