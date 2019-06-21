/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-service-ic.c
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

#include "nimf-service-ic.h"
#include "nimf-module-private.h"
#include <string.h>
#include "nimf-preeditable.h"
#include "nimf-key-syms.h"
#include "nimf-server-private.h"
#include "nimf-server.h"
#include "nimf-service.h"

struct _NimfServiceICPrivate
{
  NimfEngine       *engine;
  GList            *engines;
  gboolean          use_preedit;
  NimfRectangle    *cursor_area;
  /* preedit */
  NimfPreeditState  preedit_state;
  gchar            *preedit_string;
  NimfPreeditAttr **preedit_attrs;
  gint              preedit_cursor_pos;
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (NimfServiceIC, nimf_service_ic, G_TYPE_OBJECT);

/**
 * nimf_service_ic_emit_preedit_start:
 * @ic: a #NimfServiceIC
 *
 * Emits a #NimfIM::preedit-start signal.
 */
void nimf_service_ic_emit_preedit_start (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!ic))
    return;

  ic->priv->preedit_state = NIMF_PREEDIT_STATE_START;

  NimfServiceICClass *class  = NIMF_SERVICE_IC_GET_CLASS (ic);

  if (class->emit_preedit_start && ic->priv->use_preedit)
    class->emit_preedit_start (ic);
}

/**
 * nimf_service_ic_emit_preedit_changed:
 * @ic: a #NimfServiceIC
 * @preedit_string: preedit string
 * @attrs: an array of #NimfPreeditAttr
 * @cursor_pos: cursor position
 *
 * Emits a #NimfIM::preedit-changed signal.
 */
void
nimf_service_ic_emit_preedit_changed (NimfServiceIC    *ic,
                                      const gchar      *preedit_string,
                                      NimfPreeditAttr **attrs,
                                      gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!ic))
    return;

  g_free (ic->priv->preedit_string);
  nimf_preedit_attr_freev (ic->priv->preedit_attrs);

  ic->priv->preedit_string     = g_strdup (preedit_string);
  ic->priv->preedit_attrs      = nimf_preedit_attrs_copy (attrs);
  ic->priv->preedit_cursor_pos = cursor_pos;

  NimfServiceICClass *class  = NIMF_SERVICE_IC_GET_CLASS (ic);
  NimfServer         *server = nimf_server_get_default ();

  if (class->emit_preedit_changed && ic->priv->use_preedit)
    class->emit_preedit_changed (ic, preedit_string, attrs, cursor_pos);

  if (!ic->priv->use_preedit &&
      !nimf_candidatable_is_visible (server->priv->candidatable) &&
      strlen (preedit_string))
  {
    nimf_preeditable_set_text (server->priv->preeditable,
                               preedit_string, cursor_pos);
    nimf_preeditable_show (server->priv->preeditable);
  }
  else
  {
    nimf_preeditable_hide (server->priv->preeditable);
  }
}

/**
 * nimf_service_ic_emit_preedit_end:
 * @ic: a #NimfServiceIC
 *
 * Emits a #NimfIM::preedit-end signal.
 */
void
nimf_service_ic_emit_preedit_end (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!ic))
    return;

  ic->priv->preedit_state = NIMF_PREEDIT_STATE_END;

  NimfServiceICClass *class  = NIMF_SERVICE_IC_GET_CLASS (ic);
  NimfServer         *server = nimf_server_get_default ();

  if (class->emit_preedit_end && ic->priv->use_preedit)
    class->emit_preedit_end (ic);

  if (!ic->priv->use_preedit)
    nimf_preeditable_hide (server->priv->preeditable);
}

/**
 * nimf_service_ic_emit_commit:
 * @ic: a #NimfServiceIC
 * @text: text to commit
 *
 * Emits a #NimfIM::commit signal.
 */
void
nimf_service_ic_emit_commit (NimfServiceIC *ic,
                             const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!ic))
    return;

  NimfServiceICClass *class = NIMF_SERVICE_IC_GET_CLASS (ic);

  if (class->emit_commit)
    class->emit_commit (ic, text);
}

/**
 * nimf_service_ic_emit_retrieve_surrounding:
 * @ic: a #NimfServiceIC
 *
 * Emits a #NimfIM::retrieve-surrounding signal.
 */
gboolean
nimf_service_ic_emit_retrieve_surrounding (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!ic))
    return FALSE;

  NimfServiceICClass *class = NIMF_SERVICE_IC_GET_CLASS (ic);

  if (class->emit_retrieve_surrounding)
    return class->emit_retrieve_surrounding (ic);
  else
    return FALSE;
}

