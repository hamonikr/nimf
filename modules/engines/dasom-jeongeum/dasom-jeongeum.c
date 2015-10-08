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

#include "dasom-jeongeum.h"
#include "modules/engines/dasom-english/dasom-english.h"
#include <stdlib.h>

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

static void
dasom_jeongeum_update_preedit (DasomEngine     *engine,
                               DasomConnection *target,
                               gchar           *new_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);

  /* preedit-start */
  if (jeongeum->preedit_state == DASOM_PREEDIT_STATE_END &&
      new_preedit[0] != 0)
  {
    jeongeum->preedit_state = DASOM_PREEDIT_STATE_START;
    dasom_engine_emit_preedit_start (engine, target);
  }

  /* preedit-changed */
  if (g_strcmp0 (jeongeum->preedit_string, new_preedit) != 0)
  {
    g_free (jeongeum->preedit_string);
    jeongeum->preedit_string = new_preedit;
    dasom_engine_emit_preedit_changed (engine, target);
  }
  else
    g_free (new_preedit);

  /* preedit-end */
  if (jeongeum->preedit_state == DASOM_PREEDIT_STATE_START &&
      jeongeum->preedit_string[0] == 0)
  {
    jeongeum->preedit_state = DASOM_PREEDIT_STATE_END;
    dasom_engine_emit_preedit_end (engine, target);
  }
}

void
dasom_jeongeum_reset (DasomEngine *engine, DasomConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);

  dasom_engine_hide_candidate_window (engine);
  jeongeum->is_candidate_mode = FALSE;

  const ucschar *flush;
  flush = hangul_ic_flush (jeongeum->context);

  if (flush[0] == 0)
    return;

  dasom_jeongeum_update_preedit (engine, target, g_strdup (""));

  gchar *text = g_ucs4_to_utf8 (flush, -1, NULL, NULL, NULL);
  dasom_engine_emit_commit (engine, target, text);
  g_free (text);
}

void
dasom_jeongeum_focus_in (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));
}

void
dasom_jeongeum_focus_out (DasomEngine *engine, DasomConnection  *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (DASOM_IS_ENGINE (engine));

  dasom_jeongeum_reset (engine, target);
}

static void
on_candidate_clicked (DasomEngine *engine, DasomConnection *target, gchar *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);

  if (text)
  {
    /* hangul_ic 내부의 commit text가 사라집니다 */
    hangul_ic_reset (jeongeum->context);
    dasom_jeongeum_update_preedit (engine, target, g_strdup (""));
    dasom_engine_emit_commit (DASOM_ENGINE (jeongeum), target, text);
  }

  dasom_engine_hide_candidate_window (DASOM_ENGINE (jeongeum));
  jeongeum->is_candidate_mode = FALSE;
}

