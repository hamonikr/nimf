/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-engine.c
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

#include "dasom-engine.h"
#include "dasom-private.h"

enum
{
  PROP_0,
  PROP_SERVER
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (DasomEngine, dasom_engine, G_TYPE_OBJECT);

static void
dasom_engine_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (object));

  DasomEngine *engine = DASOM_ENGINE (object);

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
dasom_engine_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (object));

  DasomEngine *engine = DASOM_ENGINE (object);

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

void dasom_engine_reset (DasomEngine     *engine,
                         DasomConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  DasomEngineClass *class = DASOM_ENGINE_GET_CLASS (engine);

  if (class->reset)
    class->reset (engine, target);
}

void dasom_engine_focus_in (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  DasomEngineClass *class = DASOM_ENGINE_GET_CLASS (engine);

  if (class->focus_in)
    class->focus_in (engine);
}

void dasom_engine_focus_out (DasomEngine     *engine,
                             DasomConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  DasomEngineClass *class = DASOM_ENGINE_GET_CLASS (engine);

  if (class->focus_out)
    class->focus_out (engine, target);
}

gboolean dasom_engine_filter_event (DasomEngine     *engine,
                                    DasomConnection *target,
                                    DasomEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomEngineClass *class = DASOM_ENGINE_GET_CLASS (engine);

  return class->filter_event (engine, target, event);
}

gboolean dasom_engine_real_filter_event (DasomEngine     *engine,
                                         DasomConnection *target,
                                         DasomEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return FALSE;
}

void
dasom_engine_get_preedit_string (DasomEngine  *engine,
                                 gchar       **str,
                                 gint         *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  DasomEngineClass *class = DASOM_ENGINE_GET_CLASS (engine);
  class->get_preedit_string (engine, str, cursor_pos);
}

void
dasom_engine_set_surrounding (DasomEngine  *engine,
                              const char   *text,
                              gint          len,
                              gint          cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));
  g_return_if_fail (text != NULL || len == 0);

  DasomEngineClass *class = DASOM_ENGINE_GET_CLASS (engine);

  if (class->set_surrounding)
    class->set_surrounding (engine, text, len, cursor_index);
}

gboolean
dasom_engine_get_surrounding (DasomEngine      *engine,
                              DasomConnection  *target,
                              gchar           **text,
                              gint             *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;
  DasomEngineClass *class = DASOM_ENGINE_GET_CLASS (engine);

  if (class->get_surrounding)
    retval = class->get_surrounding (engine, target, text, cursor_index);

  return retval;
}

void
dasom_engine_set_cursor_location (DasomEngine          *engine,
                                  const DasomRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  DasomEngineClass *class = DASOM_ENGINE_GET_CLASS (engine);

  if (class->set_cursor_location)
    class->set_cursor_location (engine, area);
}

void
dasom_engine_set_english_mode (DasomEngine *engine,
                               gboolean     is_english_mode)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomEngineClass *class = DASOM_ENGINE_GET_CLASS (engine);

  if (class->set_english_mode)
    class->set_english_mode (engine, is_english_mode);
}

gboolean
dasom_engine_get_english_mode (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomEngineClass *class = DASOM_ENGINE_GET_CLASS (engine);

  if (class->get_english_mode)
    return class->get_english_mode (engine);

  return TRUE;
}

void
dasom_engine_emit_preedit_start (DasomEngine     *engine,
                                 DasomConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_connection_emit_preedit_start (target);
}

void
dasom_engine_emit_preedit_changed (DasomEngine     *engine,
                                   DasomConnection *target,
                                   const gchar     *preedit_string,
                                   gint             cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_connection_emit_preedit_changed (target, preedit_string, cursor_pos);
}

void
dasom_engine_emit_preedit_end (DasomEngine     *engine,
                               DasomConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_connection_emit_preedit_end (target);
}

void
dasom_engine_emit_commit (DasomEngine     *engine,
                          DasomConnection *target,
                          const gchar     *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_connection_emit_commit (target, text);
}

gboolean
dasom_engine_emit_delete_surrounding (DasomEngine     *engine,
                                      DasomConnection *target,
                                      gint             offset,
                                      gint             n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return dasom_connection_emit_delete_surrounding (target, offset, n_chars);
}

