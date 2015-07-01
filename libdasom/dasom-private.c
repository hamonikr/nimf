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

#include "dasom.h"
#include "dasom-private.h"

void
dasom_send_message (GSocket          *socket,
                    DasomMessageType  type,
                    gpointer          data,
                    GDestroyNotify    data_destroy_func)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomMessage *message;

  message = dasom_message_new_full (type, data, data_destroy_func);

  g_socket_send (socket,
                 (gchar *) message,
                 sizeof (DasomMessageHeader),
                 NULL, NULL);

  if (message->header.data_len > 0)
    g_socket_send (socket,
                   message->body.data,
                   message->body.data_len,
                   NULL, NULL);

  const gchar *name = dasom_message_get_name (message);
  if (name)
    g_print ("send: %s, fd: %d\n", name, g_socket_get_fd(socket));
  else
    g_error ("unknown message type");

  dasom_message_free (message);
}

DasomMessage *dasom_recv_message (GSocket *socket)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomMessage *message = dasom_message_new ();

  gssize n_read = 0;

  n_read = g_socket_receive (socket,
                             (gchar *) message,
                             sizeof (DasomMessageHeader),
                             NULL, NULL);

  /* FIXME: 에러 처리해야 함 */
  g_assert (n_read == sizeof (DasomMessageHeader));

  if (message->header.data_len > 1)
  {
    message->body.data = g_malloc0 (message->header.data_len);
    g_socket_condition_wait (socket, G_IO_IN, NULL, NULL);
    n_read = g_socket_receive (socket,
                               message->body.data,
                               message->header.data_len,
                               NULL,
                               NULL);
    g_assert (n_read == message->header.data_len);
  }

  const gchar *name = dasom_message_get_name (message);
  if (name)
    g_print ("recv: %s, fd: %d\n", name, g_socket_get_fd(socket));
  else
    g_error ("unknown message type");

  return message;
}