/**
 * nimf_service_ic_emit_delete_surrounding:
 * @ic: a #NimfServiceIC
 * @offset:  the character offset from the cursor position of the text to be
 *           deleted. A negative value indicates a position before the cursor.
 * @n_chars: the number of characters to be deleted
 *
 * Emits a #NimfIM::delete-surrounding signal.
 *
 * Returns: %TRUE if the signal was handled.
 */
gboolean
nimf_service_ic_emit_delete_surrounding (NimfServiceIC *ic,
                                         gint           offset,
                                         gint           n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!ic))
    return FALSE;

  NimfServiceICClass *class = NIMF_SERVICE_IC_GET_CLASS (ic);

  if (class->emit_delete_surrounding)
    return class->emit_delete_surrounding (ic, offset, n_chars);
  else
    return FALSE;
}

/**
 * nimf_service_ic_engine_changed:
 * @ic: a #NimfServiceIC
 * @engine_id: engine id
 * @name: name
 */
void
nimf_service_ic_engine_changed (NimfServiceIC *ic,
                                const gchar   *engine_id,
                                const gchar   *name)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!ic))
    return;

  NimfServer *server = nimf_server_get_default ();

  g_signal_emit_by_name (server, "engine-changed", engine_id, name);
}

/**
 * nimf_service_ic_emit_beep:
 * @ic: a #NimfServiceIC
 *
 * Emits a #NimfIM::beep signal.
 */
void
nimf_service_ic_emit_beep (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!ic))
    return;

  NimfServiceICClass *class = NIMF_SERVICE_IC_GET_CLASS (ic);

  if (class->emit_beep)
    class->emit_beep (ic);
}

/**
 * nimf_service_ic_focus_in:
 * @ic: a #NimfServiceIC
 */
void
nimf_service_ic_focus_in (NimfServiceIC *ic)
{
  g_return_if_fail (ic != NULL);

  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (ic->priv->engine == NULL))
    return;

  NimfServer *server = nimf_server_get_default ();

  nimf_engine_focus_in (ic->priv->engine, ic);
  server->priv->last_focused_im = ic;
  server->priv->last_focused_service = nimf_service_ic_get_service_id (ic);
  nimf_service_ic_engine_changed (ic, nimf_engine_get_id (ic->priv->engine),
                                  nimf_engine_get_icon_name (ic->priv->engine));
}

/**
 * nimf_service_ic_focus_out:
 * @ic: a #NimfServiceIC
 */
void
nimf_service_ic_focus_out (NimfServiceIC *ic)
{
  g_return_if_fail (ic != NULL);

  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (ic->priv->engine == NULL))
    return;

  NimfServer *server = nimf_server_get_default ();

  nimf_engine_focus_out (ic->priv->engine, ic);

  if (server->priv->last_focused_im == ic)
    nimf_service_ic_engine_changed (ic, NULL, "nimf-focus-out");

  nimf_preeditable_hide (server->priv->preeditable);
}

static gint
on_comparing_engine_with_id (NimfEngine *engine, const gchar *id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_strcmp0 (nimf_engine_get_id (engine), id);
}

static GList *
nimf_service_ic_load_engines (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *engines = NULL;
  NimfServer *server;
  GList *l;

  server = nimf_server_get_default ();

  for (l = server->priv->engines; l != NULL; l = l->next)
  {
    NimfModule  *module;
    const gchar *engine_id;

    engine_id = nimf_engine_get_id (NIMF_ENGINE (l->data));
    module = g_hash_table_lookup (server->priv->modules, engine_id);
    engines = g_list_prepend (engines, g_object_new (module->type, NULL));
  }

  return engines;
}

static NimfEngine *
nimf_service_ic_get_engine_by_id (NimfServiceIC *ic, const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  if (ic->priv->engines == NULL)
    ic->priv->engines = nimf_service_ic_load_engines (ic);

  list = g_list_find_custom (ic->priv->engines, engine_id,
                             (GCompareFunc) on_comparing_engine_with_id);
  if (list)
    return list->data;

  return NULL;
}

static NimfEngine *
nimf_service_ic_get_next_engine (NimfServiceIC *ic, NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  if (ic->priv->engines == NULL)
    ic->priv->engines = nimf_service_ic_load_engines (ic);

  list = g_list_find (ic->priv->engines, engine);

  if (list == NULL)
    list = g_list_find_custom (ic->priv->engines, nimf_engine_get_id (engine),
                               (GCompareFunc) on_comparing_engine_with_id);

  list = g_list_next (list);

  if (list == NULL)
    list = ic->priv->engines;

  return list->data;
}