gboolean
dasom_engine_emit_retrieve_surrounding (DasomEngine     *engine,
                                        DasomConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return dasom_connection_emit_retrieve_surrounding (target);
}

void
dasom_engine_emit_engine_changed (DasomEngine     *engine,
                                  DasomConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_signal_emit_by_name (target, "engine-changed",
                         dasom_engine_get_name (engine));
}

static void
dasom_engine_init (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  engine->priv = dasom_engine_get_instance_private (engine);
}

static void
dasom_engine_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomEngine *engine = DASOM_ENGINE (object);

  g_free (engine->priv->surrounding_text);

  G_OBJECT_CLASS (dasom_engine_parent_class)->finalize (object);
}


static void
dasom_engine_real_get_preedit_string (DasomEngine  *engine,
                                      gchar       **str,
                                      gint         *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (str)
    *str = g_strdup ("");

  if (cursor_pos)
    *cursor_pos = 0;
}

static void
dasom_engine_real_set_surrounding (DasomEngine  *engine,
                                   const char   *text,
                                   gint          len,
                                   gint          cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_free (engine->priv->surrounding_text);
  engine->priv->surrounding_text         = g_strndup (text, len);
  engine->priv->surrounding_cursor_index = cursor_index;
}

static gboolean
dasom_engine_real_get_surrounding (DasomEngine      *engine,
                                   DasomConnection  *target,
                                   gchar           **text,
                                   gint             *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = dasom_engine_emit_retrieve_surrounding (engine, target);

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

void
dasom_engine_update_candidate_window (DasomEngine  *engine,
                                      const gchar **strv)
{
  dasom_candidate_update_window (engine->priv->server->candidate, strv);
}

void
dasom_engine_show_candidate_window (DasomEngine     *engine,
                                    DasomConnection *target)
{
  dasom_candidate_show_window (engine->priv->server->candidate, target);
}

void
dasom_engine_hide_candidate_window (DasomEngine *engine)
{
  dasom_candidate_hide_window (engine->priv->server->candidate);
}

void
dasom_engine_select_previous_candidate_item (DasomEngine *engine)
{
  dasom_candidate_select_previous_item (engine->priv->server->candidate);
}

void
dasom_engine_select_next_candidate_item (DasomEngine *engine)
{
  dasom_candidate_select_next_item (engine->priv->server->candidate);
}

void
dasom_engine_select_page_up_candidate_item (DasomEngine *engine)
{
  dasom_candidate_select_page_up_item (engine->priv->server->candidate);
}

void
dasom_engine_select_page_down_candidate_item (DasomEngine *engine)
{
  dasom_candidate_select_page_down_item (engine->priv->server->candidate);
}

gchar *
dasom_engine_get_selected_candidate_text (DasomEngine *engine)
{
  return dasom_candidate_get_selected_text (engine->priv->server->candidate);
}

const gchar *
dasom_engine_get_id (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return DASOM_ENGINE_GET_CLASS (engine)->get_id (engine);
}

static const gchar *
dasom_engine_real_get_id (DasomEngine *engine)
{
  g_critical (G_STRLOC ": %s: You should implement your_engine_get_id ()",
              G_STRFUNC);
  return NULL;
}

const gchar *
dasom_engine_get_name (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return DASOM_ENGINE_GET_CLASS (engine)->get_name (engine);
}

static const gchar *
dasom_engine_real_get_name (DasomEngine *engine)
{
  /* FIXME */
  g_error ("You should implement your_engine_get_name ()");

  return NULL;
}

static void
dasom_engine_class_init (DasomEngineClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize     = dasom_engine_finalize;

  object_class->set_property = dasom_engine_set_property;
  object_class->get_property = dasom_engine_get_property;

  class->filter_event        = dasom_engine_real_filter_event;
  class->get_preedit_string  = dasom_engine_real_get_preedit_string;
  class->set_surrounding     = dasom_engine_real_set_surrounding;
  class->get_surrounding     = dasom_engine_real_get_surrounding;
 /* TODO: maybe get_engine_info */
  class->get_id              = dasom_engine_real_get_id;
  class->get_name            = dasom_engine_real_get_name;

  g_object_class_install_property (object_class,
                                   PROP_SERVER,
                                   g_param_spec_object ("server",
                                                        "server",
                                                        "server",
                                                        DASOM_TYPE_SERVER,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}
