/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-context.c
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

#include "nimf-context.h"
#include <string.h>
#include <X11/Xutil.h>
#include "IMdkit/Xi18n.h"

void
nimf_context_emit_preedit_start (NimfContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (context->type)
  {
    case NIMF_CONTEXT_NIMF_IM:
      if (G_UNLIKELY (context->use_preedit == FALSE &&
                      context->preedit_state == NIMF_PREEDIT_STATE_END))
        return;

      nimf_send_message (context->connection->socket, context->icid,
                         NIMF_MESSAGE_PREEDIT_START, NULL, 0, NULL);
      nimf_result_iteration_until (context->connection->result, NULL,
                                   context->icid,
                                   NIMF_MESSAGE_PREEDIT_START_REPLY);
      context->preedit_state = NIMF_PREEDIT_STATE_START;
      break;
    case NIMF_CONTEXT_XIM:
      {
        XIMS xims = context->cb_user_data;
        IMPreeditStateStruct preedit_state_data = {0};
        preedit_state_data.connect_id = context->xim_connect_id;
        preedit_state_data.icid       = context->icid;
        IMPreeditStart (xims, (XPointer) &preedit_state_data);

        IMPreeditCBStruct preedit_cb_data = {0};
        preedit_cb_data.major_code = XIM_PREEDIT_START;
        preedit_cb_data.connect_id = context->xim_connect_id;
        preedit_cb_data.icid       = context->icid;
        IMCallCallback (xims, (XPointer) &preedit_cb_data);
      }
      break;
    default:
      g_warning ("Unknown type: %d", context->type);
      break;
  }
}

