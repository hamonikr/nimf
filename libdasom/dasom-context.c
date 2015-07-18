/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-context.c
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

#include "dasom-context.h"
#include "dasom-events.h"
#include "dasom-marshalers.h"
#include "dasom-private.h"
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "IMdkit/IMdkit.h"
#include "IMdkit/Xi18n.h"

enum {
  ENGINE_CHANGED,
  LAST_SIGNAL
};

static guint context_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (DasomContext, dasom_context, G_TYPE_OBJECT);

static void
dasom_context_init (DasomContext *context)
{
  /* FIXME: overflow */
  static gint id = 0;
  id++;
  context->id = id;
  g_debug (G_STRLOC ": %s, id = %d", G_STRFUNC, context->id);

  context->use_preedit = TRUE;
  context->preedit_state = DASOM_PREEDIT_STATE_END;
}

static void
dasom_context_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomContext *context = DASOM_CONTEXT (object);
  dasom_message_unref (context->reply);

  if (context->source)
  {
    g_source_destroy (context->source);
    g_source_unref (context->source);
  }

  if (context->connection)
    g_object_unref (context->connection);

  G_OBJECT_CLASS (dasom_context_parent_class)->finalize (object);
}

static void
dasom_context_class_init (DasomContextClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = dasom_context_finalize;

  context_signals[ENGINE_CHANGED] =
    g_signal_new (g_intern_static_string ("engine-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomContextClass, engine_changed),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

void
on_signal_engine_changed (DasomContext *context,
                          const gchar  *name,
                          gpointer      user_data)
{
  g_debug (G_STRLOC ": %s:%s:id = %d", G_STRFUNC, name, context->id);

  GList *l = context->daemon->agents_list;
  while (l != NULL)
  {
    GList *next = l->next;
    dasom_send_message (DASOM_CONTEXT (l->data)->socket,
                        DASOM_MESSAGE_ENGINE_CHANGED,
                        (gchar *) name, strlen (name) + 1, NULL);
    l = next;
  }
}

DasomContext *
dasom_context_new (DasomConnectionType  type,
                   DasomEngine         *engine,
                   gpointer             cb_user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  DasomContext *context = g_object_new (DASOM_TYPE_CONTEXT, NULL);
  context->type = type;
  context->engine = engine;
  context->cb_user_data = cb_user_data;

  g_signal_connect (context,
                    "engine-changed",
                    G_CALLBACK (on_signal_engine_changed),
                    cb_user_data);

  return context;
}

guint16
dasom_context_get_id (DasomContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_return_val_if_fail (DASOM_IS_CONTEXT (context), 0);

  return context->id;
}

void dasom_context_reset (DasomContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_engine_reset (context->engine);
}

void dasom_context_focus_in (DasomContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->daemon->target = context;
  dasom_engine_focus_in (context->engine);

  g_signal_emit_by_name (context, "engine-changed",
                         dasom_engine_get_name (context->engine));
}

void dasom_context_focus_out (DasomContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_engine_focus_out (context->engine);

  gchar *str = g_strdup ("Dasom");
  g_signal_emit_by_name (context, "engine-changed", str);
  g_free (str);
  context->daemon->target = NULL;
}

void dasom_context_set_next_engine (DasomContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->engine = dasom_daemon_get_next_instance (context->daemon, context->engine);
  g_signal_emit_by_name (context, "engine-changed",
                         dasom_engine_get_name (context->engine));
}

gboolean dasom_context_filter_event (DasomContext     *context,
                                     const DasomEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (dasom_event_is_hotkey (event, (const gchar * const *) context->daemon->hotkey_names))
  {
    if (event->key.type == DASOM_EVENT_KEY_RELEASE)
    {
      dasom_context_reset (context);
      dasom_context_set_next_engine (context);
    }

    return TRUE;
  }

  return dasom_engine_filter_event (context->engine, event);
}

void
dasom_context_get_preedit_string (DasomContext  *context,
                                  gchar        **str,
                                  gint          *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_LIKELY (context->use_preedit == TRUE))
    dasom_engine_get_preedit_string (context->engine,
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
dasom_context_set_surrounding (DasomContext *context,
                               const char   *text,
                               gint          len,
                               gint          cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_engine_set_surrounding (context->engine, text, len, cursor_index);
}

gboolean
dasom_context_get_surrounding (DasomContext  *context,
                               gchar        **text,
                               gint          *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return dasom_engine_get_surrounding (context->engine, text, cursor_index);
}

void
dasom_context_set_cursor_location (DasomContext         *context,
                                   const DasomRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_engine_set_cursor_location (context->engine, area);
}

void
dasom_context_set_use_preedit (DasomContext *context,
                               gboolean      use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (context->use_preedit == TRUE && use_preedit == FALSE)
  {
    context->use_preedit = FALSE;

    if (context->preedit_state == DASOM_PREEDIT_STATE_START)
    {
      dasom_context_emit_preedit_changed (context);
      dasom_context_emit_preedit_end (context);
    }
  }
  else if (context->use_preedit == FALSE && use_preedit == TRUE)
  {
    gchar *str = NULL;
    gint   cursor_pos;

    context->use_preedit = TRUE;

    dasom_context_get_preedit_string (context, &str, &cursor_pos);

    if (*str != 0)
    {
      dasom_context_emit_preedit_start (context);
      dasom_context_emit_preedit_changed (context);
    }

    g_free (str);
  }
}

void
dasom_context_iteration_until (DasomContext     *context,
                               DasomMessageType  type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean is_dispatched;

  do {
    is_dispatched = g_main_context_iteration (NULL, TRUE);
  } while (!is_dispatched || (context->reply && (context->reply->header->type != type)));

  if (G_UNLIKELY (context->reply == NULL))
  {
    g_critical (G_STRLOC ": %s:Can't receive %s", G_STRFUNC,
                dasom_message_get_name_by_type (type));
    return;
  }

  if (context->reply->header->type != type)
  {
    const gchar *name = dasom_message_get_name (context->reply);
    gchar *mesg;

    if (name)
      mesg = g_strdup (name);
    else
      mesg = g_strdup_printf ("unknown type %d", context->reply->header->type);

    g_critical ("Reply type does not match.\n"
                "%s is required, but we received %s\n",
                dasom_message_get_name_by_type (type), mesg);
    g_free (mesg);
  }
}

void
dasom_context_emit_preedit_start (DasomContext *context)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (context->use_preedit == FALSE &&
                  context->preedit_state == DASOM_PREEDIT_STATE_END))
    return;

  dasom_send_message (context->socket, DASOM_MESSAGE_PREEDIT_START,
                      NULL, 0, NULL);
  dasom_context_iteration_until (context, DASOM_MESSAGE_PREEDIT_START_REPLY);
  context->preedit_state = DASOM_PREEDIT_STATE_START;
}

void
dasom_context_emit_preedit_changed (DasomContext *context)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (context->use_preedit == FALSE &&
                  context->preedit_state == DASOM_PREEDIT_STATE_END))
    return;

  dasom_send_message (context->socket, DASOM_MESSAGE_PREEDIT_CHANGED,
                      NULL, 0, NULL);
  dasom_context_iteration_until (context, DASOM_MESSAGE_PREEDIT_CHANGED_REPLY);
}

