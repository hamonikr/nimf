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

void nimf_service_ic_emit_preedit_start (NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  im->priv->preedit_state = NIMF_PREEDIT_STATE_START;

  NimfServiceICClass *class  = NIMF_SERVICE_IC_GET_CLASS (im);

  if (class->emit_preedit_start && im->priv->use_preedit)
    class->emit_preedit_start (im);
}

void
nimf_service_ic_emit_preedit_changed (NimfServiceIC    *im,
                                      const gchar      *preedit_string,
                                      NimfPreeditAttr **attrs,
                                      gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  g_free (im->priv->preedit_string);
  nimf_preedit_attr_freev (im->priv->preedit_attrs);

  im->priv->preedit_string     = g_strdup (preedit_string);
  im->priv->preedit_attrs      = nimf_preedit_attrs_copy (attrs);
  im->priv->preedit_cursor_pos = cursor_pos;

  NimfServiceICClass *class  = NIMF_SERVICE_IC_GET_CLASS (im);
  NimfServer         *server = nimf_server_get_default ();

  if (class->emit_preedit_changed && im->priv->use_preedit)
    class->emit_preedit_changed (im, preedit_string, attrs, cursor_pos);

  if (!im->priv->use_preedit &&
      !nimf_candidatable_is_visible (server->candidatable) &&
      strlen (preedit_string))
  {
    nimf_preeditable_set_text (server->preeditable, preedit_string, cursor_pos);
    nimf_preeditable_show (server->preeditable);
  }
  else
  {
    nimf_preeditable_hide (server->preeditable);
  }
}

void
nimf_service_ic_emit_preedit_end (NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  im->priv->preedit_state = NIMF_PREEDIT_STATE_END;

  NimfServiceICClass *class  = NIMF_SERVICE_IC_GET_CLASS (im);
  NimfServer         *server = nimf_server_get_default ();

  if (class->emit_preedit_end && im->priv->use_preedit)
    class->emit_preedit_end (im);

  if (!im->priv->use_preedit)
    nimf_preeditable_hide (server->preeditable);
}

void
nimf_service_ic_emit_commit (NimfServiceIC *im,
                             const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  NimfServiceICClass *class = NIMF_SERVICE_IC_GET_CLASS (im);

  if (class->emit_commit)
    class->emit_commit (im, text);
}

gboolean
nimf_service_ic_emit_retrieve_surrounding (NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return FALSE;

  NimfServiceICClass *class = NIMF_SERVICE_IC_GET_CLASS (im);

  if (class->emit_retrieve_surrounding)
    return class->emit_retrieve_surrounding (im);
  else
    return FALSE;
}

gboolean
nimf_service_ic_emit_delete_surrounding (NimfServiceIC *im,
                                         gint           offset,
                                         gint           n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return FALSE;

  NimfServiceICClass *class = NIMF_SERVICE_IC_GET_CLASS (im);

  if (class->emit_delete_surrounding)
    return class->emit_delete_surrounding (im, offset, n_chars);
  else
    return FALSE;
}

void
nimf_service_ic_engine_changed (NimfServiceIC *im,
                                const gchar   *engine_id,
                                const gchar   *name)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  NimfServer *server = nimf_server_get_default ();

  g_signal_emit_by_name (server, "engine-changed", engine_id, name);
}

void
nimf_service_ic_emit_beep (NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  NimfServiceICClass *class = NIMF_SERVICE_IC_GET_CLASS (im);

  if (class->emit_beep)
    class->emit_beep (im);
}

void nimf_service_ic_focus_in (NimfServiceIC *im)
{
  g_return_if_fail (im != NULL);

  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (im->priv->engine == NULL))
    return;

  NimfServer *server = nimf_server_get_default ();

  nimf_engine_focus_in (im->priv->engine, im);
  server->priv->last_focused_im = im;
  server->priv->last_focused_service = nimf_service_ic_get_service_id (im);
  nimf_service_ic_engine_changed (im, nimf_engine_get_id (im->priv->engine),
                                  nimf_engine_get_icon_name (im->priv->engine));
}

void nimf_service_ic_focus_out (NimfServiceIC *im)
{
  g_return_if_fail (im != NULL);

  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (im->priv->engine == NULL))
    return;

  NimfServer *server = nimf_server_get_default ();

  nimf_engine_focus_out (im->priv->engine, im);

  if (server->priv->last_focused_im == im)
    nimf_service_ic_engine_changed (im, NULL, "nimf-focus-out");

  nimf_preeditable_hide (server->preeditable);
}

static gint
on_comparing_engine_with_id (NimfEngine *engine, const gchar *id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_strcmp0 (nimf_engine_get_id (engine), id);
}

