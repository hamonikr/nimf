/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-connection.c
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

#include "nimf-connection.h"
#include "nimf-events.h"
#include "nimf-marshalers.h"
#include "nimf-private.h"
#include <string.h>
#include <X11/Xutil.h>
#include "IMdkit/Xi18n.h"

G_DEFINE_TYPE (NimfConnection, nimf_connection, G_TYPE_OBJECT);

static void
nimf_connection_init (NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  connection->result = g_slice_new0 (NimfResult);
}

static void
nimf_connection_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfConnection *connection = NIMF_CONNECTION (object);
  nimf_message_unref (connection->result->reply);

  if (connection->source)
  {
    g_source_destroy (connection->source);
    g_source_unref   (connection->source);
  }

  if (connection->socket_connection)
    g_object_unref (connection->socket_connection);

  g_slice_free (NimfResult, connection->result);

  G_OBJECT_CLASS (nimf_connection_parent_class)->finalize (object);
}

static void
nimf_connection_class_init (NimfConnectionClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->finalize = nimf_connection_finalize;
}

NimfConnection *
nimf_connection_new (NimfConnectionType  type,
                     NimfEngine         *engine,
                     gpointer            cb_user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfConnection *connection = g_object_new (NIMF_TYPE_CONNECTION, NULL);

  connection->type            = type;
  connection->engine          = engine;
  connection->use_preedit     = TRUE;
  connection->preedit_state   = NIMF_PREEDIT_STATE_END;
  connection->is_english_mode = TRUE;
  connection->cb_user_data    = cb_user_data;

  return connection;
}

guint16
nimf_connection_get_id (NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_CONNECTION (connection), 0);

  return connection->id;
}

void nimf_connection_reset (NimfConnection *connection,
                            guint16         client_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_LIKELY (connection->engine))
    nimf_engine_reset (connection->engine, connection, client_id);
}

void nimf_connection_focus_in (NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s: connection id = %d", G_STRFUNC, connection->id);

  if (G_UNLIKELY (connection->engine == NULL))
    return;

  nimf_engine_focus_in (connection->engine);
  nimf_connection_emit_engine_changed (connection, nimf_engine_get_icon_name (connection->engine));
}

void nimf_connection_focus_out (NimfConnection *connection,
                                guint16         client_id)
{
  g_debug (G_STRLOC ": %s: connection id = %d", G_STRFUNC, connection->id);

  if (G_UNLIKELY (connection->engine == NULL))
    return;

  nimf_engine_focus_out (connection->engine, connection, client_id);
  nimf_connection_emit_engine_changed (connection, "focus-out");
}

void nimf_connection_set_next_engine (NimfConnection *connection,
                                      guint16         client_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (connection->engine == NULL))
    return;

  connection->engine = nimf_server_get_next_instance (connection->server,
                                                      connection->engine);
  nimf_connection_emit_engine_changed (connection, nimf_engine_get_icon_name (connection->engine));
}

void
nimf_connection_set_engine_by_id (NimfConnection *connection,
                                  const gchar    *id,
                                  gboolean        is_english_mode)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  connection->engine = nimf_server_get_instance (connection->server, id);

  if (G_UNLIKELY (connection->engine == NULL))
    return;

  connection->is_english_mode = is_english_mode;
  nimf_engine_set_english_mode (connection->engine,
                                connection->is_english_mode);
  nimf_connection_emit_engine_changed (connection, nimf_engine_get_icon_name (connection->engine));
}

gboolean nimf_connection_filter_event (NimfConnection *connection,
                                       guint16         client_id,
                                       NimfEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (connection->engine == NULL))
    return FALSE;

  if (nimf_event_matches (event,
                          (const NimfKey **) connection->server->hotkeys))
  {
    if (event->key.type == NIMF_EVENT_KEY_RELEASE)
    {
      nimf_connection_reset (connection, client_id);
      nimf_connection_set_next_engine (connection, client_id);
    }

    return TRUE;
  }

  return nimf_engine_filter_event (connection->engine, connection,
                                   client_id, event);
}

