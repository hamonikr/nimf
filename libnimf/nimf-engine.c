/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-engine.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2019 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-engine.h"
#include "nimf-server.h"
#include "nimf-server-private.h"

struct _NimfEnginePrivate
{
  gchar *surrounding_text;
  gint   surrounding_cursor_index;
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (NimfEngine, nimf_engine, G_TYPE_OBJECT);

/**
 * nimf_engine_reset:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 *
 * Resets the @engine.
 */
void nimf_engine_reset (NimfEngine    *engine,
                        NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->reset)
    class->reset (engine, ic);
}

/**
 * nimf_engine_focus_in:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 *
 * Notifies the language engine that the caller has gained focus.
 */
void
nimf_engine_focus_in (NimfEngine    *engine,
                      NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->focus_in)
    class->focus_in (engine, ic);
}

/**
 * nimf_engine_focus_out:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 *
 * Notifies the language engine that the caller has lost focus.
 */
void nimf_engine_focus_out (NimfEngine    *engine,
                            NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->focus_out)
    class->focus_out (engine, ic);
}

/**
 * nimf_engine_filter_event:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 * @event: a #NimfEvent
 *
 * Let the language engine handle the event.
 *
 * Returns: %TRUE if the language engine consumed the event.
 */
gboolean
nimf_engine_filter_event (NimfEngine    *engine,
                          NimfServiceIC *ic,
                          NimfEvent     *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->filter_event)
    return class->filter_event (engine, ic, event);
  else
    return FALSE;
}

/**
 * nimf_engine_set_surrounding:
 * @engine: a #NimfEngine
 * @text: surrounding text
 * @len: the byte length of @text
 * @cursor_index: the character index of the cursor within @text.
 *
 * Sets surrounding text in @engine. This function is expected to be
 * called in response to nimf_engine_get_surrounding().
 */
void
nimf_engine_set_surrounding (NimfEngine *engine,
                             const char *text,
                             gint        len,
                             gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));
  g_return_if_fail (text != NULL || len == 0);

  g_free (engine->priv->surrounding_text);
  engine->priv->surrounding_text         = g_strndup (text, len);
  engine->priv->surrounding_cursor_index = cursor_index;
}

/**
 * nimf_engine_get_surrounding:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 * @text: (out) (transfer full): location to store surrounding text
 * @cursor_index: (out) (transfer full): location to store cursor index
 *
 * This function internally calls nimf_engine_emit_retrieve_surrounding().
 * Gets surrounding text from the caller of nimf_im_set_surrounding(),
 * if available.
 *
 * Returns: %TRUE if surrounding text is available
 */
gboolean
nimf_engine_get_surrounding (NimfEngine     *engine,
                             NimfServiceIC  *ic,
                             gchar         **text,
                             gint           *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = nimf_engine_emit_retrieve_surrounding (engine, ic);

  if (retval)
  {
    if (engine->priv->surrounding_text)
      *text = g_strdup (engine->priv->surrounding_text);
    else
      *text = g_strdup ("");

    *cursor_index = engine->priv->surrounding_cursor_index;
  }
  else
  {
    *text = NULL;
    *cursor_index = 0;
  }

  return retval;
}

/**
 * nimf_engine_set_method:
 * @engine: a #NimfEngine
 * @method_id: method id
 *
 * The engine may provide multiple input methods. Sets an input method by
 * @method_id
 */
void
nimf_engine_set_method (NimfEngine  *engine,
                        const gchar *method_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->set_method)
    class->set_method (engine, method_id);
}

/**
 * nimf_engine_emit_preedit_start:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 *
 * Emits the #NimfIM::preedit-start signal.
 */
void
nimf_engine_emit_preedit_start (NimfEngine    *engine,
                                NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_service_ic_emit_preedit_start (ic);
}

/**
 * nimf_engine_emit_preedit_changed:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 * @preedit_string: preedit string
 * @attrs: #NimfPreeditAttr array
 * @cursor_pos: cursor position within @preedit_string
 *
 * Emits the #NimfIM::preedit-changed signal.
 */