/**
 * nimf_service_ic_filter_event:
 * @ic: a #NimfServiceIC
 * @event: a #NimfEvent
 *
 * Returns: TRUE if the event is consumed.
 */
gboolean
nimf_service_ic_filter_event (NimfServiceIC *ic,
                              NimfEvent     *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (ic != NULL, FALSE);

  if (G_UNLIKELY (ic->priv->engine == NULL))
    return FALSE;

  GHashTableIter iter;
  NimfShortcut  *shortcut;
  gpointer       engine_id;
  const gchar   *engine_id0 = nimf_engine_get_id (ic->priv->engine);
  NimfServer    *server = nimf_server_get_default ();

  g_hash_table_iter_init (&iter, server->priv->shortcuts);

  while (g_hash_table_iter_next (&iter, &engine_id, (gpointer) &shortcut))
  {
    if ((shortcut->to_lang &&
         nimf_event_matches (event, (const NimfKey **) shortcut->to_lang) &&
         g_strcmp0 (engine_id0, engine_id)))
    {
      if (event->key.type == NIMF_EVENT_KEY_PRESS)
      {
        nimf_service_ic_reset (ic);
        nimf_service_ic_change_engine_by_id (ic, engine_id);
      }

      if (event->key.keyval == NIMF_KEY_Escape)
        return FALSE;

      return TRUE;
    }

    if ((shortcut->to_sys &&
         nimf_event_matches (event, (const NimfKey **) shortcut->to_sys) &&
         !g_strcmp0 (engine_id0, engine_id)))
    {
      if (event->key.type == NIMF_EVENT_KEY_PRESS)
      {
        nimf_service_ic_reset (ic);
        nimf_service_ic_change_engine_by_id (ic, "nimf-system-keyboard");
      }

      if (event->key.keyval == NIMF_KEY_Escape)
        return FALSE;

      return TRUE;
    }
  }

  if (nimf_event_matches (event, (const NimfKey **) server->priv->hotkeys))
  {
    if (event->key.type == NIMF_EVENT_KEY_PRESS)
    {
      nimf_service_ic_reset (ic);

      if (server->priv->use_singleton)
        ic->priv->engine = nimf_server_get_next_engine (server, ic->priv->engine);
      else
        ic->priv->engine = nimf_service_ic_get_next_engine (ic, ic->priv->engine);

      nimf_service_ic_engine_changed (ic, nimf_engine_get_id (ic->priv->engine),
                                      nimf_engine_get_icon_name (ic->priv->engine));
    }

    return TRUE;
  }

  return nimf_engine_filter_event (ic->priv->engine, ic, event);
}

/**
 * nimf_service_ic_set_surrounding:
 * @ic: a #NimfServiceIC
 * @text: text
 * @len: length
 * @cursor_index: cursor index
 */
void
nimf_service_ic_set_surrounding (NimfServiceIC *ic,
                                 const char    *text,
                                 gint           len,
                                 gint           cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (ic != NULL);

  if (G_UNLIKELY (ic->priv->engine == NULL))
    return;

  nimf_engine_set_surrounding (ic->priv->engine, text, len, cursor_index);
}

/**
 * nimf_service_ic_set_use_preedit:
 * @ic: a #NimfServiceIC
 * @use_preedit: whether the input method should use an on-the-spot input style
 *
 * If @use_preedit is %FALSE (default is %TRUE), then the input method may use
 * some other input styles, such as over-the-spot, off-the-spot or root-window.
 */
void
nimf_service_ic_set_use_preedit (NimfServiceIC *ic,
                                 gboolean       use_preedit)
{
  g_debug (G_STRLOC ": %s: %d", G_STRFUNC, use_preedit);

  g_return_if_fail (ic != NULL);

  if (ic->priv->use_preedit == TRUE && use_preedit == FALSE)
  {
    ic->priv->use_preedit = FALSE;

    if (ic->priv->preedit_state == NIMF_PREEDIT_STATE_START)
    {
      nimf_service_ic_emit_preedit_changed (ic, ic->priv->preedit_string,
                                                ic->priv->preedit_attrs,
                                                ic->priv->preedit_cursor_pos);
      nimf_service_ic_emit_preedit_end (ic);
    }
  }
  else if (ic->priv->use_preedit == FALSE && use_preedit == TRUE)
  {
    ic->priv->use_preedit = TRUE;

    if (ic->priv->preedit_string[0] != 0)
    {
      nimf_service_ic_emit_preedit_start   (ic);
      nimf_service_ic_emit_preedit_changed (ic, ic->priv->preedit_string,
                                                ic->priv->preedit_attrs,
                                                ic->priv->preedit_cursor_pos);
    }
  }
}