gboolean
dasom_jeongeum_filter_event (DasomEngine     *engine,
                             DasomConnection *target,
                             DasomEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  guint    keyval;
  gboolean retval = FALSE;

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);

  if (event->key.type   == DASOM_EVENT_KEY_RELEASE ||
      event->key.keyval == DASOM_KEY_Shift_L       ||
      event->key.keyval == DASOM_KEY_Shift_R)
    return FALSE;

  if (event->key.state & (DASOM_CONTROL_MASK | DASOM_MOD1_MASK))
  {
    dasom_jeongeum_reset (engine, target);
    return FALSE;
  }

  if (dasom_event_matches (event, (const DasomKey **) jeongeum->hangul_keys))
  {
    dasom_jeongeum_reset (engine, target);
    jeongeum->is_english_mode = !jeongeum->is_english_mode;
    dasom_engine_emit_engine_changed (engine, target);
    return TRUE;
  }

  if (jeongeum->is_english_mode)
    return dasom_english_filter_event (engine, target, event);

  if (G_UNLIKELY (dasom_event_matches (event,
                  (const DasomKey **) jeongeum->hanja_keys)))
  {
    if (jeongeum->is_candidate_mode == FALSE)
    {
      jeongeum->is_candidate_mode = TRUE;
      HanjaList *list = hanja_table_match_exact (jeongeum->hanja_table,
                                                 jeongeum->preedit_string);
      if (list == NULL)
        list = hanja_table_match_exact (jeongeum->symbol_table,
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

      dasom_engine_update_candidate_window (engine, (const gchar **) strv);
      g_strfreev (strv);
      dasom_engine_show_candidate_window (engine, target);
    }
    else
    {
      jeongeum->is_candidate_mode = FALSE;
      dasom_engine_hide_candidate_window (engine);
    }

    return TRUE;
  }

  if (G_UNLIKELY (jeongeum->is_candidate_mode))
  {
    switch (event->key.keyval)
    {
      case DASOM_KEY_Return:
      case DASOM_KEY_KP_Enter:
        {
          gchar *text = dasom_engine_get_selected_candidate_text (engine);
          on_candidate_clicked (engine, target, text);
          g_free (text);
        }
        break;
      case DASOM_KEY_Up:
      case DASOM_KEY_KP_Up:
        dasom_engine_select_previous_candidate_item (engine);
        break;
      case DASOM_KEY_Down:
      case DASOM_KEY_KP_Down:
        dasom_engine_select_next_candidate_item (engine);
        break;
      case DASOM_KEY_Page_Up:
      case DASOM_KEY_KP_Page_Up:
        dasom_engine_select_page_up_candidate_item (engine);
        break;
      case DASOM_KEY_Page_Down:
      case DASOM_KEY_KP_Page_Down:
        dasom_engine_select_page_down_candidate_item (engine);
        break;
      case DASOM_KEY_Escape:
        dasom_engine_hide_candidate_window (engine);
        jeongeum->is_candidate_mode = FALSE;
        break;
      default:
        break;
    }

    return TRUE;
  }

  const ucschar *ucs_commit;
  const ucschar *ucs_preedit;

  if  (G_UNLIKELY (event->key.keyval == DASOM_KEY_BackSpace ||
                   event->key.keyval == DASOM_KEY_Delete    ||
                   event->key.keyval == DASOM_KEY_KP_Delete))
  {
    retval = hangul_ic_backspace (jeongeum->context);

    if (retval)
    {
      ucs_preedit = hangul_ic_get_preedit_string (jeongeum->context);
      gchar *new_preedit = g_ucs4_to_utf8 (ucs_preedit, -1, NULL, NULL, NULL);
      dasom_jeongeum_update_preedit (engine, target, new_preedit);
    }

    return retval;
  }

  keyval = dasom_event_keycode_to_qwerty_keyval (event);
  retval = hangul_ic_process (jeongeum->context, keyval);

  ucs_commit  = hangul_ic_get_commit_string  (jeongeum->context);
  ucs_preedit = hangul_ic_get_preedit_string (jeongeum->context);

  gchar *new_commit  = g_ucs4_to_utf8 (ucs_commit,  -1, NULL, NULL, NULL);

  /* commit */
  if (ucs_commit[0] != 0)
  {
    /* clear preedit string before commit */
    /* 이유1. ㅁ 을 연속하여 누를 경우 preedit 가 동일하여 preedit-changed 신호가
     * 발생되지 않기 때문에.
     * 이유2. 커밋 전에 조합 중인 문자열을 clear 하는 것이 논리적으로 맞는 것
     * 같기 때문에.
     */
    dasom_jeongeum_update_preedit (engine, target, g_strdup (""));
    dasom_engine_emit_commit (engine, target, new_commit);
  }

  g_free (new_commit);

  gchar *new_preedit = g_ucs4_to_utf8 (ucs_preedit, -1, NULL, NULL, NULL);
  dasom_jeongeum_update_preedit (engine, target, new_preedit);

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
    dasom_engine_emit_commit (engine, target, str);
    g_free (str);
    retval = TRUE;
  }

  return retval;
}

static void
on_layout_changed (GSettings     *settings,
                   gchar         *key,
                   DasomJeongeum *jeongeum)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar *layout = g_settings_get_string (settings, key);
  hangul_ic_select_keyboard (jeongeum->context, layout);
  g_free (layout);
}

static void
on_keys_changed (GSettings     *settings,
                 gchar         *key,
                 DasomJeongeum *jeongeum)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **keys = g_settings_get_strv (settings, key);

  if (g_strcmp0 (key, "hangul-keys") == 0)
  {
    dasom_key_freev (jeongeum->hangul_keys);
    jeongeum->hangul_keys = dasom_key_newv ((const gchar **) keys);
  }
  else if (g_strcmp0 (key, "hanja-keys") == 0)
  {
    dasom_key_freev (jeongeum->hanja_keys);
    jeongeum->hanja_keys = dasom_key_newv ((const gchar **) keys);
  }

  g_strfreev (keys);
}

static void
on_kr_101_104_key_compatible_changed (GSettings     *settings,
                                      gchar         *key,
                                      gpointer       user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  int retval G_GNUC_UNUSED;

  if (g_settings_get_boolean (settings, key))
    retval = system ("xmodmap -e 'keycode 105 = Hangul_Hanja' && \
                      xmodmap -e 'keycode 108 = Hangul' && \
                      xmodmap -e 'remove mod1 = Hangul' && \
                      xmodmap -e 'remove control = Hangul_Hanja'");
  else
    retval = system ("xmodmap -e 'keycode 105 = Control_R' && \
                      xmodmap -e 'keycode 108 = Alt_R' && \
                      xmodmap -e 'add mod1 = Alt_R' && \
                      xmodmap -e 'add control = Control_R'");
}

static gboolean on_timeout (GSettings *settings)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  static guint times = 0;

  on_kr_101_104_key_compatible_changed (settings,
                                        "korean-101-104-key-compatible", NULL);
  times++;

  if (times == 5)
  {
    times = 0;
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}

