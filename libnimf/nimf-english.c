/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-english.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-english.h"
#include "nimf-key-syms.h"

G_DEFINE_TYPE (NimfEnglish, nimf_english, NIMF_TYPE_ENGINE);

void
nimf_english_reset (NimfEngine *engine, NimfConnection  *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

const gchar *
nimf_english_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_ENGLISH (engine)->id;
}

const gchar *
nimf_english_get_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_ENGLISH (engine)->name;
}

gboolean
nimf_english_filter_event (NimfEngine     *engine,
                           NimfConnection *target,
                           NimfEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;

  if ((event->key.type == NIMF_EVENT_KEY_RELEASE) ||
      (event->key.keyval == NIMF_KEY_Shift_L)     ||
      (event->key.keyval == NIMF_KEY_Shift_R)     ||
      (event->key.state & (NIMF_CONTROL_MASK | NIMF_MOD1_MASK)))
    return FALSE;

  gchar c = 0;

  if (event->key.keyval >= 32 && event->key.keyval <= 126)
    c = event->key.keyval;

  if (!c)
  {
    switch (event->key.keyval)
    {
      case NIMF_KEY_KP_Multiply: c = '*'; break;
      case NIMF_KEY_KP_Add:      c = '+'; break;
      case NIMF_KEY_KP_Subtract: c = '-'; break;
      case NIMF_KEY_KP_Divide:   c = '/'; break;
      default:
        break;
    }
  }

  if (!c && (event->key.state & NIMF_MOD2_MASK))
  {
    switch (event->key.keyval)
    {
      case NIMF_KEY_KP_Decimal:  c = '.'; break;
      case NIMF_KEY_KP_0:        c = '0'; break;
      case NIMF_KEY_KP_1:        c = '1'; break;
      case NIMF_KEY_KP_2:        c = '2'; break;
      case NIMF_KEY_KP_3:        c = '3'; break;
      case NIMF_KEY_KP_4:        c = '4'; break;
      case NIMF_KEY_KP_5:        c = '5'; break;
      case NIMF_KEY_KP_6:        c = '6'; break;
      case NIMF_KEY_KP_7:        c = '7'; break;
      case NIMF_KEY_KP_8:        c = '8'; break;
      case NIMF_KEY_KP_9:        c = '9'; break;
      default:
        break;
    }
  }

  if (c)
  {
    gchar *str = g_strdup_printf ("%c", c);
    nimf_engine_emit_commit (engine, target, str);
    g_free (str);
    retval = TRUE;
  }

  return retval;
}

static void
nimf_english_init (NimfEnglish *english)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  english->id   = g_strdup ("nimf-english");
  english->name = g_strdup ("en");
}

static void
nimf_english_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEnglish *english = NIMF_ENGLISH (object);
  g_free (english->id);
  g_free (english->name);

  G_OBJECT_CLASS (nimf_english_parent_class)->finalize (object);
}

static void
nimf_english_class_init (NimfEnglishClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  NimfEngineClass *engine_class = NIMF_ENGINE_CLASS (klass);

  engine_class->filter_event = nimf_english_filter_event;
  engine_class->reset        = nimf_english_reset;
  engine_class->get_id       = nimf_english_get_id;
  engine_class->get_name     = nimf_english_get_name;

  object_class->finalize = nimf_english_finalize;
}