void
nimf_connection_get_preedit_string (NimfConnection  *connection,
                                    gchar          **str,
                                    gint            *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_LIKELY (connection->engine && connection->use_preedit == TRUE))
    nimf_engine_get_preedit_string (connection->engine, str, cursor_pos);
  else
  {
    if (str)
      *str = g_strdup ("");

    if (cursor_pos)
      *cursor_pos = 0;
  }
}

void
nimf_connection_set_surrounding (NimfConnection *connection,
                                 const char     *text,
                                 gint            len,
                                 gint            cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (connection->engine == NULL))
    return;

  nimf_engine_set_surrounding (connection->engine, text, len, cursor_index);
}

gboolean
nimf_connection_get_surrounding (NimfConnection  *connection,
                                 guint16          client_id,
                                 gchar          **text,
                                 gint            *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (connection->engine == NULL))
    return FALSE;

  return nimf_engine_get_surrounding (connection->engine, connection,
                                      client_id, text, cursor_index);
}

void
nimf_connection_set_cursor_location (NimfConnection      *connection,
                                     const NimfRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (connection->engine == NULL))
    return;

  connection->cursor_area = *area;
  nimf_engine_set_cursor_location (connection->engine, area);
}

void
nimf_connection_set_use_preedit (NimfConnection *connection,
                                 guint16         client_id,
                                 gboolean        use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (connection->use_preedit == TRUE && use_preedit == FALSE)
  {
    connection->use_preedit = FALSE;

    if (connection->preedit_state == NIMF_PREEDIT_STATE_START)
    {
      gchar *preedit_string;
      gint   cursor_pos;
      nimf_connection_get_preedit_string   (connection,
                                            &preedit_string,
                                            &cursor_pos);
      nimf_connection_emit_preedit_changed (connection, client_id,
                                            preedit_string, cursor_pos);
      nimf_connection_emit_preedit_end     (connection, client_id);
      g_free (preedit_string);
    }
  }
  else if (connection->use_preedit == FALSE && use_preedit == TRUE)
  {
    gchar *preedit_string;
    gint   cursor_pos;

    connection->use_preedit = TRUE;

    nimf_connection_get_preedit_string (connection,
                                        &preedit_string,
                                        &cursor_pos);
    if (preedit_string[0] != 0)
    {
      nimf_connection_emit_preedit_start   (connection, client_id);
      nimf_connection_emit_preedit_changed (connection, client_id,
                                            preedit_string, cursor_pos);
    }

    g_free (preedit_string);
  }
}

void
nimf_connection_emit_preedit_start (NimfConnection *connection,
                                    guint16         client_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (connection->type)
  {
    case NIMF_CONNECTION_NIMF_IM:
      if (G_UNLIKELY (connection->use_preedit == FALSE &&
                      connection->preedit_state == NIMF_PREEDIT_STATE_END))
        return;

      nimf_send_message (connection->socket, client_id,
                         NIMF_MESSAGE_PREEDIT_START, NULL, 0, NULL);
      nimf_result_iteration_until (connection->result, NULL, client_id,
                                   NIMF_MESSAGE_PREEDIT_START_REPLY);
      connection->preedit_state = NIMF_PREEDIT_STATE_START;
      break;
    case NIMF_CONNECTION_XIM:
      {
        XIMS xims = connection->cb_user_data;
        IMPreeditStateStruct preedit_state_data = {0};
        preedit_state_data.connect_id = connection->xim_connect_id;
        preedit_state_data.icid       = connection->id;
        IMPreeditStart (xims, (XPointer) &preedit_state_data);

        IMPreeditCBStruct preedit_cb_data = {0};
        preedit_cb_data.major_code = XIM_PREEDIT_START;
        preedit_cb_data.connect_id = connection->xim_connect_id;
        preedit_cb_data.icid       = connection->id;
        IMCallCallback (xims, (XPointer) &preedit_cb_data);
      }
      break;
    default:
      g_warning ("Unknown type: %d", connection->type);
      break;
  }
}

