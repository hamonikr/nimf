/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-jeongeum.c
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

#include "dasom.h"
#include "dasom-jeongeum.h"
#include "modules/engines/dasom-english/dasom-english.h"

G_DEFINE_DYNAMIC_TYPE (DasomJeongeum, dasom_jeongeum, DASOM_TYPE_ENGINE);

/* only for PC keyboards */
guint dasom_event_keycode_to_qwerty_keyval (const DasomEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  guint keyval = 0;
  gboolean is_shift = event->key.state & DASOM_SHIFT_MASK;

  switch (event->key.hardware_keycode)
  {
    /* 20(-) ~ 21(=) */
    case 20: keyval = is_shift ? '_' : '-';  break;
    case 21: keyval = is_shift ? '+' : '=';  break;
    /* 24(q) ~ 35(]) */
    case 24: keyval = is_shift ? 'Q' : 'q';  break;
    case 25: keyval = is_shift ? 'W' : 'w';  break;
    case 26: keyval = is_shift ? 'E' : 'e';  break;
    case 27: keyval = is_shift ? 'R' : 'r';  break;
    case 28: keyval = is_shift ? 'T' : 't';  break;
    case 29: keyval = is_shift ? 'Y' : 'y';  break;
    case 30: keyval = is_shift ? 'U' : 'u';  break;
    case 31: keyval = is_shift ? 'I' : 'i';  break;
    case 32: keyval = is_shift ? 'O' : 'o';  break;
    case 33: keyval = is_shift ? 'P' : 'p';  break;
    case 34: keyval = is_shift ? '{' : '[';  break;
    case 35: keyval = is_shift ? '}' : ']';  break;
    /* 38(a) ~ 48(') */
    case 38: keyval = is_shift ? 'A' : 'a';  break;
    case 39: keyval = is_shift ? 'S' : 's';  break;
    case 40: keyval = is_shift ? 'D' : 'd';  break;
    case 41: keyval = is_shift ? 'F' : 'f';  break;
    case 42: keyval = is_shift ? 'G' : 'g';  break;
    case 43: keyval = is_shift ? 'H' : 'h';  break;
    case 44: keyval = is_shift ? 'J' : 'j';  break;
    case 45: keyval = is_shift ? 'K' : 'k';  break;
    case 46: keyval = is_shift ? 'L' : 'l';  break;
    case 47: keyval = is_shift ? ':' : ';';  break;
    case 48: keyval = is_shift ? '"' : '\''; break;
    /* 52(z) ~ 61(?) */
    case 52: keyval = is_shift ? 'Z' : 'z';  break;
    case 53: keyval = is_shift ? 'X' : 'x';  break;
    case 54: keyval = is_shift ? 'C' : 'c';  break;
    case 55: keyval = is_shift ? 'V' : 'v';  break;
    case 56: keyval = is_shift ? 'B' : 'b';  break;
    case 57: keyval = is_shift ? 'N' : 'n';  break;
    case 58: keyval = is_shift ? 'M' : 'm';  break;
    case 59: keyval = is_shift ? '<' : ',';  break;
    case 60: keyval = is_shift ? '>' : '.';  break;
    case 61: keyval = is_shift ? '?' : '/';  break;
    default: keyval = event->key.keyval;     break;
  }

  return keyval;
}

void
dasom_jeongeum_reset (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);

  dasom_candidate_hide_window (jeongeum->candidate);
  jeongeum->is_candidate_mode = FALSE;

  const ucschar *flush;
  flush = hangul_ic_flush (jeongeum->context);

  if (flush[0] == 0)
    return;

  gchar *text = NULL;

  text = g_ucs4_to_utf8 (flush, -1, NULL, NULL, NULL);

  if (jeongeum->preedit_string != NULL)
  {
    g_free (jeongeum->preedit_string);
    jeongeum->preedit_string = NULL;
    dasom_engine_emit_preedit_changed (engine);
    dasom_engine_emit_preedit_end (engine);
  }

  dasom_engine_emit_commit (engine, text);

  g_free (text);
}

void
dasom_jeongeum_focus_in (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));
}

void
dasom_jeongeum_focus_out (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  dasom_jeongeum_reset (engine);
}

