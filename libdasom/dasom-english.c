/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-english.c
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

#include "dasom-english.h"
#include "dasom-key-syms.h"

G_DEFINE_TYPE (DasomEnglish, dasom_english, DASOM_TYPE_ENGINE);

void
dasom_english_reset (DasomEngine *engine, DasomConnection  *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

const gchar *
dasom_english_get_id (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return DASOM_ENGLISH (engine)->id;
}

const gchar *
dasom_english_get_name (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return DASOM_ENGLISH (engine)->name;
}

gboolean
dasom_english_filter_event (DasomEngine     *engine,
                            DasomConnection *target,
                            DasomEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;

  if ((event->key.type == DASOM_EVENT_KEY_RELEASE) ||
      (event->key.keyval == DASOM_KEY_Shift_L)     ||
      (event->key.keyval == DASOM_KEY_Shift_R)     ||
      (event->key.state & (DASOM_CONTROL_MASK | DASOM_MOD1_MASK)))
    return FALSE;

  gchar c = 0;

  if (event->key.keyval >= 32 && event->key.keyval <= 126)
    c = event->key.keyval;

  if (!c)
  {
    switch (event->key.keyval)
    {
      case DASOM_KEY_KP_Multiply: c = '*'; break;
      case DASOM_KEY_KP_Add:      c = '+'; break;
      case DASOM_KEY_KP_Subtract: c = '-'; break;
      case DASOM_KEY_KP_Divide:   c = '/'; break;
      default:
        break;
    }
  }

  if (!c && (event->key.state & DASOM_MOD2_MASK))
  {
    switch (event->key.keyval)
    {
      case DASOM_KEY_KP_Decimal:  c = '.'; break;
      case DASOM_KEY_KP_0:        c = '0'; break;
      case DASOM_KEY_KP_1:        c = '1'; break;
      case DASOM_KEY_KP_2:        c = '2'; break;
      case DASOM_KEY_KP_3:        c = '3'; break;
      case DASOM_KEY_KP_4:        c = '4'; break;
      case DASOM_KEY_KP_5:        c = '5'; break;
      case DASOM_KEY_KP_6:        c = '6'; break;
      case DASOM_KEY_KP_7:        c = '7'; break;
      case DASOM_KEY_KP_8:        c = '8'; break;
      case DASOM_KEY_KP_9:        c = '9'; break;
      default:
        break;
    }
  }

  if (c)
  {
    gchar *str = g_strdup_printf ("%c", c);
    dasom_engine_emit_commit (engine, target, str);
    g_free (str);
    retval = TRUE;
  }

  return retval;
}

static void
dasom_english_init (DasomEnglish *english)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  english->id   = g_strdup ("dasom-english");
  english->name = g_strdup ("en");
}

static void
dasom_english_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomEnglish *english = DASOM_ENGLISH (object);
  g_free (english->id);
  g_free (english->name);

  G_OBJECT_CLASS (dasom_english_parent_class)->finalize (object);
}

static void
dasom_english_class_init (DasomEnglishClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  DasomEngineClass *engine_class = DASOM_ENGINE_CLASS (klass);

  engine_class->filter_event = dasom_english_filter_event;
  engine_class->reset        = dasom_english_reset;
  engine_class->get_id       = dasom_english_get_id;
  engine_class->get_name     = dasom_english_get_name;

  object_class->finalize = dasom_english_finalize;
}
