/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-im.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015,2016 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-im.h"
#include "nimf-events.h"
#include "nimf-types.h"
#include "nimf-module-manager.h"
#include "nimf-key-syms.h"
#include "nimf-marshalers.h"
#include <gio/gunixsocketaddress.h>
#include "nimf-message.h"
#include "nimf-private.h"
#include <string.h>

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
static GMainContext *nimf_im_sockets_context = NULL;
static guint         nimf_im_sockets_context_ref_count = 0;

G_DEFINE_TYPE (NimfIM, nimf_im, G_TYPE_OBJECT);

static gboolean
on_incoming_message (GSocket      *socket,
                     GIOCondition  condition,
                     gpointer      user_data)
{
  g_debug (G_STRLOC ": %s: socket fd:%d", G_STRFUNC, g_socket_get_fd (socket));

  NimfIM *im = NIMF_IM (user_data);
  nimf_message_unref (im->result->reply);
  im->result->is_dispatched = TRUE;

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    /* Because two GSource is created over one socket,
     * when G_IO_HUP | G_IO_ERR, callback can run two times.
     * the following code avoid that callback runs two times. */
    GSource *source = g_main_current_source ();

    if (source == im->default_context_source)
      g_source_destroy (im->sockets_context_source);
    else if (source == im->sockets_context_source)
      g_source_destroy (im->default_context_source);

    if (!g_socket_is_closed (socket))
      g_socket_close (socket, NULL);

    im->result->reply    = NULL;

    g_critical (G_STRLOC ": %s: G_IO_HUP | G_IO_ERR", G_STRFUNC);

    return G_SOURCE_REMOVE;
  }

  NimfMessage *message;
  message = nimf_recv_message (socket);
  im->result->reply = message;
  gboolean retval;

  if (G_UNLIKELY (message == NULL))
  {
    g_critical (G_STRLOC ": NULL message");
    return G_SOURCE_CONTINUE;
  }

  switch (message->header->type)
  {
    /* reply */
    case NIMF_MESSAGE_FILTER_EVENT_REPLY:
    case NIMF_MESSAGE_RESET_REPLY:
    case NIMF_MESSAGE_FOCUS_IN_REPLY:
    case NIMF_MESSAGE_FOCUS_OUT_REPLY:
    case NIMF_MESSAGE_SET_SURROUNDING_REPLY:
    case NIMF_MESSAGE_GET_SURROUNDING_REPLY:
    case NIMF_MESSAGE_SET_CURSOR_LOCATION_REPLY:
    case NIMF_MESSAGE_SET_USE_PREEDIT_REPLY:
      break;
    /* signals */
    case NIMF_MESSAGE_PREEDIT_START:
      g_signal_emit_by_name (im, "preedit-start");
      nimf_send_message (socket, NIMF_MESSAGE_PREEDIT_START_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_PREEDIT_END:
      g_signal_emit_by_name (im, "preedit-end");
      nimf_send_message (socket, NIMF_MESSAGE_PREEDIT_END_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_PREEDIT_CHANGED:
      g_free (im->preedit_string);
      im->preedit_string = g_strndup (message->data,
                                      message->header->data_len - 1 - sizeof (gint));
      im->cursor_pos = *(gint *) (message->data +
                                  message->header->data_len - sizeof (gint));
      g_signal_emit_by_name (im, "preedit-changed");
      nimf_send_message (socket, NIMF_MESSAGE_PREEDIT_CHANGED_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_COMMIT:
      nimf_message_ref (message);
      g_signal_emit_by_name (im, "commit", (const gchar *) message->data);
      nimf_message_unref (message);
      nimf_send_message (socket, NIMF_MESSAGE_COMMIT_REPLY, NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_RETRIEVE_SURROUNDING:
      g_signal_emit_by_name (im, "retrieve-surrounding", &retval);
      nimf_send_message (socket, NIMF_MESSAGE_RETRIEVE_SURROUNDING_REPLY,
                         &retval, sizeof (gboolean), NULL);
      break;
    case NIMF_MESSAGE_DELETE_SURROUNDING:
      nimf_message_ref (message);
      g_signal_emit_by_name (im, "delete-surrounding",
                             ((gint *) message->data)[0],
                             ((gint *) message->data)[1], &retval);
      nimf_message_unref (message);
      nimf_send_message (socket, NIMF_MESSAGE_DELETE_SURROUNDING_REPLY,
                         &retval, sizeof (gboolean), NULL);
      break;
    default:
      g_warning (G_STRLOC ": %s: Unknown message type: %d", G_STRFUNC, message->header->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

void nimf_im_focus_out (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  nimf_send_message (socket, NIMF_MESSAGE_FOCUS_OUT, NULL, 0, NULL);
  nimf_result_iteration_until (im->result, nimf_im_sockets_context,
                               NIMF_MESSAGE_FOCUS_OUT_REPLY);
}

void nimf_im_set_cursor_location (NimfIM              *im,
                                  const NimfRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  nimf_send_message (socket, NIMF_MESSAGE_SET_CURSOR_LOCATION,
                     (gchar *) area, sizeof (NimfRectangle), NULL);
  nimf_result_iteration_until (im->result, nimf_im_sockets_context,
                               NIMF_MESSAGE_SET_CURSOR_LOCATION_REPLY);
}

void nimf_im_set_use_preedit (NimfIM   *im,
                              gboolean  use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  nimf_send_message (socket, NIMF_MESSAGE_SET_USE_PREEDIT,
                     (gchar *) &use_preedit, sizeof (gboolean), NULL);
  nimf_result_iteration_until (im->result, nimf_im_sockets_context,
                               NIMF_MESSAGE_SET_USE_PREEDIT_REPLY);
}

gboolean nimf_im_get_surrounding (NimfIM  *im,
                                  gchar  **text,
                                  gint    *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_IM (im), FALSE);

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    if (text)
      *text = g_strdup ("");

    if (cursor_index)
      *cursor_index = 0;

    g_warning ("socket is closed");

    return FALSE;
  }

  nimf_send_message (socket, NIMF_MESSAGE_GET_SURROUNDING, NULL, 0, NULL);
  nimf_result_iteration_until (im->result, nimf_im_sockets_context,
                               NIMF_MESSAGE_GET_SURROUNDING_REPLY);

  if (im->result->reply == NULL)
  {
    if (text)
      *text = g_strdup ("");

    if (cursor_index)
      *cursor_index = 0;

    return FALSE;
  }

  if (text)
    *text = g_strndup (im->result->reply->data,
                       im->result->reply->header->data_len - 1 -
                       sizeof (gint) - sizeof (gboolean));

  if (cursor_index)
  {
    *cursor_index = *(gint *) (im->result->reply->data +
                               im->result->reply->header->data_len -
                               sizeof (gint) - sizeof (gboolean));
  }

  return *(gboolean *) (im->result->reply->data - sizeof (gboolean));
}

void nimf_im_set_surrounding (NimfIM     *im,
                              const char *text,
                              gint        len,
                              gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  gchar *data = NULL;
  gint   str_len;

  if (len == -1)
    str_len = strlen (text);
  else
    str_len = len;

  data = g_strndup (text, str_len);
  data = g_realloc (data, str_len + 1 + 2 * sizeof (gint));

  *(gint *) (data + str_len + 1) = len;
  *(gint *) (data + str_len + 1 + sizeof (gint)) = cursor_index;

  nimf_send_message (socket, NIMF_MESSAGE_SET_SURROUNDING, data,
                     str_len + 1 + 2 * sizeof (gint), g_free);
  nimf_result_iteration_until (im->result, nimf_im_sockets_context,
                               NIMF_MESSAGE_SET_SURROUNDING_REPLY);
}

void nimf_im_focus_in (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  nimf_send_message (socket, NIMF_MESSAGE_FOCUS_IN, NULL, 0, NULL);
  nimf_result_iteration_until (im->result, nimf_im_sockets_context,
                               NIMF_MESSAGE_FOCUS_IN_REPLY);
}

void
nimf_im_get_preedit_string (NimfIM  *im,
                            gchar  **str,
                            gint    *cursor_pos)
{
  g_debug (G_STRLOC ":%s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  if (str)
    *str = g_strdup (im->preedit_string);

  if (cursor_pos)
    *cursor_pos = im->cursor_pos;
}

void nimf_im_reset (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  nimf_send_message (socket, NIMF_MESSAGE_RESET, NULL, 0, NULL);
  nimf_result_iteration_until (im->result, nimf_im_sockets_context,
                               NIMF_MESSAGE_RESET_REPLY);
}

gboolean
nimf_im_filter_event_fallback (NimfIM    *im,
                               NimfEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if ((event->key.type   == NIMF_EVENT_KEY_RELEASE) ||
      (event->key.keyval == NIMF_KEY_Shift_L)       ||
      (event->key.keyval == NIMF_KEY_Shift_R)       ||
      (event->key.state & (NIMF_CONTROL_MASK | NIMF_MOD1_MASK)))
    return FALSE;

  gunichar ch;
  gchar buf[10];
  gint len;

  ch = nimf_keyval_to_unicode (event->key.keyval);
  g_return_val_if_fail (g_unichar_validate (ch), FALSE);

  len = g_unichar_to_utf8 (ch, buf);
  buf[len] = '\0';

  if (ch != 0 && !g_unichar_iscntrl (ch))
  {
    g_signal_emit_by_name (im, "commit", &buf);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

gboolean nimf_im_filter_event (NimfIM *im, NimfEvent *event)
{
  g_debug (G_STRLOC ":%s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_IM (im), FALSE);

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return nimf_im_filter_event_fallback (im, event);
  }

  nimf_send_message (socket, NIMF_MESSAGE_FILTER_EVENT, event,
                     sizeof (NimfEvent), NULL);
  nimf_result_iteration_until (im->result, nimf_im_sockets_context,
                               NIMF_MESSAGE_FILTER_EVENT_REPLY);

  if (im->result->reply == NULL)
    return nimf_im_filter_event_fallback (im, event);

  return *(gboolean *) (im->result->reply->data);
}

NimfIM *
nimf_im_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIM *im = g_object_new (NIMF_TYPE_IM, NULL);

  GSocketClient  *client;
  GSocketAddress *address;
  GSocket        *socket;
  GError         *error = NULL;
  gint            retry_limit = 5;
  gint            retry_count = 0;

  address = g_unix_socket_address_new_with_type (NIMF_ADDRESS, -1,
                                                 G_UNIX_SOCKET_ADDRESS_ABSTRACT);
  client = g_socket_client_new ();

  for (retry_count = 0; retry_count < retry_limit; retry_count++)
  {
    g_clear_error (&error);
    im->connection = g_socket_client_connect (client,
                                              G_SOCKET_CONNECTABLE (address),
                                              NULL, &error);
    if (im->connection)
      break;
    else
      g_usleep (G_USEC_PER_SEC);;
  }

  g_object_unref (address);
  g_object_unref (client);

  if (im->connection == NULL)
  {
    g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
    g_clear_error (&error);
    return im;
  }

  socket = g_socket_connection_get_socket (im->connection);

  if (!socket)
  {
    g_critical (G_STRLOC ": %s: %s", G_STRFUNC, "Can't get socket");
    return im;
  }

  NimfMessage *message;
  NimfConnectionType type = NIMF_CONNECTION_NIMF_IM;

  nimf_send_message (socket, NIMF_MESSAGE_CONNECT, &type,
                     sizeof (NimfConnectionType), NULL);
  g_socket_condition_wait (socket, G_IO_IN, NULL, NULL);
  message = nimf_recv_message (socket);

  if (G_UNLIKELY (message == NULL ||
                  message->header->type != NIMF_MESSAGE_CONNECT_REPLY))
  {
    nimf_message_unref (message);
    g_error ("Couldn't connect to nimf daemon");
  }

  nimf_message_unref (message);

  GMutex mutex;

  g_mutex_init (&mutex);
  g_mutex_lock (&mutex);

  if (G_UNLIKELY (nimf_im_sockets_context == NULL))
  {
    nimf_im_sockets_context = g_main_context_new ();
    nimf_im_sockets_context_ref_count++;
  }
  else
  {
    nimf_im_sockets_context = g_main_context_ref (nimf_im_sockets_context);
    nimf_im_sockets_context_ref_count++;
  }

  g_mutex_unlock (&mutex);

  /* when g_main_context_iteration(), iterate only sockets */
  im->sockets_context_source = g_socket_create_source (socket, G_IO_IN, NULL);
  g_source_set_can_recurse (im->sockets_context_source, TRUE);
  g_source_attach (im->sockets_context_source, nimf_im_sockets_context);
  g_source_set_callback (im->sockets_context_source,
                         (GSourceFunc) on_incoming_message, im, NULL);

  im->default_context_source = g_socket_create_source (socket, G_IO_IN, NULL);
  g_source_set_can_recurse (im->default_context_source, TRUE);
  g_source_set_callback (im->default_context_source,
                         (GSourceFunc) on_incoming_message, im, NULL);
  g_source_attach (im->default_context_source, NULL);

  return im;
}

static void
nimf_im_init (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  im->preedit_string = g_strdup ("");
  im->result = g_slice_new0 (NimfResult);
}

static void
nimf_im_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIM *im = NIMF_IM (object);

  if (im->sockets_context_source)
  {
    g_source_destroy (im->sockets_context_source);
    g_source_unref   (im->sockets_context_source);
  }

  if (im->default_context_source)
  {
    g_source_destroy (im->default_context_source);
    g_source_unref   (im->default_context_source);
  }

  if (im->connection)
    g_object_unref (im->connection);

  GMutex mutex;

  g_mutex_init (&mutex);
  g_mutex_lock (&mutex);

  if (nimf_im_sockets_context)
  {
    g_main_context_unref (nimf_im_sockets_context);
    nimf_im_sockets_context_ref_count--;

    if (nimf_im_sockets_context_ref_count == 0)
      nimf_im_sockets_context = NULL;
  }

  g_mutex_unlock (&mutex);
  g_free (im->preedit_string);

  if (im->result)
    g_slice_free (NimfResult, im->result);

  G_OBJECT_CLASS (nimf_im_parent_class)->finalize (object);
}

static void
nimf_im_class_init (NimfIMClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = nimf_im_finalize;

  im_signals[PREEDIT_START] =
    g_signal_new (g_intern_static_string ("preedit-start"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, preedit_start),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  im_signals[PREEDIT_END] =
    g_signal_new (g_intern_static_string ("preedit-end"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, preedit_end),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  im_signals[PREEDIT_CHANGED] =
    g_signal_new (g_intern_static_string ("preedit-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, preedit_changed),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  im_signals[COMMIT] =
    g_signal_new (g_intern_static_string ("commit"),
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, commit),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  im_signals[RETRIEVE_SURROUNDING] =
    g_signal_new (g_intern_static_string ("retrieve-surrounding"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, retrieve_surrounding),
                  g_signal_accumulator_true_handled, NULL,
                  nimf_cclosure_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);

  im_signals[DELETE_SURROUNDING] =
    g_signal_new (g_intern_static_string ("delete-surrounding"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, delete_surrounding),
                  g_signal_accumulator_true_handled, NULL,
                  nimf_cclosure_marshal_BOOLEAN__INT_INT,
                  G_TYPE_BOOLEAN, 2,
                  G_TYPE_INT,
                  G_TYPE_INT);
}