static GList *
nimf_service_ic_create_engines (NimfServiceIC *im)
{
  GList *engines = NULL;
  GHashTableIter iter;
  gpointer       module;
  NimfServer    *server = nimf_server_get_default ();

  g_hash_table_iter_init (&iter, server->priv->modules);

  while (g_hash_table_iter_next (&iter, NULL, &module))
  {
    NimfEngine *engine;
    engine = g_object_new (NIMF_MODULE (module)->type, NULL);
    engines = g_list_prepend (engines, engine);
  }

  return engines;
}

static NimfEngine *
nimf_service_ic_get_instance (NimfServiceIC *im, const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  if (im->priv->engines == NULL)
    im->priv->engines = nimf_service_ic_create_engines (im);

  list = g_list_find_custom (g_list_first (im->priv->engines), engine_id,
                             (GCompareFunc) on_comparing_engine_with_id);
  if (list)
    return list->data;

  return NULL;
}

static NimfEngine *
nimf_service_ic_get_next_instance (NimfServiceIC *im, NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  if (im->priv->engines == NULL)
    im->priv->engines = nimf_service_ic_create_engines (im);

  im->priv->engines = g_list_first (im->priv->engines);
  im->priv->engines = g_list_find  (im->priv->engines, engine);

  list = g_list_next (im->priv->engines);

  if (list == NULL)
    list = g_list_first (im->priv->engines);

  if (list)
  {
    im->priv->engines = list;
    return list->data;
  }

  g_assert (list != NULL);

  return engine;
}

