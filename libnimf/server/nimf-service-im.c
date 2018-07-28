/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-service-im.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2017 Hodong Kim <cogniti@gmail.com>
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
#include "nimf-module.h"
#include <string.h>
#include "nimf-preeditable.h"
#include "nimf-key-syms.h"

G_DEFINE_ABSTRACT_TYPE (NimfServiceIM, nimf_service_im, G_TYPE_OBJECT);

enum
{
  PROP_0,
  PROP_SERVICE_IM_SERVER
};

void nimf_service_im_emit_preedit_start (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  NimfServiceIMClass *class = NIMF_SERVICE_IM_GET_CLASS (im);

  if (class->emit_preedit_start)
    class->emit_preedit_start (im);

  if (!im->use_preedit)
    nimf_preeditable_show (im->server->preeditable);
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

  NimfServiceIMClass *class = NIMF_SERVICE_IM_GET_CLASS (im);

  if (class->emit_preedit_changed)
    class->emit_preedit_changed (im, preedit_string, attrs, cursor_pos);

  if (!im->use_preedit)
    nimf_preeditable_set_text (im->server->preeditable, preedit_string);
}

void
nimf_service_im_emit_preedit_end (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!im))
    return;

  NimfServiceIMClass *class = NIMF_SERVICE_IM_GET_CLASS (im);

  if (class->emit_preedit_end)
    class->emit_preedit_end (im);

  if (!im->use_preedit)
    nimf_preeditable_hide (im->server->preeditable);
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

  g_signal_emit_by_name (im->server, "engine-changed", engine_id, name);
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

  if (G_UNLIKELY (im->engine == NULL))
    return;

  nimf_engine_focus_in (im->engine, im);
  nimf_service_im_engine_changed (im, nimf_engine_get_id (im->engine),
                                  nimf_engine_get_icon_name (im->engine));
}

void nimf_service_im_focus_out (NimfServiceIM *im)
{
  g_return_if_fail (im != NULL);

  g_debug (G_STRLOC ": %s: im icid = %d", G_STRFUNC, im->icid);

  if (G_UNLIKELY (im->engine == NULL))
    return;

  nimf_engine_focus_out (im->engine, im);
  nimf_service_im_engine_changed (im, NULL, "nimf-focus-out");
  nimf_preeditable_hide (im->server->preeditable);
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

  g_hash_table_iter_init (&iter, im->server->modules);

  while (g_hash_table_iter_next (&iter, NULL, &module))
  {
    NimfEngine *engine;
    engine = g_object_new (NIMF_MODULE (module)->type, "server",
                           im->server, NULL);
    engines = g_list_prepend (engines, engine);
  }

  return engines;
}