void
nimf_connection_emit_preedit_changed (NimfConnection *connection,
                                      guint16         client_id,
                                      const gchar    *preedit_string,
                                      gint            cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (connection->type)
  {
    case NIMF_CONNECTION_NIMF_IM:
      if (G_UNLIKELY (connection->use_preedit == FALSE &&
                      connection->preedit_state == NIMF_PREEDIT_STATE_END))
        return;

      {
        gsize data_len = strlen (preedit_string) + 1 + sizeof (gint);
        gchar *data = g_strndup (preedit_string, data_len - 1);
        *(gint *) (data + data_len - sizeof (gint)) = cursor_pos;

        nimf_send_message (connection->socket, client_id,
                           NIMF_MESSAGE_PREEDIT_CHANGED,
                           data, data_len, g_free);
        nimf_result_iteration_until (connection->result, NULL, client_id,
                                     NIMF_MESSAGE_PREEDIT_CHANGED_REPLY);
      }
      break;
    case NIMF_CONNECTION_XIM:
      {
        XIMS xims = connection->cb_user_data;
        gchar *preedit_string;
        gint   cursor_pos;
        nimf_connection_get_preedit_string (connection, &preedit_string,
                                            &cursor_pos);
        IMPreeditCBStruct preedit_cb_data = {0};
        XIMText           text;
        XTextProperty     text_property;

        static XIMFeedback *feedback;
        gint i, len;

        if (preedit_string == NULL)
          return;

        len = g_utf8_strlen (preedit_string, -1);

        feedback = g_malloc (sizeof (XIMFeedback) * (len + 1));

        for (i = 0; i < len; i++)
          feedback[i] = XIMUnderline;

        feedback[len] = 0;

        preedit_cb_data.major_code = XIM_PREEDIT_DRAW;
        preedit_cb_data.connect_id = connection->xim_connect_id;
        preedit_cb_data.icid = connection->id;
        preedit_cb_data.todo.draw.caret = len;
        preedit_cb_data.todo.draw.chg_first = 0;
        preedit_cb_data.todo.draw.chg_length = connection->xim_preedit_length;
        preedit_cb_data.todo.draw.text = &text;

        text.feedback = feedback;

        if (len > 0)
        {
          Xutf8TextListToTextProperty (xims->core.display,
                                       (char **) &preedit_string, 1,
                                       XCompoundTextStyle, &text_property);
          text.encoding_is_wchar = 0;
          text.length = strlen ((char *) text_property.value);
          text.string.multi_byte = (char *) text_property.value;
          IMCallCallback (xims, (XPointer) &preedit_cb_data);
          XFree (text_property.value);
        }
        else
        {
          text.encoding_is_wchar = 0;
          text.length = 0;
          text.string.multi_byte = "";
          IMCallCallback (xims, (XPointer) &preedit_cb_data);
          len = 0;
        }

        connection->xim_preedit_length = len;
        g_free (feedback);
      }
      break;
    default:
      g_warning ("Unknown type: %d", connection->type);
      break;
  }
}

void
nimf_connection_emit_preedit_end (NimfConnection *connection,
                                  guint16         client_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (connection->type)
  {
    case NIMF_CONNECTION_NIMF_IM:
      if (G_UNLIKELY (connection->use_preedit == FALSE &&
                      connection->preedit_state == NIMF_PREEDIT_STATE_END))
        return;

      nimf_send_message (connection->socket, client_id,
                         NIMF_MESSAGE_PREEDIT_END, NULL, 0, NULL);
      nimf_result_iteration_until (connection->result, NULL, client_id,
                                   NIMF_MESSAGE_PREEDIT_END_REPLY);
      connection->preedit_state = NIMF_PREEDIT_STATE_END;
      break;
    case NIMF_CONNECTION_XIM:
      {
        XIMS xims = connection->cb_user_data;
        IMPreeditStateStruct preedit_state_data = {0};
        preedit_state_data.connect_id = connection->xim_connect_id;
        preedit_state_data.icid       = connection->id;
        IMPreeditEnd (xims, (XPointer) &preedit_state_data);

        IMPreeditCBStruct preedit_cb_data = {0};
        preedit_cb_data.major_code = XIM_PREEDIT_DONE;
        preedit_cb_data.connect_id = connection->xim_connect_id;
        preedit_cb_data.icid       = connection->id;
        IMCallCallback (xims, (XPointer) &preedit_cb_data);
      }
      break;
    default:
      g_warning ("Unknown type: %d", connection->type);
      break;
  }
}

