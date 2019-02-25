/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-service-im.c
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

#include "nimf-service-im.h"
#include "nimf-module-private.h"
#include <string.h>
#include "nimf-preeditable.h"
#include "nimf-key-syms.h"
#include "nimf-server-private.h"
#include "nimf-server.h"
#include "nimf-service.h"

struct _NimfServiceIMPrivate
{
  NimfEngine *engine;
  GList      *engines;
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (NimfServiceIM, nimf_service_im, G_TYPE_OBJECT);

void nimf_service_im_emit_preedit_start (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  NimfServiceIMClass *class  = NIMF_SERVICE_IM_GET_CLASS (im);
  NimfServer         *server = nimf_server_get_default ();

  if (class->emit_preedit_start)
    class->emit_preedit_start (im);

  if (!im->use_preedit)
    nimf_preeditable_show (server->preeditable);
}

void
nimf_service_im_emit_preedit_changed (NimfServiceIM    *im,
                                      const gchar      *preedit_string,
                                      NimfPreeditAttr **attrs,
                                      gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  g_free (im->preedit_string);
  nimf_preedit_attr_freev (im->preedit_attrs);

  im->preedit_string = g_strdup (preedit_string);
  im->preedit_attrs = nimf_preedit_attrs_copy (attrs);
  im->preedit_cursor_pos = cursor_pos;

  NimfServiceIMClass *class  = NIMF_SERVICE_IM_GET_CLASS (im);
  NimfServer         *server = nimf_server_get_default ();

  if (class->emit_preedit_changed)
    class->emit_preedit_changed (im, preedit_string, attrs, cursor_pos);

  if (!im->use_preedit)
    nimf_preeditable_set_text (server->preeditable, preedit_string);
}

void
nimf_service_im_emit_preedit_end (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  NimfServiceIMClass *class  = NIMF_SERVICE_IM_GET_CLASS (im);
  NimfServer         *server = nimf_server_get_default ();

  if (class->emit_preedit_end)
    class->emit_preedit_end (im);

  if (!im->use_preedit)
    nimf_preeditable_hide (server->preeditable);
}

void
nimf_service_im_emit_commit (NimfServiceIM *im,
                             const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  NimfServiceIMClass *class = NIMF_SERVICE_IM_GET_CLASS (im);

  if (class->emit_commit)
    class->emit_commit (im, text);
}

gboolean
nimf_service_im_emit_retrieve_surrounding (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return FALSE;

  NimfServiceIMClass *class = NIMF_SERVICE_IM_GET_CLASS (im);

  if (class->emit_retrieve_surrounding)
    return class->emit_retrieve_surrounding (im);
  else
    return FALSE;
}

gboolean
nimf_service_im_emit_delete_surrounding (NimfServiceIM *im,
                                         gint           offset,
                                         gint           n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return FALSE;

  NimfServiceIMClass *class = NIMF_SERVICE_IM_GET_CLASS (im);

  if (class->emit_delete_surrounding)
    return class->emit_delete_surrounding (im, offset, n_chars);
  else
    return FALSE;
}

void
nimf_service_im_engine_changed (NimfServiceIM *im,
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
nimf_service_im_emit_beep (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  NimfServiceIMClass *class = NIMF_SERVICE_IM_GET_CLASS (im);

  if (class->emit_beep)
    class->emit_beep (im);
}

void nimf_service_im_focus_in (NimfServiceIM *im)
{
  g_return_if_fail (im != NULL);

  g_debug (G_STRLOC ": %s: im icid = %d", G_STRFUNC, im->icid);

  if (G_UNLIKELY (im->priv->engine == NULL))
    return;

  NimfServer *server = nimf_server_get_default ();

  nimf_engine_focus_in (im->priv->engine, im);
  server->priv->last_focused_im = im;
  server->priv->last_focused_service = nimf_service_im_get_service_id (im);
  nimf_service_im_engine_changed (im, nimf_engine_get_id (im->priv->engine),
                                  nimf_engine_get_icon_name (im->priv->engine));
}

void nimf_service_im_focus_out (NimfServiceIM *im)
{
  g_return_if_fail (im != NULL);

  g_debug (G_STRLOC ": %s: im icid = %d", G_STRFUNC, im->icid);

  if (G_UNLIKELY (im->priv->engine == NULL))
    return;

  NimfServer *server = nimf_server_get_default ();

  nimf_engine_focus_out (im->priv->engine, im);

  if (server->priv->last_focused_im == im)
    nimf_service_im_engine_changed (im, NULL, "nimf-focus-out");

  nimf_preeditable_hide (server->preeditable);
}

static gint
on_comparing_engine_with_id (NimfEngine *engine, const gchar *id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_strcmp0 (nimf_engine_get_id (engine), id);
}

static GList *
nimf_service_im_create_engines (NimfServiceIM *im)
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
nimf_service_im_get_instance (NimfServiceIM *im, const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  if (im->priv->engines == NULL)
    im->priv->engines = nimf_service_im_create_engines (im);

  list = g_list_find_custom (g_list_first (im->priv->engines), engine_id,
                             (GCompareFunc) on_comparing_engine_with_id);
  if (list)
    return list->data;

  return NULL;
}

static NimfEngine *
nimf_service_im_get_next_instance (NimfServiceIM *im, NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  if (im->priv->engines == NULL)
    im->priv->engines = nimf_service_im_create_engines (im);

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

gboolean nimf_service_im_filter_event (NimfServiceIM *im,
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
        nimf_service_im_reset (im);

        if (g_strcmp0 (nimf_engine_get_id (im->priv->engine), engine_id) != 0)
        {
          if (server->priv->use_singleton)
            im->priv->engine = nimf_server_get_instance (server, engine_id);
          else
            im->priv->engine = nimf_service_im_get_instance (im, engine_id);
        }
        else
        {
          if (server->priv->use_singleton)
            im->priv->engine = nimf_server_get_instance (server, "nimf-system-keyboard");
          else
            im->priv->engine = nimf_service_im_get_instance (im, "nimf-system-keyboard");
        }

        nimf_service_im_engine_changed (im, nimf_engine_get_id (im->priv->engine),
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
      nimf_service_im_reset (im);

      if (server->priv->use_singleton)
        im->priv->engine = nimf_server_get_next_instance (server, im->priv->engine);
      else
        im->priv->engine = nimf_service_im_get_next_instance (im, im->priv->engine);

      nimf_service_im_engine_changed (im, nimf_engine_get_id (im->priv->engine),
                                      nimf_engine_get_icon_name (im->priv->engine));
    }

    return TRUE;
  }

  return nimf_engine_filter_event (im->priv->engine, im, event);
}

void
nimf_service_im_set_surrounding (NimfServiceIM *im,
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
nimf_service_im_set_use_preedit (NimfServiceIM *im,
                                 gboolean       use_preedit)
{
  g_debug (G_STRLOC ": %s: %d", G_STRFUNC, use_preedit);

  g_return_if_fail (im != NULL);

  if (im->use_preedit == TRUE && use_preedit == FALSE)
  {
    im->use_preedit = FALSE;

    if (im->preedit_state == NIMF_PREEDIT_STATE_START)
    {
      nimf_service_im_emit_preedit_changed (im, im->preedit_string,
                                                im->preedit_attrs,
                                                im->preedit_cursor_pos);
      nimf_service_im_emit_preedit_end (im);
    }
  }
  else if (im->use_preedit == FALSE && use_preedit == TRUE)
  {
    im->use_preedit = TRUE;

    if (im->preedit_string[0] != 0)
    {
      nimf_service_im_emit_preedit_start   (im);
      nimf_service_im_emit_preedit_changed (im, im->preedit_string,
                                                im->preedit_attrs,
                                                im->preedit_cursor_pos);
    }
  }
}

void
nimf_service_im_set_cursor_location (NimfServiceIM       *im,
                                     const NimfRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (im != NULL);

  if (G_UNLIKELY (im->priv->engine == NULL))
    return;

  NimfServer *server = nimf_server_get_default ();
  im->cursor_area    = *area;

  if (!im->use_preedit)
    nimf_preeditable_set_cursor_location (server->preeditable, area);
}

void nimf_service_im_reset (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (im != NULL);

  if (G_LIKELY (im->priv->engine))
    nimf_engine_reset (im->priv->engine, im);
}

void
nimf_service_im_set_engine_by_id (NimfServiceIM *im,
                                  const gchar   *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngine *engine;
  NimfServer *server = nimf_server_get_default ();

  if (server->priv->use_singleton)
    engine = nimf_server_get_instance (server, engine_id);
  else
    engine = nimf_service_im_get_instance (im, engine_id);

  g_return_if_fail (engine != NULL);

  im->priv->engine = engine;
  nimf_service_im_engine_changed (im, engine_id,
                                  nimf_engine_get_icon_name (im->priv->engine));
}

void
nimf_service_im_set_engine (NimfServiceIM *im,
                            const gchar   *engine_id,
                            const gchar   *method_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngine *engine;
  NimfServer *server = nimf_server_get_default ();

  if (server->priv->use_singleton)
    engine = nimf_server_get_instance (server, engine_id);
  else
    engine = nimf_service_im_get_instance (im, engine_id);

  g_return_if_fail (engine != NULL);

  im->priv->engine = engine;
  nimf_engine_set_method (engine, method_id);
  nimf_service_im_engine_changed (im, engine_id,
                                  nimf_engine_get_icon_name (im->priv->engine));
}

/**
 * nimf_service_im_get_service_id:
 * @im: a #NimfServiceIM
 *
 * Returns: (transfer none): a service id
 */
const gchar *
nimf_service_im_get_service_id (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIMClass *class = NIMF_SERVICE_IM_GET_CLASS (im);

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
 * nimf_service_im_get_engine:
 * @im: a #NimfServiceIM
 *
 * Returns the associated #NimfEngine instance.
 *
 * Returns: (transfer none): the engine instance
 */
NimfEngine *
nimf_service_im_get_engine (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return im->priv->engine;
}

static NimfEngine *
nimf_service_im_get_default_engine (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings  *settings;
  gchar      *engine_id;
  NimfEngine *engine;

  settings  = g_settings_new ("org.nimf.engines");
  engine_id = g_settings_get_string (settings, "default-engine");
  engine    = nimf_service_im_get_instance (im, engine_id);

  if (G_UNLIKELY (engine == NULL))
  {
    g_settings_reset (settings, "default-engine");
    g_free (engine_id);
    engine_id = g_settings_get_string (settings, "default-engine");
    engine = nimf_service_im_get_instance (im, engine_id);
  }

  g_free (engine_id);
  g_object_unref (settings);

  return engine;
}

static void
nimf_service_im_init (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  im->priv = nimf_service_im_get_instance_private (im);
}

static void
nimf_service_im_constructed (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIM *im     = NIMF_SERVICE_IM (object);
  NimfServer    *server = nimf_server_get_default ();

  im->use_preedit   = TRUE;
  im->preedit_state = NIMF_PREEDIT_STATE_END;

  if (server->priv->use_singleton)
  {
    im->priv->engine = nimf_server_get_default_engine (server);
  }
  else
  {
    im->priv->engines = nimf_service_im_create_engines (im);
    im->priv->engine  = nimf_service_im_get_default_engine (im);
  }

  im->preedit_string = g_strdup ("");
  im->preedit_attrs = g_malloc0_n (1, sizeof (NimfPreeditAttr *));
  im->preedit_attrs[0] = NULL;
  im->preedit_cursor_pos = 0;
}

static void
nimf_service_im_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIM *im = NIMF_SERVICE_IM (object);

  if (im->priv->engines)
    g_list_free_full (im->priv->engines, g_object_unref);

  g_free (im->preedit_string);
  nimf_preedit_attr_freev (im->preedit_attrs);

  G_OBJECT_CLASS (nimf_service_im_parent_class)->finalize (object);
}

static void
nimf_service_im_class_init (NimfServiceIMClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize    = nimf_service_im_finalize;
  object_class->constructed = nimf_service_im_constructed;
}
