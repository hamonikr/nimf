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
#include "nimf-enum-types.h"
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
extern GMainContext *nimf_client_sockets_context;

G_DEFINE_TYPE (NimfIM, nimf_im, NIMF_TYPE_CLIENT);

void nimf_im_focus_out (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  NimfClient *client = NIMF_CLIENT (im);

  GSocket *socket = g_socket_connection_get_socket (client->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  nimf_send_message (socket, NIMF_MESSAGE_FOCUS_OUT, NULL, 0, NULL);
  nimf_result_iteration_until (client->result, nimf_client_sockets_context,
                               NIMF_MESSAGE_FOCUS_OUT_REPLY);
}

void nimf_im_set_cursor_location (NimfIM              *im,
                                  const NimfRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  NimfClient *client = NIMF_CLIENT (im);

  GSocket *socket = g_socket_connection_get_socket (client->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  nimf_send_message (socket, NIMF_MESSAGE_SET_CURSOR_LOCATION,
                     (gchar *) area, sizeof (NimfRectangle), NULL);
  nimf_result_iteration_until (client->result, nimf_client_sockets_context,
                               NIMF_MESSAGE_SET_CURSOR_LOCATION_REPLY);
}

void nimf_im_set_use_preedit (NimfIM   *im,
                              gboolean  use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  NimfClient *client = NIMF_CLIENT (im);

  GSocket *socket = g_socket_connection_get_socket (client->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  nimf_send_message (socket, NIMF_MESSAGE_SET_USE_PREEDIT,
                     (gchar *) &use_preedit, sizeof (gboolean), NULL);
  nimf_result_iteration_until (client->result, nimf_client_sockets_context,
                               NIMF_MESSAGE_SET_USE_PREEDIT_REPLY);
}

void nimf_im_set_use_fallback_filter (NimfIM   *im,
                                      gboolean  use_fallback_filter)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  im->use_fallback_filter = use_fallback_filter;
}

gboolean nimf_im_get_surrounding (NimfIM  *im,
                                  gchar  **text,
                                  gint    *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_IM (im), FALSE);

  NimfClient *client = NIMF_CLIENT (im);

  GSocket *socket = g_socket_connection_get_socket (client->connection);
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
  nimf_result_iteration_until (client->result, nimf_client_sockets_context,
                               NIMF_MESSAGE_GET_SURROUNDING_REPLY);

  if (client->result->reply == NULL)
  {
    if (text)
      *text = g_strdup ("");

    if (cursor_index)
      *cursor_index = 0;

    return FALSE;
  }

  if (text)
    *text = g_strndup (client->result->reply->data,
                       client->result->reply->header->data_len - 1 -
                       sizeof (gint) - sizeof (gboolean));

  if (cursor_index)
  {
    *cursor_index = *(gint *) (client->result->reply->data +
                               client->result->reply->header->data_len -
                               sizeof (gint) - sizeof (gboolean));
  }

  return *(gboolean *) (client->result->reply->data - sizeof (gboolean));
}

void nimf_im_set_surrounding (NimfIM     *im,
                              const char *text,
                              gint        len,
                              gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  NimfClient *client = NIMF_CLIENT (im);

  GSocket *socket = g_socket_connection_get_socket (client->connection);
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
  nimf_result_iteration_until (client->result, nimf_client_sockets_context,
                               NIMF_MESSAGE_SET_SURROUNDING_REPLY);
}

void nimf_im_focus_in (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_IM (im));

  NimfClient *client = NIMF_CLIENT (im);

  GSocket *socket = g_socket_connection_get_socket (client->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  nimf_send_message (socket, NIMF_MESSAGE_FOCUS_IN, NULL, 0, NULL);
  nimf_result_iteration_until (client->result, nimf_client_sockets_context,
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

  NimfClient *client = NIMF_CLIENT (im);

  GSocket *socket = g_socket_connection_get_socket (client->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  nimf_send_message (socket, NIMF_MESSAGE_RESET, NULL, 0, NULL);
  nimf_result_iteration_until (client->result, nimf_client_sockets_context,
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

  NimfClient *client = NIMF_CLIENT (im);

  GSocket *socket = g_socket_connection_get_socket (client->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");

    if (im->use_fallback_filter)
      return nimf_im_filter_event_fallback (im, event);
    else
      return FALSE;
  }

  nimf_send_message (socket, NIMF_MESSAGE_FILTER_EVENT, event,
                     sizeof (NimfEvent), NULL);
  nimf_result_iteration_until (client->result, nimf_client_sockets_context,
                               NIMF_MESSAGE_FILTER_EVENT_REPLY);

  if (client->result->reply && *(gboolean *) (client->result->reply->data))
    return TRUE;

  if (im->use_fallback_filter)
    return nimf_im_filter_event_fallback (im, event);
  else
    return FALSE;
}

NimfIM *
nimf_im_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (NIMF_TYPE_IM,
                       "connection-type", NIMF_CONNECTION_NIMF_IM, NULL);
}

static void
nimf_im_init (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  im->preedit_string = g_strdup ("");
  im->use_fallback_filter = TRUE;
}

static void
nimf_im_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_free (NIMF_IM (object)->preedit_string);

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