static NimfEngine *
nimf_service_im_get_instance (NimfServiceIM *im, const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  if (im->engines == NULL)
    im->engines = nimf_service_im_create_engines (im);

  list = g_list_find_custom (g_list_first (im->engines), engine_id,
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

  if (im->engines == NULL)
    im->engines = nimf_service_im_create_engines (im);

  im->engines = g_list_first (im->engines);
  im->engines = g_list_find  (im->engines, engine);

  list = g_list_next (im->engines);

  if (list == NULL)
    list = g_list_first (im->engines);

  if (list)
  {
    im->engines = list;
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

  if (G_UNLIKELY (im->engine == NULL))
    return FALSE;

  GHashTableIter iter;
  gpointer       trigger_keys;
  gpointer       engine_id;

  g_hash_table_iter_init (&iter, im->server->trigger_keys);

  while (g_hash_table_iter_next (&iter, &trigger_keys, &engine_id))
  {
    if (nimf_event_matches (event, trigger_keys))
    {
      if (event->key.type == NIMF_EVENT_KEY_PRESS)
      {
        nimf_service_im_reset (im);

        if (g_strcmp0 (nimf_engine_get_id (im->engine), engine_id) != 0)
        {
          if (im->server->use_singleton)
            im->engine = nimf_server_get_instance (im->server, engine_id);
          else
            im->engine = nimf_service_im_get_instance (im, engine_id);
        }
        else
        {
          if (im->server->use_singleton)
            im->engine = nimf_server_get_instance (im->server,
                                                   "nimf-system-keyboard");
          else
            im->engine = nimf_service_im_get_instance (im,
                                                       "nimf-system-keyboard");
        }

        nimf_service_im_engine_changed (im, nimf_engine_get_id (im->engine),
                                        nimf_engine_get_icon_name (im->engine));
      }

      if (event->key.keyval == NIMF_KEY_Escape)
        return FALSE;

      return TRUE;
    }
  }

  if (nimf_event_matches (event, (const NimfKey **) im->server->hotkeys))
  {
    if (event->key.type == NIMF_EVENT_KEY_PRESS)
    {
      nimf_service_im_reset (im);

      if (im->server->use_singleton)
        im->engine = nimf_server_get_next_instance (im->server, im->engine);
      else
        im->engine = nimf_service_im_get_next_instance (im, im->engine);

      nimf_service_im_engine_changed (im, nimf_engine_get_id (im->engine),
                                      nimf_engine_get_icon_name (im->engine));
    }

    return TRUE;
  }

  return nimf_engine_filter_event (im->engine, im, event);
}

void
nimf_service_im_set_surrounding (NimfServiceIM *im,
                                 const char    *text,
                                 gint           len,
                                 gint           cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (im != NULL);

  if (G_UNLIKELY (im->engine == NULL))
    return;

  nimf_engine_set_surrounding (im->engine, text, len, cursor_index);
}

gboolean
nimf_service_im_get_surrounding (NimfServiceIM  *im,
                                 gchar         **text,
                                 gint           *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (im != NULL, FALSE);

  if (G_UNLIKELY (im->engine == NULL))
    return FALSE;

  return nimf_engine_get_surrounding (im->engine, im, text, cursor_index);
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

  if (G_UNLIKELY (im->engine == NULL))
    return;

  im->cursor_area = *area;

  if (!im->use_preedit)
    nimf_preeditable_set_cursor_location (im->server->preeditable, area);
}

void nimf_service_im_reset (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (im != NULL);

  if (G_LIKELY (im->engine))
    nimf_engine_reset (im->engine, im);
}

void
nimf_service_im_set_engine_by_id (NimfServiceIM *im,
                                  const gchar   *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngine *engine;

  if (im->server->use_singleton)
    engine = nimf_server_get_instance (im->server, engine_id);
  else
    engine = nimf_service_im_get_instance (im, engine_id);

  g_return_if_fail (engine != NULL);

  im->engine = engine;
  nimf_service_im_engine_changed (im, engine_id,
                                  nimf_engine_get_icon_name (im->engine));
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
}

static void
nimf_service_im_constructed (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIM *im = NIMF_SERVICE_IM (object);
  im->use_preedit   = TRUE;
  im->preedit_state = NIMF_PREEDIT_STATE_END;

  if (im->server->use_singleton)
  {
    im->engine = nimf_server_get_default_engine (im->server);
  }
  else
  {
    im->engines = nimf_service_im_create_engines (im);
    im->engine  = nimf_service_im_get_default_engine (im);
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

  if (im->engines)
    g_list_free_full (im->engines, g_object_unref);

  g_free (im->preedit_string);
  nimf_preedit_attr_freev (im->preedit_attrs);

  G_OBJECT_CLASS (nimf_service_im_parent_class)->finalize (object);
}

static void
nimf_service_im_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIM *im = NIMF_SERVICE_IM (object);

  switch (prop_id)
  {
    case PROP_SERVICE_IM_SERVER:
      im->server = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
nimf_service_im_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIM *im = NIMF_SERVICE_IM (object);

  switch (prop_id)
  {
    case PROP_SERVICE_IM_SERVER:
      g_value_set_object (value, im->server);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
nimf_service_im_class_init (NimfServiceIMClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = nimf_service_im_finalize;
  object_class->set_property = nimf_service_im_set_property;
  object_class->get_property = nimf_service_im_get_property;
  object_class->constructed  = nimf_service_im_constructed;

  g_object_class_install_property (object_class,
                                   PROP_SERVICE_IM_SERVER,
                                   g_param_spec_object ("server",
                                                        "server",
                                                        "server",
                                                        NIMF_TYPE_SERVER,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}
