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

enum {
  PREEDIT_START,
  PREEDIT_END,
  PREEDIT_CHANGED,
  COMMIT,
  RETRIEVE_SURROUNDING,
  DELETE_SURROUNDING,
  ENGINE_CHANGED,
  LAST_SIGNAL
};

static guint context_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (DasomContext, dasom_context, G_TYPE_OBJECT);

static void
dasom_context_init (DasomContext *context)
{
  static gint id = 0;
  id++;
  context->id = id;
  g_debug (G_STRLOC ": %s, id = %d", G_STRFUNC, context->id);

  context->commit_text = g_strdup ("");
}

static void
dasom_context_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomContext *context = DASOM_CONTEXT (object);
  g_free (context->commit_text);
  dasom_message_free (context->reply);

  G_OBJECT_CLASS (dasom_context_parent_class)->finalize (object);
}

static void
dasom_context_class_init (DasomContextClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = dasom_context_finalize;

  context_signals[PREEDIT_START] =
    g_signal_new (g_intern_static_string ("preedit-start"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomContextClass, preedit_start),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  context_signals[PREEDIT_END] =
    g_signal_new (g_intern_static_string ("preedit-end"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomContextClass, preedit_end),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  context_signals[PREEDIT_CHANGED] =
    g_signal_new (g_intern_static_string ("preedit-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomContextClass, preedit_changed),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  context_signals[COMMIT] =
    g_signal_new (g_intern_static_string ("commit"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomContextClass, commit),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  context_signals[RETRIEVE_SURROUNDING] =
    g_signal_new (g_intern_static_string ("retrieve-surrounding"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomContextClass, retrieve_surrounding),
                  g_signal_accumulator_true_handled, NULL,
                  dasom_cclosure_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);

  context_signals[DELETE_SURROUNDING] =
    g_signal_new (g_intern_static_string ("delete-surrounding"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomContextClass, delete_surrounding),
                  g_signal_accumulator_true_handled, NULL,
                  dasom_cclosure_marshal_BOOLEAN__INT_INT,
                  G_TYPE_BOOLEAN, 2,
                  G_TYPE_INT,
                  G_TYPE_INT);

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

DasomContext *
dasom_context_new (DasomConnectionType  type,
                   DasomEngine         *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  DasomContext *context = g_object_new (DASOM_TYPE_CONTEXT, NULL);
  context->type = type;
  context->engine = engine;

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

  context->engine->priv->caller = context;
  dasom_engine_reset (context->engine);
}

void dasom_context_focus_in (DasomContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->engine->priv->caller = context;
  dasom_engine_focus_in (context->engine);

  g_signal_emit_by_name (context, "engine-changed",
                         dasom_engine_get_name (context->engine));
}

void dasom_context_focus_out (DasomContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar *str = g_strdup ("Dasom");

  context->engine->priv->caller = context;

  dasom_engine_focus_out (context->engine);

  g_signal_emit_by_name (context, "engine-changed", str);

  g_free (str);
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

  context->engine->priv->caller = context;

  if (dasom_event_is_hotkey (event, (const gchar * const *) context->daemon->hotkey_names))
  {
    if (event->key.type == DASOM_KEY_RELEASE)
    {
      dasom_context_reset (context);
      dasom_context_set_next_engine (context);
    }
  }

  return dasom_engine_filter_event (context->engine, event);
}

void
dasom_context_get_preedit_string (DasomContext  *context,
                                  gchar        **str,
                                  gint          *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->engine->priv->caller = context;
  dasom_engine_get_preedit_string (context->engine,
                                   str,
                                   cursor_pos);
}