/**
 * nimf_service_ic_get_use_preedit:
 * @ic: a #NimfServiceIC
 *
 * Returns: TRUE if an on-the-spot input style is used
 */
gboolean
nimf_service_ic_get_use_preedit (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return ic->priv->use_preedit;
}

/**
 * nimf_service_ic_set_cursor_location:
 * @ic: a #NimfServiceIC
 * @area: a #NimfRectangle
 *
 * Notifies the service @ic that a change in cursor position has been made. The
 * location is the position of a window position in root window coordinates.
 */
void
nimf_service_ic_set_cursor_location (NimfServiceIC       *ic,
                                     const NimfRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (ic != NULL);

  if (G_UNLIKELY (ic->priv->engine == NULL))
    return;

  NimfServer *server = nimf_server_get_default ();
  *ic->priv->cursor_area = *area;

  if (!ic->priv->use_preedit)
    nimf_preeditable_set_cursor_location (server->priv->preeditable, area);
}

/**
 * nimf_service_ic_get_cursor_location:
 * @ic: a #NimfServiceIC
 *
 * Returns: (transfer none): a #NimfRectangle
 */
const NimfRectangle *
nimf_service_ic_get_cursor_location (NimfServiceIC  *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return ic->priv->cursor_area;
}

/**
 * nimf_service_ic_reset:
 * @ic: a #NimfServiceIC
 */
void
nimf_service_ic_reset (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (ic != NULL);

  if (G_LIKELY (ic->priv->engine))
    nimf_engine_reset (ic->priv->engine, ic);
}

/**
 * nimf_service_ic_change_engine_by_id:
 * @ic: a #NimfServiceIC
 * @engine_id: engine id
 *
 * Changes the engine by engine id.
 */
void
nimf_service_ic_change_engine_by_id (NimfServiceIC *ic,
                                     const gchar   *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngine *engine;
  NimfServer *server = nimf_server_get_default ();

  if (server->priv->use_singleton)
    engine = nimf_server_get_engine_by_id (server, engine_id);
  else
    engine = nimf_service_ic_get_engine_by_id (ic, engine_id);

  g_return_if_fail (engine != NULL);

  ic->priv->engine = engine;
  nimf_service_ic_engine_changed (ic, engine_id,
                                  nimf_engine_get_icon_name (ic->priv->engine));
}

/**
 * nimf_service_ic_change_engine:
 * @ic: a #NimfServiceIC
 * @engine_id: engine id
 * @method_id: method id
 *
 * Changes the engine by engine id and method id.
 */
void
nimf_service_ic_change_engine (NimfServiceIC *ic,
                               const gchar   *engine_id,
                               const gchar   *method_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngine *engine;
  NimfServer *server = nimf_server_get_default ();

  if (server->priv->use_singleton)
    engine = nimf_server_get_engine_by_id (server, engine_id);
  else
    engine = nimf_service_ic_get_engine_by_id (ic, engine_id);

  g_return_if_fail (engine != NULL);

  ic->priv->engine = engine;
  nimf_engine_set_method (engine, method_id);
  nimf_service_ic_engine_changed (ic, engine_id,
                                  nimf_engine_get_icon_name (ic->priv->engine));
}

/**
 * nimf_service_ic_get_service_id:
 * @ic: a #NimfServiceIC
 *
 * Returns: (transfer none): a service id
 */
const gchar *
nimf_service_ic_get_service_id (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceICClass *class = NIMF_SERVICE_IC_GET_CLASS (ic);

  if (class->get_service_id)
  {
    return class->get_service_id (ic);
  }
  else
  {
    g_critical (G_STRLOC ": %s: You should implement your_get_service_id ()",
                G_STRFUNC);
    return NULL;
  }
}

/**
 * nimf_service_ic_get_engine:
 * @ic: a #NimfServiceIC
 *
 * Returns the associated #NimfEngine instance.
 *
 * Returns: (transfer none): the engine instance
 */
NimfEngine *
nimf_service_ic_get_engine (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return ic->priv->engine;
}