void
nimf_context_emit_preedit_changed (NimfContext *context,
                                   const gchar *preedit_string,
                                   gint         cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (context->type)
  {
    case NIMF_CONTEXT_NIMF_IM:
      if (G_UNLIKELY (context->use_preedit == FALSE &&
                      context->preedit_state == NIMF_PREEDIT_STATE_END))
        return;

      {
        gsize data_len = strlen (preedit_string) + 1 + sizeof (gint);
        gchar *data = g_strndup (preedit_string, data_len - 1);
        *(gint *) (data + data_len - sizeof (gint)) = cursor_pos;

        nimf_send_message (context->connection->socket, context->icid,
                           NIMF_MESSAGE_PREEDIT_CHANGED,
                           data, data_len, g_free);
        nimf_result_iteration_until (context->connection->result, NULL,
                                     context->icid,
                                     NIMF_MESSAGE_PREEDIT_CHANGED_REPLY);
      }
      break;
    case NIMF_CONTEXT_XIM:
      {
        XIMS xims = context->cb_user_data;
        gchar *preedit_string;
        gint   cursor_pos;
        nimf_context_get_preedit_string (context, &preedit_string,
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
        preedit_cb_data.connect_id = context->xim_connect_id;
        preedit_cb_data.icid = context->icid;
        preedit_cb_data.todo.draw.caret = len;
        preedit_cb_data.todo.draw.chg_first = 0;
        preedit_cb_data.todo.draw.chg_length = context->xim_preedit_length;
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

        context->xim_preedit_length = len;
        g_free (feedback);
      }
      break;
    default:
      g_warning ("Unknown type: %d", context->type);
      break;
  }
}

void
nimf_context_emit_preedit_end (NimfContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (context->type)
  {
    case NIMF_CONTEXT_NIMF_IM:
      if (G_UNLIKELY (context->use_preedit == FALSE &&
                      context->preedit_state == NIMF_PREEDIT_STATE_END))
        return;

      nimf_send_message (context->connection->socket, context->icid,
                         NIMF_MESSAGE_PREEDIT_END, NULL, 0, NULL);
      nimf_result_iteration_until (context->connection->result, NULL,
                                   context->icid,
                                   NIMF_MESSAGE_PREEDIT_END_REPLY);
      context->preedit_state = NIMF_PREEDIT_STATE_END;
      break;
    case NIMF_CONTEXT_XIM:
      {
        XIMS xims = context->cb_user_data;
        IMPreeditStateStruct preedit_state_data = {0};
        preedit_state_data.connect_id = context->xim_connect_id;
        preedit_state_data.icid       = context->icid;
        IMPreeditEnd (xims, (XPointer) &preedit_state_data);

        IMPreeditCBStruct preedit_cb_data = {0};
        preedit_cb_data.major_code = XIM_PREEDIT_DONE;
        preedit_cb_data.connect_id = context->xim_connect_id;
        preedit_cb_data.icid       = context->icid;
        IMCallCallback (xims, (XPointer) &preedit_cb_data);
      }
      break;
    default:
      g_warning ("Unknown type: %d", context->type);
      break;
  }
}

void
nimf_context_emit_commit (NimfContext *context,
                          const gchar *text)
{
  g_debug (G_STRLOC ": %s: id = %d", G_STRFUNC, context->icid);

  switch (context->type)
  {
    case NIMF_CONTEXT_NIMF_IM:
      nimf_send_message (context->connection->socket, context->icid,
                         NIMF_MESSAGE_COMMIT,
                         (gchar *) text, strlen (text) + 1, NULL);
      nimf_result_iteration_until (context->connection->result, NULL,
                                   context->icid,
                                   NIMF_MESSAGE_COMMIT_REPLY);
      break;
    case NIMF_CONTEXT_XIM:
      {
        XIMS xims = context->cb_user_data;
        XTextProperty property;
        Xutf8TextListToTextProperty (xims->core.display,
                                     (char **)&text, 1, XCompoundTextStyle,
                                     &property);

        IMCommitStruct commit_data = {0};
        commit_data.major_code = XIM_COMMIT;
        commit_data.connect_id = context->xim_connect_id;
        commit_data.icid       = context->icid;
        commit_data.flag       = XimLookupChars;
        commit_data.commit_string = (gchar *) property.value;
        IMCommitString (xims, (XPointer) &commit_data);

        XFree (property.value);
      }
      break;
    default:
      g_warning ("Unknown type: %d", context->type);
      break;
  }
}

gboolean
nimf_context_emit_retrieve_surrounding (NimfContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_send_message (context->connection->socket, context->icid,
                     NIMF_MESSAGE_RETRIEVE_SURROUNDING, NULL, 0, NULL);
  nimf_result_iteration_until (context->connection->result, NULL,
                               context->icid,
                               NIMF_MESSAGE_RETRIEVE_SURROUNDING_REPLY);

  if (context->connection->result->reply == NULL)
    return FALSE;

  return *(gboolean *) (context->connection->result->reply->data);
}

gboolean
nimf_context_emit_delete_surrounding (NimfContext *context,
                                      gint         offset,
                                      gint         n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gint *data = g_malloc (2 * sizeof (gint));
  data[0] = offset;
  data[1] = n_chars;

  nimf_send_message (context->connection->socket, context->icid,
                     NIMF_MESSAGE_DELETE_SURROUNDING,
                     data, 2 * sizeof (gint), g_free);
  nimf_result_iteration_until (context->connection->result, NULL,
                               context->icid,
                               NIMF_MESSAGE_DELETE_SURROUNDING_REPLY);

  if (context->connection->result->reply == NULL)
    return FALSE;

  return *(gboolean *) (context->connection->result->reply->data);
}

void
nimf_context_emit_engine_changed (NimfContext *context,
                                  const gchar *name)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gpointer       agent;
  GHashTableIter iter;

  g_hash_table_iter_init (&iter, context->server->agents);

  while (g_hash_table_iter_next (&iter, NULL, &agent))
    nimf_send_message (((NimfContext *) agent)->connection->socket,
                       ((NimfContext *) agent)->icid,
                       NIMF_MESSAGE_ENGINE_CHANGED,
                       (gchar *) name, strlen (name) + 1, NULL);
}

void nimf_context_focus_in (NimfContext *context)
{
  g_debug (G_STRLOC ": %s: context icid = %d", G_STRFUNC, context->icid);

  if (G_UNLIKELY (context->engine == NULL))
    return;

  nimf_engine_focus_in (context->engine, context);
  nimf_context_emit_engine_changed (context,
                                    nimf_engine_get_icon_name (context->engine));
}

void nimf_context_focus_out (NimfContext *context)
{
  g_debug (G_STRLOC ": %s: context icid = %d", G_STRFUNC, context->icid);

  if (G_UNLIKELY (context->engine == NULL))
    return;

  nimf_engine_focus_out (context->engine, context);
  nimf_context_emit_engine_changed (context, "nimf-indicator");
}

static void nimf_context_set_next_engine (NimfContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (context->engine == NULL))
    return;

  context->engine = nimf_server_get_next_instance (context->server,
                                                   context->engine);
  nimf_context_emit_engine_changed (context,
                                    nimf_engine_get_icon_name (context->engine));
}