gboolean
dasom_jeongeum_filter_event (DasomEngine *engine,
                             DasomEvent  *event)
{
  g_debug (G_STRLOC ": %s:keyval:%d\t hardware_keycode:%d",
           G_STRFUNC,
           event->key.keyval,
           event->key.hardware_keycode);

  guint keyval;
  gboolean retval = FALSE;

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);

  if (event->key.type   == DASOM_EVENT_KEY_RELEASE ||
      event->key.keyval == DASOM_KEY_Shift_L       ||
      event->key.keyval == DASOM_KEY_Shift_R)
    return FALSE;

  if (event->key.state & (DASOM_CONTROL_MASK | DASOM_MOD1_MASK))
  {
    dasom_jeongeum_reset (engine);
    return FALSE;
  }

  if (dasom_event_matches (event, (const DasomKey **) jeongeum->hangul_keys))
  {
    dasom_jeongeum_reset (engine);
    jeongeum->is_english_mode = !jeongeum->is_english_mode;
    dasom_engine_emit_engine_changed (engine);
    return TRUE;
  }

  if (jeongeum->is_english_mode)
    return dasom_english_filter_event (engine, event);

  if (event->key.keyval == DASOM_KEY_Hangul_Hanja)
  {
    if (jeongeum->is_candidate_mode == FALSE)
    {
      jeongeum->is_candidate_mode = TRUE;
      HanjaList *list = hanja_table_match_exact (jeongeum->hanja_table,
                                                 jeongeum->preedit_string);
      gint list_len = hanja_list_get_size (list);
      gchar **strv = g_malloc0 ((list_len + 1) * sizeof (gchar *));

      if (list)
      {
        gint i;
        for (i = 0; i < list_len; i++)
        {
          const char *hanja = hanja_list_get_nth_value (list, i);
          strv[i] = g_strdup (hanja);
        }

        hanja_list_delete (list);
      }

      dasom_candidate_update_window (jeongeum->candidate, (const gchar **) strv);
      g_strfreev (strv);
      dasom_candidate_show_window (jeongeum->candidate);
    }
    else
    {
      jeongeum->is_candidate_mode = FALSE;
      dasom_candidate_hide_window (jeongeum->candidate);
    }

    return TRUE;
  }

  if (jeongeum->is_candidate_mode)
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    switch (event->key.keyval)
    {
      case DASOM_KEY_Return:
      case DASOM_KEY_KP_Enter:
        {
          gchar *text = dasom_candidate_get_selected_text (jeongeum->candidate);
          g_signal_emit_by_name (jeongeum->candidate, "row-activated", text);
          g_free (text);
        }
        break;
      case DASOM_KEY_Up:
        dasom_candidate_select_previous_item (jeongeum->candidate);
        break;
      case DASOM_KEY_Down:
        dasom_candidate_select_next_item (jeongeum->candidate);
        break;
      case DASOM_KEY_Escape:
        dasom_candidate_hide_window (jeongeum->candidate);
        jeongeum->is_candidate_mode = FALSE;
        break;
      default:
        break;
    }

    return TRUE;
  }

  const ucschar *commit  = hangul_ic_get_commit_string  (jeongeum->context);
  const ucschar *preedit = hangul_ic_get_preedit_string (jeongeum->context);

  gchar *new_commit  = NULL;
  gchar *new_preedit = NULL;
  gchar *old_preedit = g_strdup (jeongeum->preedit_string);

  if (event->key.keyval == DASOM_KEY_BackSpace ||
      event->key.keyval == DASOM_KEY_Delete    ||
      event->key.keyval == DASOM_KEY_KP_Delete)
  {
    retval = hangul_ic_backspace (jeongeum->context);

    if (retval)
    {
      gchar *new_preedit = NULL;
      const ucschar *preedit = hangul_ic_get_preedit_string (jeongeum->context);

      if (preedit[0] != 0)
        new_preedit = g_ucs4_to_utf8 (preedit, -1, NULL, NULL, NULL);

      if (g_strcmp0 (old_preedit, new_preedit) != 0)
      {
        g_free (jeongeum->preedit_string);
        jeongeum->preedit_string = new_preedit;
        dasom_engine_emit_preedit_changed (engine);
      }

      if (old_preedit != NULL && preedit[0] == 0)
        dasom_engine_emit_preedit_end (engine);
    }

    return retval;
  }

  keyval = dasom_event_keycode_to_qwerty_keyval (event);
  retval = hangul_ic_process (jeongeum->context, keyval);

  if (commit[0] != 0)
    new_commit  = g_ucs4_to_utf8 (commit, -1, NULL, NULL, NULL);

  if (preedit[0] != 0)
    new_preedit = g_ucs4_to_utf8 (preedit, -1, NULL, NULL, NULL);

  if (commit[0] == 0)
  {
    if (old_preedit == NULL && preedit[0] != 0)
      dasom_engine_emit_preedit_start (engine);

    if (g_strcmp0 (old_preedit, new_preedit) != 0)
    {
      g_free (jeongeum->preedit_string);
      jeongeum->preedit_string = new_preedit;
      dasom_engine_emit_preedit_changed (engine);
    }

    if (old_preedit != NULL && preedit[0] == 0)
      dasom_engine_emit_preedit_end (engine);
  }

  if (commit[0] != 0)
  {
    if (old_preedit != NULL)
    {
      g_free (jeongeum->preedit_string);
      jeongeum->preedit_string = NULL;
      dasom_engine_emit_preedit_changed (engine);
      dasom_engine_emit_preedit_end (engine);
    }

    dasom_engine_emit_commit (engine, new_commit);

    if (preedit[0] != 0)
    {
      g_free (jeongeum->preedit_string);
      jeongeum->preedit_string = new_preedit;
      dasom_engine_emit_preedit_start (engine);
      dasom_engine_emit_preedit_changed (engine);
    }
  }

  g_free (new_commit);
  g_free (old_preedit);

  if (retval)
    return TRUE;

  gchar c = 0;

  if (event->key.keyval == DASOM_KEY_space)
    c = ' ';

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
    dasom_engine_emit_commit (engine, str);
    g_free (str);
    retval = TRUE;
  }

  return retval;
}