static NimfEngine *
nimf_service_ic_get_default_engine (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings  *settings;
  gchar      *engine_id;
  NimfEngine *engine;

  settings  = g_settings_new ("org.nimf.engines");
  engine_id = g_settings_get_string (settings, "default-engine");
  engine    = nimf_service_ic_get_engine_by_id (ic, engine_id);

  if (G_UNLIKELY (engine == NULL))
  {
    g_settings_reset (settings, "default-engine");
    g_free (engine_id);
    engine_id = g_settings_get_string (settings, "default-engine");
    engine = nimf_service_ic_get_engine_by_id (ic, engine_id);
  }

  g_free (engine_id);
  g_object_unref (settings);

  return engine;
}

static void
nimf_service_ic_init (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  ic->priv = nimf_service_ic_get_instance_private (ic);
  ic->priv->cursor_area = g_slice_new0 (NimfRectangle);
}

void
nimf_service_ic_load_engine (NimfServiceIC *ic,
                             const gchar   *engine_id,
                             NimfServer    *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (ic->priv->engines)
  {
    if (server->priv->use_singleton)
    {
      NimfServer *server = nimf_server_get_default ();

      if (g_list_find (server->priv->engines, ic->priv->engine) == NULL)
      {
        const gchar *id = nimf_engine_get_id (ic->priv->engine);
        ic->priv->engine = nimf_server_get_engine_by_id (server, id);
      }

      g_list_free_full (ic->priv->engines, g_object_unref);
      ic->priv->engines = NULL;
    }
    else
    {
      NimfModule *module;
      NimfEngine *engine;

      module = g_hash_table_lookup (server->priv->modules, engine_id);
      engine = g_object_new (module->type, NULL);
      ic->priv->engines = g_list_prepend (ic->priv->engines, engine);
    }
  }
}

void
nimf_service_ic_unload_engine (NimfServiceIC *ic,
                               const gchar   *engine_id,
                               NimfEngine    *signleton_engine_to_be_deleted,
                               NimfServer    *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (ic->priv->engines)
  {
    GList *list;
    list = g_list_find_custom (ic->priv->engines, engine_id,
                               (GCompareFunc) on_comparing_engine_with_id);
    if (list)
    {
      if (ic->priv->engine == list->data)
      {
        if (server->priv->use_singleton)
          ic->priv->engine = nimf_server_get_default_engine (server);
        else
          ic->priv->engine = nimf_service_ic_get_default_engine (ic);
      }

      g_object_unref (list->data);
      ic->priv->engines = g_list_delete_link (ic->priv->engines, list);
    }
  }
  else
  {
    if (ic->priv->engine == signleton_engine_to_be_deleted)
    {
      if (server->priv->use_singleton)
        ic->priv->engine = nimf_server_get_default_engine (server);
      else
        ic->priv->engine = nimf_service_ic_get_default_engine (ic);
    }
  }
}

static void
nimf_service_ic_constructed (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIC *ic     = NIMF_SERVICE_IC (object);
  NimfServer    *server = nimf_server_get_default ();

  ic->priv->use_preedit   = TRUE;
  ic->priv->preedit_state = NIMF_PREEDIT_STATE_END;

  if (server->priv->use_singleton)
  {
    ic->priv->engine = nimf_server_get_default_engine (server);
  }
  else
  {
    ic->priv->engines = nimf_service_ic_load_engines (ic);
    ic->priv->engine  = nimf_service_ic_get_default_engine (ic);
  }

  ic->priv->preedit_string     = g_strdup ("");
  ic->priv->preedit_attrs      = g_malloc0_n (1, sizeof (NimfPreeditAttr *));
  ic->priv->preedit_attrs[0]   = NULL;
  ic->priv->preedit_cursor_pos = 0;

  g_ptr_array_add (server->priv->ics, ic);
}

static void
nimf_service_ic_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIC *ic     = NIMF_SERVICE_IC (object);
  NimfServer    *server = nimf_server_get_default ();

  g_ptr_array_remove_fast (server->priv->ics, ic);

  if (ic->priv->engines)
    g_list_free_full (ic->priv->engines, g_object_unref);

  g_free                  (ic->priv->preedit_string);
  nimf_preedit_attr_freev (ic->priv->preedit_attrs);
  g_slice_free            (NimfRectangle, ic->priv->cursor_area);

  G_OBJECT_CLASS (nimf_service_ic_parent_class)->finalize (object);
}

static void
nimf_service_ic_class_init (NimfServiceICClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize    = nimf_service_ic_finalize;
  object_class->constructed = nimf_service_ic_constructed;
}