static void
dasom_jeongeum_init (DasomJeongeum *jeongeum)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar *layout, **hangul_keys, **hanja_keys;

  jeongeum->settings = g_settings_new ("org.freedesktop.Dasom.engines.jeongeum");

  layout      = g_settings_get_string (jeongeum->settings, "layout");
  hangul_keys = g_settings_get_strv   (jeongeum->settings, "hangul-keys");
  hanja_keys  = g_settings_get_strv   (jeongeum->settings, "hanja-keys");

  jeongeum->hangul_keys = dasom_key_newv ((const gchar **) hangul_keys);
  jeongeum->hanja_keys  = dasom_key_newv ((const gchar **) hanja_keys);
  jeongeum->context = hangul_ic_new (layout);
  jeongeum->en_name = g_strdup ("EN");
  jeongeum->ko_name = g_strdup ("정");
  jeongeum->is_english_mode = TRUE;
  jeongeum->hanja_table  = hanja_table_load (NULL);
  jeongeum->symbol_table = hanja_table_load ("/usr/share/libhangul/hanja/mssymbol.txt"); /* FIXME */

  g_free (layout);
  g_strfreev (hangul_keys);
  g_strfreev (hanja_keys);

  /* FIXME: workaround for xkeyboard-config < 2.14
   * https://bugs.freedesktop.org/show_bug.cgi?id=84404
   *
   * xmodmap 의 정확한 적용 시점을 모르겠습니다.
   * .xprofile .profile .xinitrc .bashrc 에 xmodmap 설정을 넣어도 이상하게도
   * gnome 에서 이것이 적용되지 않기 때문에, 그래서 dasom-daemon 이 실행하면서
   * xmodmap 을 맨 처음 1번만 실행하도록 만들었었는데 맨 처음 1번만 실행해서는
   * 이것이 적용되지 않기 때문에 몇 초 간격으로 5번을 실행합니다.
   * xmodmap 실헹에 소요되는 시간은 약 0.01초입니다. 따라서 thread 를 도입할
   * 필요는 없습니다.
   *
   * xkeyboard-config >= 2.14 를 사용하시는 경우에는 dasom-daemon 이 아닌
   * 시스템 설정에서 xkb 옵션을 설정하실 것을 권장합니다.
   */
  if (g_settings_get_boolean (jeongeum->settings,
                              "korean-101-104-key-compatible"))
    g_timeout_add_seconds (2, (GSourceFunc) on_timeout, jeongeum->settings);

  g_signal_connect (jeongeum->settings, "changed::layout",
                    G_CALLBACK (on_layout_changed), jeongeum);
  g_signal_connect (jeongeum->settings, "changed::hangul-keys",
                    G_CALLBACK (on_keys_changed), jeongeum);
  g_signal_connect (jeongeum->settings, "changed::hanja-keys",
                    G_CALLBACK (on_keys_changed), jeongeum);
  g_signal_connect (jeongeum->settings,
                    "changed::korean-101-104-key-compatible",
                    G_CALLBACK (on_kr_101_104_key_compatible_changed), NULL);
}

static void
dasom_jeongeum_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (object);

  hanja_table_delete (jeongeum->hanja_table);
  hanja_table_delete (jeongeum->symbol_table);
  hangul_ic_delete   (jeongeum->context);
  g_free (jeongeum->preedit_string);
  g_free (jeongeum->en_name);
  g_free (jeongeum->ko_name);
  dasom_key_freev (jeongeum->hangul_keys);
  dasom_key_freev (jeongeum->hanja_keys);
  g_object_unref (jeongeum->settings);

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

  DASOM_JEONGEUM (engine)->is_english_mode = is_english_mode;
}

gboolean
dasom_jeongeum_get_english_mode (DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomJeongeum *jeongeum = DASOM_JEONGEUM (engine);
  return jeongeum->is_english_mode;
}

static void
dasom_jeongeum_class_init (DasomJeongeumClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  DasomEngineClass *engine_class = DASOM_ENGINE_CLASS (class);

  engine_class->filter_event       = dasom_jeongeum_filter_event;
  engine_class->get_preedit_string = dasom_jeongeum_get_preedit_string;
  engine_class->reset              = dasom_jeongeum_reset;
  engine_class->focus_in           = dasom_jeongeum_focus_in;
  engine_class->focus_out          = dasom_jeongeum_focus_out;

  engine_class->candidate_clicked  = on_candidate_clicked;

  engine_class->get_name           = dasom_jeongeum_get_name;
  engine_class->set_english_mode   = dasom_jeongeum_set_english_mode;
  engine_class->get_english_mode   = dasom_jeongeum_get_english_mode;

  object_class->finalize = dasom_jeongeum_finalize;
}

static void
dasom_jeongeum_class_finalize (DasomJeongeumClass *class)
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