gboolean nimf_context_filter_event (NimfContext *context,
                                    NimfEvent   *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (context->engine == NULL))
    return FALSE;

  GHashTableIter iter;
  gpointer       trigger_keys;
  gpointer       engine;

  g_hash_table_iter_init (&iter, context->server->trigger_keys);

  while (g_hash_table_iter_next (&iter, &trigger_keys, &engine))
  {
    if (nimf_event_matches (event, trigger_keys))
    {
      if (event->key.type == NIMF_EVENT_KEY_PRESS)
      {
        nimf_context_reset (context);

        if (context->engine != engine)
          context->engine = engine;
        else
          context->engine = nimf_server_get_instance (context->server,
                                                      "nimf-system-keyboard");

        nimf_context_emit_engine_changed (context,
                                          nimf_engine_get_icon_name (context->engine));
      }

      return TRUE;
    }
  }

  if (nimf_event_matches (event,
                          (const NimfKey **) context->server->hotkeys))
  {
    if (event->key.type == NIMF_EVENT_KEY_PRESS)
    {
      nimf_context_reset (context);
      nimf_context_set_next_engine (context);
    }

    return TRUE;
  }

  return nimf_engine_filter_event (context->engine, context, event);
}

void
nimf_context_get_preedit_string (NimfContext  *context,
                                 gchar       **str,
                                 gint         *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_LIKELY (context->engine && context->use_preedit == TRUE))
    nimf_engine_get_preedit_string (context->engine, str, cursor_pos);
  else
  {
    if (str)
      *str = g_strdup ("");

    if (cursor_pos)
      *cursor_pos = 0;
  }
}

void
nimf_context_set_surrounding (NimfContext *context,
                              const char  *text,
                              gint         len,
                              gint         cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (context->engine == NULL))
    return;

  nimf_engine_set_surrounding (context->engine, text, len, cursor_index);
}

gboolean
nimf_context_get_surrounding (NimfContext  *context,
                              gchar       **text,
                              gint         *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (context->engine == NULL))
    return FALSE;

  return nimf_engine_get_surrounding (context->engine, context,
                                      text, cursor_index);
}

void
nimf_context_set_use_preedit (NimfContext *context,
                              gboolean     use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (context->use_preedit == TRUE && use_preedit == FALSE)
  {
    context->use_preedit = FALSE;

    if (context->preedit_state == NIMF_PREEDIT_STATE_START)
    {
      gchar *preedit_string;
      gint   cursor_pos;
      nimf_context_get_preedit_string   (context, &preedit_string, &cursor_pos);
      nimf_context_emit_preedit_changed (context, preedit_string, cursor_pos);
      nimf_context_emit_preedit_end     (context);
      g_free (preedit_string);
    }
  }
  else if (context->use_preedit == FALSE && use_preedit == TRUE)
  {
    gchar *preedit_string;
    gint   cursor_pos;

    context->use_preedit = TRUE;

    nimf_context_get_preedit_string (context, &preedit_string, &cursor_pos);

    if (preedit_string[0] != 0)
    {
      nimf_context_emit_preedit_start   (context);
      nimf_context_emit_preedit_changed (context,
                                         preedit_string, cursor_pos);
    }

    g_free (preedit_string);
  }
}

void
nimf_context_set_cursor_location (NimfContext         *context,
                                  const NimfRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (context->engine == NULL))
    return;

  context->cursor_area = *area;
  nimf_engine_set_cursor_location (context->engine, area);
}

void
nimf_context_xim_set_cursor_location (NimfContext *context,
                                      Display     *display)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfRectangle preedit_area = context->cursor_area;

  Window target;

  if (context->focus_window)
    target = context->focus_window;
  else
    target = context->client_window;

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

  nimf_context_set_cursor_location (context, &preedit_area);
}

void nimf_context_reset (NimfContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_LIKELY (context->engine))
    nimf_engine_reset (context->engine, context);
}

void
nimf_context_set_engine_by_id (NimfContext *context,
                               const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngine *engine;

  engine = nimf_server_get_instance (context->server, engine_id);

  g_return_if_fail (engine != NULL);

  context->engine = engine;
  nimf_context_emit_engine_changed (context,
                                    nimf_engine_get_icon_name (context->engine));
}

NimfContext *nimf_context_new (NimfContextType  type,
                               NimfConnection  *connection,
                               NimfServer      *server,
                               gpointer         cb_user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfContext *context;

  context = g_slice_new0 (NimfContext);
  context->type          = type;
  context->connection    = connection;
  context->server        = server;
  context->cb_user_data  = cb_user_data;
  context->engine        = nimf_server_get_default_engine (server);
  context->use_preedit   = TRUE;
  context->preedit_state = NIMF_PREEDIT_STATE_END;

  return context;
}

void nimf_context_free (NimfContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (context->type == NIMF_CONTEXT_NIMF_AGENT)
    g_hash_table_steal (context->server->agents,
                        GUINT_TO_POINTER (context->icid));

  g_slice_free (NimfContext, context);
}