gboolean nimf_service_ic_filter_event (NimfServiceIC *im,
                                       NimfEvent     *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (im != NULL, FALSE);

  if (G_UNLIKELY (im->priv->engine == NULL))
    return FALSE;

  GHashTableIter iter;
  gpointer       trigger_keys;
  gpointer       engine_id;
  NimfServer    *server = nimf_server_get_default ();

  g_hash_table_iter_init (&iter, server->priv->trigger_keys);

  while (g_hash_table_iter_next (&iter, &trigger_keys, &engine_id))
  {
    if (nimf_event_matches (event, trigger_keys))
    {
      if (event->key.type == NIMF_EVENT_KEY_PRESS)
      {
        nimf_service_ic_reset (im);

        if (g_strcmp0 (nimf_engine_get_id (im->priv->engine), engine_id) != 0)
        {
          if (server->priv->use_singleton)
            im->priv->engine = nimf_server_get_instance (server, engine_id);
          else
            im->priv->engine = nimf_service_ic_get_instance (im, engine_id);
        }
        else
        {
          if (server->priv->use_singleton)
            im->priv->engine = nimf_server_get_instance (server, "nimf-system-keyboard");
          else
            im->priv->engine = nimf_service_ic_get_instance (im, "nimf-system-keyboard");
        }

        nimf_service_ic_engine_changed (im, nimf_engine_get_id (im->priv->engine),
                                        nimf_engine_get_icon_name (im->priv->engine));
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
      nimf_service_ic_reset (im);

      if (server->priv->use_singleton)
        im->priv->engine = nimf_server_get_next_instance (server, im->priv->engine);
      else
        im->priv->engine = nimf_service_ic_get_next_instance (im, im->priv->engine);

      nimf_service_ic_engine_changed (im, nimf_engine_get_id (im->priv->engine),
                                      nimf_engine_get_icon_name (im->priv->engine));
    }

    return TRUE;
  }

  return nimf_engine_filter_event (im->priv->engine, im, event);
}

void
nimf_service_ic_set_surrounding (NimfServiceIC *im,
                                 const char    *text,
                                 gint           len,
                                 gint           cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (im != NULL);

  if (G_UNLIKELY (im->priv->engine == NULL))
    return;

  nimf_engine_set_surrounding (im->priv->engine, text, len, cursor_index);
}

void
nimf_service_ic_set_use_preedit (NimfServiceIC *im,
                                 gboolean       use_preedit)
{
  g_debug (G_STRLOC ": %s: %d", G_STRFUNC, use_preedit);

  g_return_if_fail (im != NULL);

  if (im->priv->use_preedit == TRUE && use_preedit == FALSE)
  {
    im->priv->use_preedit = FALSE;

    if (im->priv->preedit_state == NIMF_PREEDIT_STATE_START)
    {
      nimf_service_ic_emit_preedit_changed (im, im->priv->preedit_string,
                                                im->priv->preedit_attrs,
                                                im->priv->preedit_cursor_pos);
      nimf_service_ic_emit_preedit_end (im);
    }
  }
  else if (im->priv->use_preedit == FALSE && use_preedit == TRUE)
  {
    im->priv->use_preedit = TRUE;

    if (im->priv->preedit_string[0] != 0)
    {
      nimf_service_ic_emit_preedit_start   (im);
      nimf_service_ic_emit_preedit_changed (im, im->priv->preedit_string,
                                                im->priv->preedit_attrs,
                                                im->priv->preedit_cursor_pos);
    }
  }
}

gboolean
nimf_service_ic_get_use_preedit (NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return im->priv->use_preedit;
}

void
nimf_service_ic_set_cursor_location (NimfServiceIC       *im,
                                     const NimfRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (im != NULL);

  if (G_UNLIKELY (im->priv->engine == NULL))
    return;

  NimfServer *server = nimf_server_get_default ();
  *im->priv->cursor_area = *area;

  if (!im->priv->use_preedit)
    nimf_preeditable_set_cursor_location (server->preeditable, area);
}

/**
 * nimf_service_ic_get_cursor_location:
 * @im: a #NimfServiceIC
 *
 * Returns: (transfer none): a #NimfRectangle
 */
const NimfRectangle *
nimf_service_ic_get_cursor_location (NimfServiceIC  *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return im->priv->cursor_area;
}

void nimf_service_ic_reset (NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (im != NULL);

  if (G_LIKELY (im->priv->engine))
    nimf_engine_reset (im->priv->engine, im);
}

void
nimf_service_ic_set_engine_by_id (NimfServiceIC *im,
                                  const gchar   *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngine *engine;
  NimfServer *server = nimf_server_get_default ();

  if (server->priv->use_singleton)
    engine = nimf_server_get_instance (server, engine_id);
  else
    engine = nimf_service_ic_get_instance (im, engine_id);

  g_return_if_fail (engine != NULL);

  im->priv->engine = engine;
  nimf_service_ic_engine_changed (im, engine_id,
                                  nimf_engine_get_icon_name (im->priv->engine));
}

void
nimf_service_ic_set_engine (NimfServiceIC *im,
                            const gchar   *engine_id,
                            const gchar   *method_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngine *engine;
  NimfServer *server = nimf_server_get_default ();

  if (server->priv->use_singleton)
    engine = nimf_server_get_instance (server, engine_id);
  else
    engine = nimf_service_ic_get_instance (im, engine_id);

  g_return_if_fail (engine != NULL);

  im->priv->engine = engine;
  nimf_engine_set_method (engine, method_id);
  nimf_service_ic_engine_changed (im, engine_id,
                                  nimf_engine_get_icon_name (im->priv->engine));
}

/**
 * nimf_service_ic_get_service_id:
 * @im: a #NimfServiceIC
 *
 * Returns: (transfer none): a service id
 */
const gchar *
nimf_service_ic_get_service_id (NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceICClass *class = NIMF_SERVICE_IC_GET_CLASS (im);

  if (class->get_service_id)
  {
    return class->get_service_id (im);
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
 * @im: a #NimfServiceIC
 *
 * Returns the associated #NimfEngine instance.
 *
 * Returns: (transfer none): the engine instance
 */
NimfEngine *
nimf_service_ic_get_engine (NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return im->priv->engine;
}

static NimfEngine *
nimf_service_ic_get_default_engine (NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings  *settings;
  gchar      *engine_id;
  NimfEngine *engine;

  settings  = g_settings_new ("org.nimf.engines");
  engine_id = g_settings_get_string (settings, "default-engine");
  engine    = nimf_service_ic_get_instance (im, engine_id);

  if (G_UNLIKELY (engine == NULL))
  {
    g_settings_reset (settings, "default-engine");
    g_free (engine_id);
    engine_id = g_settings_get_string (settings, "default-engine");
    engine = nimf_service_ic_get_instance (im, engine_id);
  }

  g_free (engine_id);
  g_object_unref (settings);

  return engine;
}

static void
nimf_service_ic_init (NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  im->priv = nimf_service_ic_get_instance_private (im);
  im->priv->cursor_area = g_slice_new0 (NimfRectangle);
}

static void
nimf_service_ic_constructed (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIC *im     = NIMF_SERVICE_IC (object);
  NimfServer    *server = nimf_server_get_default ();

  im->priv->use_preedit   = TRUE;
  im->priv->preedit_state = NIMF_PREEDIT_STATE_END;

  if (server->priv->use_singleton)
  {
    im->priv->engine = nimf_server_get_default_engine (server);
  }
  else
  {
    im->priv->engines = nimf_service_ic_create_engines (im);
    im->priv->engine  = nimf_service_ic_get_default_engine (im);
  }

  im->priv->preedit_string     = g_strdup ("");
  im->priv->preedit_attrs      = g_malloc0_n (1, sizeof (NimfPreeditAttr *));
  im->priv->preedit_attrs[0]   = NULL;
  im->priv->preedit_cursor_pos = 0;
}

static void
nimf_service_ic_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIC *im = NIMF_SERVICE_IC (object);

  if (im->priv->engines)
    g_list_free_full (im->priv->engines, g_object_unref);

  g_free                  (im->priv->preedit_string);
  nimf_preedit_attr_freev (im->priv->preedit_attrs);
  g_slice_free            (NimfRectangle, im->priv->cursor_area);

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
