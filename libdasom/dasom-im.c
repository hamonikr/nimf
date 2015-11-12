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
static GMainContext *dasom_im_sockets_context = NULL;
static guint         dasom_im_sockets_context_ref_count = 0;

G_DEFINE_TYPE (DasomIM, dasom_im, G_TYPE_OBJECT);

static gboolean
on_incoming_message (GSocket      *socket,
                     GIOCondition  condition,
                     gpointer      user_data)
{
  g_debug (G_STRLOC ": %s: socket fd:%d", G_STRFUNC, g_socket_get_fd (socket));

  DasomIM *im = DASOM_IM (user_data);

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

    dasom_message_unref (im->reply);
    im->reply = NULL;

    g_critical (G_STRLOC ": %s: G_IO_HUP | G_IO_ERR", G_STRFUNC);

    return G_SOURCE_REMOVE;
  }

  DasomMessage *message;
  message = dasom_recv_message (socket);
  dasom_message_unref (im->reply);
  im->reply = message;
  im->is_dispatched = TRUE;
  gboolean retval;

  if (G_UNLIKELY (message == NULL))
  {
    g_critical (G_STRLOC ": NULL message");
    return G_SOURCE_CONTINUE;
  }

  switch (message->header->type)
  {
    /* reply */
    case DASOM_MESSAGE_FILTER_EVENT_REPLY:
    case DASOM_MESSAGE_GET_PREEDIT_STRING_REPLY:
    case DASOM_MESSAGE_RESET_REPLY:
    case DASOM_MESSAGE_FOCUS_IN_REPLY:
    case DASOM_MESSAGE_FOCUS_OUT_REPLY:
    case DASOM_MESSAGE_SET_SURROUNDING_REPLY:
    case DASOM_MESSAGE_GET_SURROUNDING_REPLY:
    case DASOM_MESSAGE_SET_CURSOR_LOCATION_REPLY:
    case DASOM_MESSAGE_SET_USE_PREEDIT_REPLY:
      break;
    /* signals */
    case DASOM_MESSAGE_PREEDIT_START:
      g_signal_emit_by_name (im, "preedit-start");
      dasom_send_message (socket, DASOM_MESSAGE_PREEDIT_START_REPLY, NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_PREEDIT_END:
      g_signal_emit_by_name (im, "preedit-end");
      dasom_send_message (socket, DASOM_MESSAGE_PREEDIT_END_REPLY, NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_PREEDIT_CHANGED:
      g_signal_emit_by_name (im, "preedit-changed");
      dasom_send_message (socket, DASOM_MESSAGE_PREEDIT_CHANGED_REPLY, NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_COMMIT:
      dasom_message_ref (message);
      g_signal_emit_by_name (im, "commit", (const gchar *) message->data);
      dasom_message_unref (message);
      dasom_send_message (socket, DASOM_MESSAGE_COMMIT_REPLY, NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_RETRIEVE_SURROUNDING:
      g_signal_emit_by_name (im, "retrieve-surrounding", &retval);
      dasom_send_message (socket, DASOM_MESSAGE_RETRIEVE_SURROUNDING_REPLY,
                          &retval, sizeof (gboolean), NULL);
      break;
    case DASOM_MESSAGE_DELETE_SURROUNDING:
      dasom_message_ref (message);
      g_signal_emit_by_name (im, "delete-surrounding",
                             ((gint *) message->data)[0],
                             ((gint *) message->data)[1], &retval);
      dasom_message_unref (message);
      dasom_send_message (socket, DASOM_MESSAGE_DELETE_SURROUNDING_REPLY,
                          &retval, sizeof (gboolean), NULL);
      break;
    default:
      g_warning (G_STRLOC ": %s: Unknown message type: %d", G_STRFUNC, message->header->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

void
dasom_iteration_until (DasomIM          *im,
                       DasomMessageType  type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  do {
    im->is_dispatched = FALSE;
    g_main_context_iteration (dasom_im_sockets_context, TRUE);
  } while ((im->is_dispatched == FALSE) ||
           (im->reply && (im->reply->header->type != type)));

  im->is_dispatched = FALSE;

  if (G_UNLIKELY (im->reply == NULL))
  {
    g_critical (G_STRLOC ": %s:Can't receive %s", G_STRFUNC,
                dasom_message_get_name_by_type (type));
    return;
  }
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

  dasom_send_message (socket, DASOM_MESSAGE_FOCUS_OUT, NULL, 0, NULL);
  dasom_iteration_until (im, DASOM_MESSAGE_FOCUS_OUT_REPLY);
}

void dasom_im_set_cursor_location (DasomIM              *im,
                                   const DasomRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  dasom_send_message (socket, DASOM_MESSAGE_SET_CURSOR_LOCATION,
                      (gchar *) area, sizeof (DasomRectangle), NULL);
  dasom_iteration_until (im, DASOM_MESSAGE_SET_CURSOR_LOCATION_REPLY);
}

void dasom_im_set_use_preedit (DasomIM  *im,
                               gboolean  use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  dasom_send_message (socket, DASOM_MESSAGE_SET_USE_PREEDIT,
                      (gchar *) &use_preedit, sizeof (gboolean), NULL);
  dasom_iteration_until (im, DASOM_MESSAGE_SET_USE_PREEDIT_REPLY);
}

gboolean dasom_im_get_surrounding (DasomIM  *im,
                                   gchar   **text,
                                   gint     *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (DASOM_IS_IM (im), FALSE);

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

  dasom_send_message (socket, DASOM_MESSAGE_GET_SURROUNDING, NULL, 0, NULL);
  dasom_iteration_until (im, DASOM_MESSAGE_GET_SURROUNDING_REPLY);

  if (im->reply == NULL)
  {
    if (text)
      *text = g_strdup ("");

    if (cursor_index)
      *cursor_index = 0;

    return FALSE;
  }

  if (text)
    *text = g_strndup (im->reply->data,
                       im->reply->header->data_len - 1 -
                       sizeof (gint) - sizeof (gboolean));

  if (cursor_index)
  {
    *cursor_index = *(gint *) (im->reply->data +
                               im->reply->header->data_len -
                               sizeof (gint) - sizeof (gboolean));
  }

  return *(gboolean *) (im->reply->data - sizeof (gboolean));
}

void dasom_im_set_surrounding (DasomIM    *im,
                               const char *text,
                               gint        len,
                               gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_IM (im));

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

  dasom_send_message (socket, DASOM_MESSAGE_SET_SURROUNDING, data,
                      str_len + 1 + 2 * sizeof (gint), g_free);
  dasom_iteration_until (im, DASOM_MESSAGE_SET_SURROUNDING_REPLY);
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

  dasom_send_message (socket, DASOM_MESSAGE_FOCUS_IN, NULL, 0, NULL);
  dasom_iteration_until (im, DASOM_MESSAGE_FOCUS_IN_REPLY);
}

void
dasom_im_get_preedit_string (DasomIM  *im,
                             gchar   **str,
                             gint     *cursor_pos)
{
  g_debug (G_STRLOC ":%s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_IM (im));

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

  dasom_send_message (socket, DASOM_MESSAGE_GET_PREEDIT_STRING, NULL, 0, NULL);
  dasom_iteration_until (im, DASOM_MESSAGE_GET_PREEDIT_STRING_REPLY);

  if (im->reply == NULL)
  {
    if (str)
      *str = g_strdup ("");

    if (cursor_pos)
      *cursor_pos = 0;

    return;
  }

  if (str)
    *str = g_strndup (im->reply->data,
                      im->reply->header->data_len - 1 - sizeof (gint));

  if (cursor_pos)
    *cursor_pos = *(gint *) (im->reply->data +
                             im->reply->header->data_len - sizeof (gint));
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

  dasom_send_message (socket, DASOM_MESSAGE_RESET, NULL, 0, NULL);
  dasom_iteration_until (im, DASOM_MESSAGE_RESET_REPLY);
}

/* TODO: reduce duplicate code
 * dasom_im_filter_event_fallback() is made from
 * dasom_english_filter_event (DasomEngine     *engine,
 *                             DasomConnection *target,
 *                             DasomEvent      *event);
 */
gboolean
dasom_im_filter_event_fallback (DasomIM    *im,
                                DasomEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;

  if ((event->key.type   == DASOM_EVENT_KEY_RELEASE) ||
      (event->key.keyval == DASOM_KEY_Shift_L)       ||
      (event->key.keyval == DASOM_KEY_Shift_R)       ||
      (event->key.state & (DASOM_CONTROL_MASK | DASOM_MOD1_MASK)))
    return FALSE;

  gchar c = 0;

  if (event->key.keyval >= 32 && event->key.keyval <= 126)
    c = event->key.keyval;

  if (!c)
  {
    switch (event->key.keyval)
    {
      case DASOM_KEY_KP_Multiply: c = '*'; break;
      case DASOM_KEY_KP_Add:      c = '+'; break;
      case DASOM_KEY_KP_Subtract: c = '-'; break;
      case DASOM_KEY_KP_Divide:   c = '/'; break;
      default:
        break;
    }
  }

  if (!c && (event->key.state & DASOM_MOD2_MASK))
  {
    switch (event->key.keyval)
    {
      case DASOM_KEY_KP_Decimal:  c = '.'; break;
      case DASOM_KEY_KP_0:        c = '0'; break;
      case DASOM_KEY_KP_1:        c = '1'; break;
      case DASOM_KEY_KP_2:        c = '2'; break;
      case DASOM_KEY_KP_3:        c = '3'; break;
      case DASOM_KEY_KP_4:        c = '4'; break;
      case DASOM_KEY_KP_5:        c = '5'; break;
      case DASOM_KEY_KP_6:        c = '6'; break;
      case DASOM_KEY_KP_7:        c = '7'; break;
      case DASOM_KEY_KP_8:        c = '8'; break;
      case DASOM_KEY_KP_9:        c = '9'; break;
      default:
        break;
    }
  }

  if (c)
  {
    gchar *str = g_strdup_printf ("%c", c);
    g_signal_emit_by_name (im, "commit", str);
    g_free (str);
    retval = TRUE;
  }

  return retval;
}

gboolean dasom_im_filter_event (DasomIM *im, DasomEvent *event)
{
  g_debug (G_STRLOC ":%s", G_STRFUNC);

  g_return_val_if_fail (DASOM_IS_IM (im), FALSE);

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return dasom_im_filter_event_fallback (im, event);
  }

  dasom_send_message (socket, DASOM_MESSAGE_FILTER_EVENT, event,
                      sizeof (DasomEvent), NULL);
  dasom_iteration_until (im, DASOM_MESSAGE_FILTER_EVENT_REPLY);

  if (im->reply == NULL)
    return dasom_im_filter_event_fallback (im, event);

  return *(gboolean *) (im->reply->data);
}

DasomIM *
dasom_im_new (void)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (DASOM_TYPE_IM, NULL);
}

static void
dasom_im_init (DasomIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSocketClient  *client;
  GSocketAddress *address;
  GSocket        *socket;
  GError         *error = NULL;

  address = g_unix_socket_address_new_with_type (DASOM_ADDRESS, -1,
                                                 G_UNIX_SOCKET_ADDRESS_ABSTRACT);
  client = g_socket_client_new ();
  im->connection = g_socket_client_connect (client,
                                            G_SOCKET_CONNECTABLE (address),
                                            NULL, &error);
  g_object_unref (address);
  g_object_unref (client);

  if (im->connection == NULL)
  {
    g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
    g_clear_error (&error);
    return;
  }

  socket = g_socket_connection_get_socket (im->connection);

  if (!socket)
  {
    g_critical (G_STRLOC ": %s: %s", G_STRFUNC, "Can't get socket");
    return;
  }

  DasomMessage *message;

  DasomConnectionType type = DASOM_CONNECTION_DASOM_IM;

  dasom_send_message (socket, DASOM_MESSAGE_CONNECT, &type, sizeof (DasomConnectionType), NULL);
  g_socket_condition_wait (socket, G_IO_IN, NULL, NULL);
  message = dasom_recv_message (socket);

  if (G_UNLIKELY (message == NULL ||
                  message->header->type != DASOM_MESSAGE_CONNECT_REPLY))
  {
    dasom_message_unref (message);
    g_error ("Couldn't connect to dasom daemon");
  }

  dasom_message_unref (message);

  GMutex mutex;

  g_mutex_init (&mutex);
  g_mutex_lock (&mutex);

  if (G_UNLIKELY (dasom_im_sockets_context == NULL))
  {
    dasom_im_sockets_context = g_main_context_new ();
    dasom_im_sockets_context_ref_count++;
  }
  else
  {
    dasom_im_sockets_context = g_main_context_ref (dasom_im_sockets_context);
    dasom_im_sockets_context_ref_count++;
  }

  g_mutex_unlock (&mutex);

  /* when g_main_context_iteration(), iterate only sockets */
  im->sockets_context_source = g_socket_create_source (socket, G_IO_IN, NULL);
  g_source_set_can_recurse (im->sockets_context_source, TRUE);
  g_source_attach (im->sockets_context_source, dasom_im_sockets_context);
  g_source_set_callback (im->sockets_context_source,
                         (GSourceFunc) on_incoming_message,
                         im, NULL);

  im->default_context_source = g_socket_create_source (socket, G_IO_IN, NULL);
  g_source_set_can_recurse (im->default_context_source, TRUE);
  g_source_set_callback (im->default_context_source,
                         (GSourceFunc) on_incoming_message, im, NULL);
  g_source_attach (im->default_context_source, NULL);
}

static void
dasom_im_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomIM *im = DASOM_IM (object);

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

  if (dasom_im_sockets_context)
  {
    g_main_context_unref (dasom_im_sockets_context);
    dasom_im_sockets_context_ref_count--;

    if (dasom_im_sockets_context_ref_count == 0)
      dasom_im_sockets_context = NULL;
  }

  g_mutex_unlock (&mutex);

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