void
on_candidate_row_activated (DasomCandidate *candidate,
                            const gchar    *text,
                            gpointer        user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomJeongeum *jeongeum = user_data;

  if (text)
  {
    hangul_ic_reset (jeongeum->context);

    if (jeongeum->preedit_string != NULL)
    {
      g_free (jeongeum->preedit_string);
      jeongeum->preedit_string = NULL;
      dasom_engine_emit_preedit_changed (DASOM_ENGINE (jeongeum));
      dasom_engine_emit_preedit_end (DASOM_ENGINE (jeongeum));
    }

    dasom_engine_emit_commit (DASOM_ENGINE (jeongeum), text);
  }

  dasom_candidate_hide_window (jeongeum->candidate);
  jeongeum->is_candidate_mode = FALSE;
}

static void
dasom_jeongeum_init (DasomJeongeum *jeongeum)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings *settings;
  gchar     *layout, **hangul_keys;

  settings = g_settings_new ("org.freedesktop.Dasom.engines.jeongeum");
  layout   = g_settings_get_string (settings, "layout");
  hangul_keys = g_settings_get_strv (settings, "hangul-keys");

  jeongeum->hangul_keys = dasom_key_newv ((const gchar **) hangul_keys);
  jeongeum->context = hangul_ic_new (layout);
  jeongeum->en_name = g_strdup ("EN");
  jeongeum->ko_name = g_strdup ("정");
  jeongeum->is_english_mode = TRUE;
  jeongeum->hanja_table = hanja_table_load (NULL);
  jeongeum->candidate = dasom_candidate_new (); /* FIXME */
  g_signal_connect (jeongeum->candidate, "row-activated",
                    (GCallback) on_candidate_row_activated, jeongeum);

  g_object_unref (settings);
  g_free (layout);
  g_strfreev (hangul_keys);
}

static void
dasom_jeongeum_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (object);

  hanja_table_delete (jeongeum->hanja_table);
  hangul_ic_delete   (jeongeum->context);
  g_object_unref (jeongeum->candidate); /* FIXME */
  g_free (jeongeum->preedit_string);
  g_free (jeongeum->en_name);
  g_free (jeongeum->ko_name);
  dasom_key_freev (jeongeum->hangul_keys);

  G_OBJECT_CLASS (dasom_jeongeum_parent_class)->finalize (object);
}

void
dasom_jeongeum_get_preedit_string (DasomEngine  *engine,
                                   gchar       **str,
                                   gint         *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);

  if (str)
  {
    if (jeongeum->preedit_string)
      *str = g_strdup (jeongeum->preedit_string);
    else
      *str = g_strdup ("");
  }

  if (cursor_pos)
  {
    if (jeongeum->preedit_string)
      *cursor_pos = g_utf8_strlen (jeongeum->preedit_string, -1);
    else
      *cursor_pos = 0;
  }
}

const gchar *
dasom_jeongeum_get_name (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);

  return jeongeum->is_english_mode ? jeongeum->en_name : jeongeum->ko_name;
}

void
dasom_jeongeum_set_english_mode (DasomEngine *engine,
                                 gboolean     is_english_mode)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);

  if (jeongeum->is_english_mode != is_english_mode)
  {
    jeongeum->is_english_mode = is_english_mode;
    dasom_engine_emit_engine_changed (engine);
  }
}

gboolean
dasom_jeongeum_get_english_mode (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);
  return jeongeum->is_english_mode;
}

static void
dasom_jeongeum_class_init (DasomJeongeumClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  DasomEngineClass *engine_class = DASOM_ENGINE_CLASS (klass);

  engine_class->filter_event       = dasom_jeongeum_filter_event;
  engine_class->get_preedit_string = dasom_jeongeum_get_preedit_string;
  engine_class->reset              = dasom_jeongeum_reset;
  engine_class->focus_in           = dasom_jeongeum_focus_in;
  engine_class->focus_out          = dasom_jeongeum_focus_out;

  engine_class->get_name           = dasom_jeongeum_get_name;
  engine_class->set_english_mode   = dasom_jeongeum_set_english_mode;
  engine_class->get_english_mode   = dasom_jeongeum_get_english_mode;

  object_class->finalize = dasom_jeongeum_finalize;
}

static void
dasom_jeongeum_class_finalize (DasomJeongeumClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_load (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_jeongeum_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return dasom_jeongeum_get_type ();
}

void module_unload ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}