void
nimf_engine_emit_preedit_changed (NimfEngine       *engine,
                                  NimfServiceIC    *ic,
                                  const gchar      *preedit_string,
                                  NimfPreeditAttr **attrs,
                                  gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_service_ic_emit_preedit_changed (ic, preedit_string, attrs, cursor_pos);
}

/**
 * nimf_engine_emit_preedit_end:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 *
 * Emits the #NimfIM::preedit-end signal.
 */
void
nimf_engine_emit_preedit_end (NimfEngine    *engine,
                              NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_service_ic_emit_preedit_end (ic);
}

/**
 * nimf_engine_emit_commit:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 * @text: text to commit
 *
 * Emits the #NimfIM::commit signal.
 */
void
nimf_engine_emit_commit (NimfEngine    *engine,
                         NimfServiceIC *ic,
                         const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_service_ic_emit_commit (ic, text);
}

/**
 * nimf_engine_emit_delete_surrounding:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 * @offset:  the character offset from the cursor position of the text to be
 *           deleted. A negative value indicates a position before the cursor.
 * @n_chars: the number of characters to be deleted
 *
 * Emits the #NimfIM::delete-surrounding signal.
 */
gboolean
nimf_engine_emit_delete_surrounding (NimfEngine    *engine,
                                     NimfServiceIC *ic,
                                     gint           offset,
                                     gint           n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_service_ic_emit_delete_surrounding (ic, offset, n_chars);
}

/**
 * nimf_engine_emit_retrieve_surrounding:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 *
 * Emits the #NimfIM::retrieve-surrounding signal.
 */
gboolean
nimf_engine_emit_retrieve_surrounding (NimfEngine    *engine,
                                       NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_service_ic_emit_retrieve_surrounding (ic);
}

/**
 * nimf_engine_status_changed:
 * @engine: a #NimfEngine
 *
 * Notifies that the status of the @engine has changed.
 */
void
nimf_engine_status_changed (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_signal_emit_by_name (nimf_server_get_default (), "engine-status-changed",
                         nimf_engine_get_id (engine),
                         nimf_engine_get_icon_name (engine));
}

/**
 * nimf_engine_emit_beep:
 * @engine: a #NimfEngine
 * @ic: a #NimfServiceIC associated with @engine
 *
 * Emits the #NimfIM::beep signal.
 */
void
nimf_engine_emit_beep (NimfEngine    *engine,
                       NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_service_ic_emit_beep (ic);
}

static void
nimf_engine_init (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  engine->priv = nimf_engine_get_instance_private (engine);
}

static void
nimf_engine_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngine *engine = NIMF_ENGINE (object);

  g_free (engine->priv->surrounding_text);

  G_OBJECT_CLASS (nimf_engine_parent_class)->finalize (object);
}

/**
 * nimf_engine_get_id:
 * @engine: #NimfEngine
 *
 * Returns: (transfer none): engine id
 */
const gchar *
nimf_engine_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->get_id)
    return class->get_id (engine);
  else
    g_critical ("You should implement your_engine_get_id ()");

  return NULL;
}

/**
 * nimf_engine_get_candidatable:
 * @engine: #NimfEngine
 *
 * Returns: (transfer none): a #NimfCandidatable
 */
NimfCandidatable *
nimf_engine_get_candidatable (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_server_get_default ()->priv->candidatable;
}

/**
 * nimf_engine_get_icon_name:
 * @engine: #NimfEngine
 *
 * Returns: (transfer none): icon name
 */
const gchar *
nimf_engine_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->get_icon_name)
    return class->get_icon_name (engine);
  else
    g_critical ("You should implement your_engine_get_icon_name ()");

  return NULL;
}

static void
nimf_engine_class_init (NimfEngineClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  G_OBJECT_CLASS (class)->finalize = nimf_engine_finalize;
}
