/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-connection.c
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

#include "dasom-connection.h"
#include "dasom-events.h"
#include "dasom-marshalers.h"
#include "dasom-private.h"
#include <string.h>
#include <X11/Xutil.h>
#include "IMdkit/Xi18n.h"

enum {
  ENGINE_CHANGED,
  LAST_SIGNAL
};

static guint connection_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (DasomConnection, dasom_connection, G_TYPE_OBJECT);

static void
dasom_connection_init (DasomConnection *connection)
{
}

static void
dasom_connection_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomConnection *connection = DASOM_CONNECTION (object);
  dasom_message_unref (connection->reply);

  if (connection->source)
  {
    g_source_destroy (connection->source);
    g_source_unref   (connection->source);
  }

  if (connection->socket_connection)
    g_object_unref (connection->socket_connection);

  G_OBJECT_CLASS (dasom_connection_parent_class)->finalize (object);
}

static void
dasom_connection_class_init (DasomConnectionClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->finalize = dasom_connection_finalize;

  connection_signals[ENGINE_CHANGED] =
    g_signal_new (g_intern_static_string ("engine-changed"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomConnectionClass, engine_changed),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

void
on_signal_engine_changed (DasomConnection *connection,
                          const gchar     *name,
                          gpointer         user_data)
{
  g_debug (G_STRLOC ": %s: %s: connection id = %d", G_STRFUNC,
           name, connection->id);

  GList *l = connection->server->agents_list;
  while (l != NULL)
  {
    GList *next = l->next;
    dasom_send_message (DASOM_CONNECTION (l->data)->socket,
                        DASOM_MESSAGE_ENGINE_CHANGED,
                        (gchar *) name, strlen (name) + 1, NULL);
    l = next;
  }
}

DasomConnection *
dasom_connection_new (DasomConnectionType  type,
                      DasomEngine         *engine,
                      gpointer             cb_user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomConnection *connection = g_object_new (DASOM_TYPE_CONNECTION, NULL);

  connection->type            = type;
  connection->engine          = engine;
  connection->use_preedit     = TRUE;
  connection->preedit_state   = DASOM_PREEDIT_STATE_END;
  connection->is_english_mode = TRUE;
  connection->cb_user_data    = cb_user_data;

  g_signal_connect (connection,
                    "engine-changed",
                    G_CALLBACK (on_signal_engine_changed),
                    NULL);
  return connection;
}

guint16
dasom_connection_get_id (DasomConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (DASOM_IS_CONNECTION (connection), 0);

  return connection->id;
}

void dasom_connection_reset (DasomConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_engine_reset (connection->engine, connection);
}

void dasom_connection_focus_in (DasomConnection *connection)
{
  g_debug (G_STRLOC ": %s: connection id = %d", G_STRFUNC, connection->id);

  dasom_engine_focus_in (connection->engine);

  g_signal_emit_by_name (connection, "engine-changed",
                         dasom_engine_get_name (connection->engine));
}

void dasom_connection_focus_out (DasomConnection *connection)
{
  g_debug (G_STRLOC ": %s: connection id = %d", G_STRFUNC, connection->id);

  dasom_engine_focus_out (connection->engine, connection);

  /* FIXME: in case of "", g_socket_recieve() returns any number of bytes, up
   * to size, I don't know the reason.
   * Maybe there is something wrong somewhere. */
  g_signal_emit_by_name (connection, "engine-changed", "focus-out");
}

void dasom_connection_set_next_engine (DasomConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  connection->engine = dasom_server_get_next_instance (connection->server,
                                                       connection->engine);
  g_signal_emit_by_name (connection, "engine-changed",
                         dasom_engine_get_name (connection->engine));
}

gboolean dasom_connection_filter_event (DasomConnection *connection,
                                        DasomEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (dasom_event_matches (event,
                           (const DasomKey **) connection->server->hotkeys))
  {
    if (event->key.type == DASOM_EVENT_KEY_RELEASE)
    {
      dasom_connection_reset (connection);
      dasom_connection_set_next_engine (connection);
    }

    return TRUE;
  }

  return dasom_engine_filter_event (connection->engine, connection, event);
}

void
dasom_connection_get_preedit_string (DasomConnection  *connection,
                                     gchar           **str,
                                     gint             *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_LIKELY (connection->use_preedit == TRUE))
    dasom_engine_get_preedit_string (connection->engine,
                                     str,
                                     cursor_pos);
  else
  {
    if (str)
      *str = g_strdup ("");

    if (cursor_pos)
      *cursor_pos = 0;
  }
}

void
dasom_connection_set_surrounding (DasomConnection *connection,
                                  const char      *text,
                                  gint             len,
                                  gint             cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_engine_set_surrounding (connection->engine, text, len, cursor_index);
}

gboolean
dasom_connection_get_surrounding (DasomConnection  *connection,
                                  gchar           **text,
                                  gint             *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return dasom_engine_get_surrounding (connection->engine, connection,
                                       text, cursor_index);
}

void
dasom_connection_set_cursor_location (DasomConnection      *connection,
                                      const DasomRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  connection->cursor_area = *area;
  dasom_engine_set_cursor_location (connection->engine, area);
}

void
dasom_connection_set_use_preedit (DasomConnection *connection,
                                  gboolean         use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (connection->use_preedit == TRUE && use_preedit == FALSE)
  {
    connection->use_preedit = FALSE;

    if (connection->preedit_state == DASOM_PREEDIT_STATE_START)
    {
      dasom_connection_emit_preedit_changed (connection);
      dasom_connection_emit_preedit_end     (connection);
    }
  }
  else if (connection->use_preedit == FALSE && use_preedit == TRUE)
  {
    gchar *str = NULL;
    gint   cursor_pos;

    connection->use_preedit = TRUE;

    dasom_connection_get_preedit_string (connection, &str, &cursor_pos);

    if (*str != 0)
    {
      dasom_connection_emit_preedit_start   (connection);
      dasom_connection_emit_preedit_changed (connection);
    }

    g_free (str);
  }
}

void
dasom_connection_iteration_until (DasomConnection  *connection,
                                  DasomMessageType  type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  do {
    connection->is_dispatched = FALSE;
    g_main_context_iteration (NULL, TRUE);
  } while ((connection->is_dispatched == FALSE) ||
           (connection->reply && (connection->reply->header->type != type)));

  connection->is_dispatched = FALSE;

  if (G_UNLIKELY (connection->reply == NULL))
  {
    g_critical (G_STRLOC ": %s:Can't receive %s", G_STRFUNC,
                dasom_message_get_name_by_type (type));
    return;
  }
}

void
dasom_connection_emit_preedit_start (DasomConnection *connection)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (connection->type)
  {
    case DASOM_CONNECTION_DASOM_IM:
      if (G_UNLIKELY (connection->use_preedit == FALSE &&
                      connection->preedit_state == DASOM_PREEDIT_STATE_END))
        return;

      dasom_send_message (connection->socket, DASOM_MESSAGE_PREEDIT_START,
                          NULL, 0, NULL);
      dasom_connection_iteration_until (connection,
                                        DASOM_MESSAGE_PREEDIT_START_REPLY);
      connection->preedit_state = DASOM_PREEDIT_STATE_START;
      break;
    case DASOM_CONNECTION_XIM:
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
dasom_connection_emit_preedit_changed (DasomConnection *connection)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (connection->type)
  {
    case DASOM_CONNECTION_DASOM_IM:
      if (G_UNLIKELY (connection->use_preedit == FALSE &&
                      connection->preedit_state == DASOM_PREEDIT_STATE_END))
        return;

      dasom_send_message (connection->socket, DASOM_MESSAGE_PREEDIT_CHANGED,
                          NULL, 0, NULL);
      dasom_connection_iteration_until (connection,
                                        DASOM_MESSAGE_PREEDIT_CHANGED_REPLY);
      break;
    case DASOM_CONNECTION_XIM:
      {
        XIMS xims = connection->cb_user_data;
        gchar *preedit_string;
        gint   cursor_pos;
        dasom_connection_get_preedit_string (connection, &preedit_string,
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
dasom_connection_emit_preedit_end (DasomConnection *connection)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (connection->type)
  {
    case DASOM_CONNECTION_DASOM_IM:
      if (G_UNLIKELY (connection->use_preedit == FALSE &&
                      connection->preedit_state == DASOM_PREEDIT_STATE_END))
        return;

      dasom_send_message (connection->socket, DASOM_MESSAGE_PREEDIT_END,
                          NULL, 0, NULL);
      dasom_connection_iteration_until (connection,
                                        DASOM_MESSAGE_PREEDIT_END_REPLY);
      connection->preedit_state = DASOM_PREEDIT_STATE_END;
      break;
    case DASOM_CONNECTION_XIM:
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
dasom_connection_emit_commit (DasomConnection *connection,
                              const gchar     *text)
{
  g_debug (G_STRLOC ": %s: id = %d", G_STRFUNC, connection->id);

  switch (connection->type)
  {
    case DASOM_CONNECTION_DASOM_IM:
      dasom_send_message (connection->socket, DASOM_MESSAGE_COMMIT,
                          (gchar *) text, strlen (text) + 1, NULL);
      dasom_connection_iteration_until (connection,
                                        DASOM_MESSAGE_COMMIT_REPLY);
      break;
    case DASOM_CONNECTION_XIM:
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
dasom_connection_emit_retrieve_surrounding (DasomConnection *connection)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_send_message (connection->socket, DASOM_MESSAGE_RETRIEVE_SURROUNDING,
                      NULL, 0, NULL);
  dasom_connection_iteration_until (connection,
                                    DASOM_MESSAGE_RETRIEVE_SURROUNDING_REPLY);

  if (connection->reply == NULL)
    return FALSE;

  return *(gboolean *) (connection->reply->data);
}

gboolean
dasom_connection_emit_delete_surrounding (DasomConnection *connection,
                                          gint             offset,
                                          gint             n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gint *data = g_malloc (2 * sizeof (gint));
  data[0] = offset;
  data[1] = n_chars;

  dasom_send_message (connection->socket, DASOM_MESSAGE_DELETE_SURROUNDING,
                      data, 2 * sizeof (gint), g_free);
  dasom_connection_iteration_until (connection,
                                    DASOM_MESSAGE_DELETE_SURROUNDING_REPLY);

  if (connection->reply == NULL)
    return FALSE;

  return *(gboolean *) (connection->reply->data);
}

void
dasom_connection_xim_set_cursor_location (DasomConnection *connection,
                                          XIMS             xims)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomRectangle preedit_area = connection->cursor_area;

  Window target;

  if (connection->focus_window)
    target = connection->focus_window;
  else
    target = connection->client_window;

  if (target)
  {
    XWindowAttributes xwa;
    Window child;

    XGetWindowAttributes (xims->core.display, target, &xwa);
    XTranslateCoordinates (xims->core.display, target,
                           xwa.root,
                           preedit_area.x,
                           preedit_area.y,
                           &preedit_area.x,
                           &preedit_area.y,
                           &child);
  }

  dasom_connection_set_cursor_location (connection, &preedit_area);
}
