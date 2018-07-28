/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-engine.c
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

#include "nimf-engine.h"
#include "nimf-private.h"

enum
{
  PROP_0,
  PROP_SERVER
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (NimfEngine, nimf_engine, G_TYPE_OBJECT);

static void
nimf_engine_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (object));

  NimfEngine *engine = NIMF_ENGINE (object);

  switch (prop_id)
  {
    case PROP_SERVER:
      engine->priv->server = g_value_get_object (value);
      g_object_notify_by_pspec (object, pspec);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
nimf_engine_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (object));

  NimfEngine *engine = NIMF_ENGINE (object);

  switch (prop_id)
  {
    case PROP_SERVER:
      g_value_set_object (value, engine->priv->server);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

void nimf_engine_reset (NimfEngine    *engine,
                        NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->reset)
    class->reset (engine, im);
}

void nimf_engine_focus_in (NimfEngine    *engine,
                           NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->focus_in)
    class->focus_in (engine, im);
}

void nimf_engine_focus_out (NimfEngine    *engine,
                            NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->focus_out)
    class->focus_out (engine, im);
}

gboolean nimf_engine_filter_event (NimfEngine    *engine,
                                   NimfServiceIM *im,
                                   NimfEvent     *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  return class->filter_event (engine, im, event);
}

gboolean nimf_engine_real_filter_event (NimfEngine    *engine,
                                        NimfServiceIM *im,
                                        NimfEvent     *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return FALSE;
}

void
nimf_engine_set_surrounding (NimfEngine *engine,
                             const char *text,
                             gint        len,
                             gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));
  g_return_if_fail (text != NULL || len == 0);

  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->set_surrounding)
    class->set_surrounding (engine, text, len, cursor_index);
}

gboolean
nimf_engine_get_surrounding (NimfEngine     *engine,
                             NimfServiceIM  *im,
                             gchar         **text,
                             gint           *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;
  NimfEngineClass *class = NIMF_ENGINE_GET_CLASS (engine);

  if (class->get_surrounding)
    retval = class->get_surrounding (engine, im, text, cursor_index);

  return retval;
}

void
nimf_engine_emit_preedit_start (NimfEngine    *engine,
                                NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_service_im_emit_preedit_start (im);
}

void
nimf_engine_emit_preedit_changed (NimfEngine       *engine,
                                  NimfServiceIM    *im,
                                  const gchar      *preedit_string,
                                  NimfPreeditAttr **attrs,
                                  gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_service_im_emit_preedit_changed (im, preedit_string, attrs, cursor_pos);
}

void
nimf_engine_emit_preedit_end (NimfEngine    *engine,
                              NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_service_im_emit_preedit_end (im);
}

void
nimf_engine_emit_commit (NimfEngine    *engine,
                         NimfServiceIM *im,
                         const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_service_im_emit_commit (im, text);
}

gboolean
nimf_engine_emit_delete_surrounding (NimfEngine    *engine,
                                     NimfServiceIM *im,
                                     gint           offset,
                                     gint           n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_service_im_emit_delete_surrounding (im, offset, n_chars);
}

gboolean
nimf_engine_emit_retrieve_surrounding (NimfEngine    *engine,
                                       NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_service_im_emit_retrieve_surrounding (im);
}

void
nimf_engine_status_changed (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_signal_emit_by_name (engine->priv->server, "engine-status-changed",
                         nimf_engine_get_id (engine),
                         nimf_engine_get_icon_name (engine));
}

void
nimf_engine_emit_beep (NimfEngine    *engine,
                       NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_service_im_emit_beep (im);
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

static void
nimf_engine_real_set_surrounding (NimfEngine *engine,
                                  const char *text,
                                  gint        len,
                                  gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_free (engine->priv->surrounding_text);
  engine->priv->surrounding_text         = g_strndup (text, len);
  engine->priv->surrounding_cursor_index = cursor_index;
}

static gboolean
nimf_engine_real_get_surrounding (NimfEngine     *engine,
                                  NimfServiceIM  *im,
                                  gchar         **text,
                                  gint           *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = nimf_engine_emit_retrieve_surrounding (engine, im);

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

const gchar *
nimf_engine_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_ENGINE_GET_CLASS (engine)->get_id (engine);
}

NimfCandidatable *
nimf_engine_get_candidatable (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return engine->priv->server->candidatable;
}

static const gchar *
nimf_engine_real_get_id (NimfEngine *engine)
{
  g_critical (G_STRLOC ": %s: You should implement your_engine_get_id ()",
              G_STRFUNC);
  return NULL;
}

const gchar *
nimf_engine_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_ENGINE_GET_CLASS (engine)->get_icon_name (engine);
}

static const gchar *
nimf_engine_real_get_icon_name (NimfEngine *engine)
{
  g_error ("You should implement your_engine_get_icon_name ()");

  return NULL;
}

static void
nimf_engine_class_init (NimfEngineClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize     = nimf_engine_finalize;

  object_class->set_property = nimf_engine_set_property;
  object_class->get_property = nimf_engine_get_property;

  class->filter_event        = nimf_engine_real_filter_event;
  class->set_surrounding     = nimf_engine_real_set_surrounding;
  class->get_surrounding     = nimf_engine_real_get_surrounding;
  class->get_id              = nimf_engine_real_get_id;
  class->get_icon_name       = nimf_engine_real_get_icon_name;

  g_object_class_install_property (object_class,
                                   PROP_SERVER,
                                   g_param_spec_object ("server",
                                                        "server",
                                                        "server",
                                                        NIMF_TYPE_SERVER,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}