void
dasom_context_emit_preedit_end (DasomContext *context)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (context->use_preedit == FALSE &&
                  context->preedit_state == DASOM_PREEDIT_STATE_END))
    return;

  dasom_send_message (context->socket, DASOM_MESSAGE_PREEDIT_END,
                      NULL, 0, NULL);
  dasom_context_iteration_until (context, DASOM_MESSAGE_PREEDIT_END_REPLY);
  context->preedit_state = DASOM_PREEDIT_STATE_END;
}

void
dasom_context_emit_commit (DasomContext *context,
                           const gchar  *text)
{
  g_debug (G_STRLOC ": %s:%s:id = %d", G_STRFUNC, text, context->id);

  switch (context->type)
  {
    case DASOM_CONNECTION_DASOM_IM:
      dasom_send_message (context->socket, DASOM_MESSAGE_COMMIT,
                          (gchar *) text, strlen (text) + 1, NULL);
      dasom_context_iteration_until (context, DASOM_MESSAGE_COMMIT_REPLY);
      break;
    case DASOM_CONNECTION_XIM:
      {
        XIMS xims = context->cb_user_data;
        XTextProperty property;
        /* TODO: gdk 소스를 확인해봅시다. utf8인가... 문자열 복사할 때
         * 문자열 내에 널 값이 있을 수 있다는 영어를 어디선가 본 것 같은데
         * 정확하게 확인해봅시다 */
        Xutf8TextListToTextProperty (xims->core.display,
                                     (char **)&text, 1, XCompoundTextStyle,
                                     &property);

        IMCommitStruct commit_data;
        commit_data.major_code = XIM_COMMIT;
        commit_data.minor_code = 0;
        commit_data.connect_id = context->xim_connect_id;
        commit_data.icid       = context->id;
        commit_data.flag       = XimLookupChars;
        /* commit_data.keysym = ??; */
        commit_data.commit_string = (gchar *) property.value;
        IMCommitString (xims, (XPointer) &commit_data);

        XFree (property.value);
      }
      break;
    default:
      g_warning ("Unknown type: %d", context->type);
      break;
  }

  g_debug (G_STRLOC ":EXIT: %s", G_STRFUNC);
}

gboolean
dasom_context_emit_retrieve_surrounding (DasomContext *context)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_send_message (context->socket, DASOM_MESSAGE_RETRIEVE_SURROUNDING,
                      NULL, 0, NULL);
  dasom_context_iteration_until (context,
                                 DASOM_MESSAGE_RETRIEVE_SURROUNDING_REPLY);

  if (context->reply == NULL)
    return FALSE;

  return *(gboolean *) (context->reply->data);
}

gboolean
dasom_context_emit_delete_surrounding (DasomContext *context,
                                       gint          offset,
                                       gint          n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gint *data = g_malloc (2 * sizeof (gint));
  data[0] = offset;
  data[1] = n_chars;

  dasom_send_message (context->socket, DASOM_MESSAGE_DELETE_SURROUNDING,
                      data, 2 * sizeof (gint), g_free);
  dasom_context_iteration_until (context,
                                 DASOM_MESSAGE_DELETE_SURROUNDING_REPLY);

  if (context->reply == NULL)
    return FALSE;

  return *(gboolean *) (context->reply->data);
}