void
nimf_connection_emit_commit (NimfConnection *connection,
                             guint16         client_id,
                             const gchar    *text)
{
  g_debug (G_STRLOC ": %s: id = %d", G_STRFUNC, connection->id);

  switch (connection->type)
  {
    case NIMF_CONNECTION_NIMF_IM:
      nimf_send_message (connection->socket, client_id, NIMF_MESSAGE_COMMIT,
                         (gchar *) text, strlen (text) + 1, NULL);
      nimf_result_iteration_until (connection->result, NULL, client_id,
                                   NIMF_MESSAGE_COMMIT_REPLY);
      break;
    case NIMF_CONNECTION_XIM:
      {
        XIMS xims = connection->cb_user_data;
        XTextProperty property;
        Xutf8TextListToTextProperty (xims->core.display,
                                     (char **)&text, 1, XCompoundTextStyle,
                                     &property);

        IMCommitStruct commit_data = {0};
        commit_data.major_code = XIM_COMMIT;
        commit_data.connect_id = connection->xim_connect_id;
        commit_data.icid       = connection->id;
        commit_data.flag       = XimLookupChars;
        commit_data.commit_string = (gchar *) property.value;
        IMCommitString (xims, (XPointer) &commit_data);

        XFree (property.value);
      }
      break;
    default:
      g_warning ("Unknown type: %d", connection->type);
      break;
  }
}

gboolean
nimf_connection_emit_retrieve_surrounding (NimfConnection *connection,
                                           guint16         client_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_send_message (connection->socket, client_id,
                     NIMF_MESSAGE_RETRIEVE_SURROUNDING, NULL, 0, NULL);
  nimf_result_iteration_until (connection->result, NULL, client_id,
                               NIMF_MESSAGE_RETRIEVE_SURROUNDING_REPLY);

  if (connection->result->reply == NULL)
    return FALSE;

  return *(gboolean *) (connection->result->reply->data);
}

gboolean
nimf_connection_emit_delete_surrounding (NimfConnection *connection,
                                         guint16         client_id,
                                         gint            offset,
                                         gint            n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gint *data = g_malloc (2 * sizeof (gint));
  data[0] = offset;
  data[1] = n_chars;

  nimf_send_message (connection->socket, client_id,
                     NIMF_MESSAGE_DELETE_SURROUNDING,
                     data, 2 * sizeof (gint), g_free);
  nimf_result_iteration_until (connection->result, NULL, client_id,
                               NIMF_MESSAGE_DELETE_SURROUNDING_REPLY);

  if (connection->result->reply == NULL)
    return FALSE;

  return *(gboolean *) (connection->result->reply->data);
}

void
nimf_connection_emit_engine_changed (NimfConnection *connection,
                                     const gchar    *name)
{
  g_debug (G_STRLOC ": %s: %s: connection id = %d", G_STRFUNC,
           name, connection->id);

  GList *l = connection->server->agents_list;
  while (l != NULL)
  {
    GList *next = l->next;
    nimf_send_message (NIMF_CONNECTION (l->data)->socket, 0,
                       NIMF_MESSAGE_ENGINE_CHANGED,
                       (gchar *) name, strlen (name) + 1, NULL);
    l = next;
  }
}

void
nimf_connection_xim_set_cursor_location (NimfConnection *connection,
                                         Display        *display)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfRectangle preedit_area = connection->cursor_area;

  Window target;

  if (connection->focus_window)
    target = connection->focus_window;
  else
    target = connection->client_window;

  if (target)
  {
    XWindowAttributes xwa;
    Window child;

    XGetWindowAttributes (display, target, &xwa);
    XTranslateCoordinates (display, target,
                           xwa.root,
                           preedit_area.x,
                           preedit_area.y,
                           &preedit_area.x,
                           &preedit_area.y,
                           &child);
  }

  nimf_connection_set_cursor_location (connection, &preedit_area);
}
