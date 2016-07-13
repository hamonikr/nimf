/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-anthy.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2016 Hodong Kim <cogniti@gmail.com>
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

#include <nimf.h>
#include <anthy/anthy.h>
#include <glib/gi18n.h>

#define NIMF_TYPE_ANTHY             (nimf_anthy_get_type ())
#define NIMF_ANTHY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_ANTHY, NimfAnthy))
#define NIMF_ANTHY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_ANTHY, NimfAnthyClass))
#define NIMF_IS_ANTHY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_ANTHY))
#define NIMF_IS_ANTHY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_ANTHY))
#define NIMF_ANTHY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_ANTHY, NimfAnthyClass))

typedef struct _NimfAnthy      NimfAnthy;
typedef struct _NimfAnthyClass NimfAnthyClass;

struct _NimfAnthy
{
  NimfEngine parent_instance;

  GString          *preedit1;
  GString          *preedit2;
  NimfPreeditAttr **preedit_attrs;
  glong             offset;
  gchar            *id;

  anthy_context_t context;
};

struct _NimfAnthyClass
{
  /*< private >*/
  NimfEngineClass parent_class;
};

static gint        nimf_anthy_ref_count = 0;
static GHashTable *nimf_anthy_romaji = NULL;;

G_DEFINE_DYNAMIC_TYPE (NimfAnthy, nimf_anthy, NIMF_TYPE_ENGINE);

void
nimf_anthy_reset (NimfEngine  *engine,
                  NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfAnthy *anthy = NIMF_ANTHY (engine);

  nimf_engine_hide_candidate_window (engine);
  anthy->offset = 0;

  if (anthy->preedit1->len > 0 || anthy->preedit2->len > 0)
  {
    gchar *commit_str;

    commit_str = g_strjoin (NULL, anthy->preedit1->str,
                                  anthy->preedit2->str, NULL);
    nimf_engine_emit_commit (engine, target, commit_str);
    g_string_assign (anthy->preedit1, "");
    g_string_assign (anthy->preedit2, "");
    anthy->preedit_attrs[0]->start_index = 0;
    anthy->preedit_attrs[0]->end_index   = 0;
    anthy->preedit_attrs[1]->start_index = 0;
    anthy->preedit_attrs[1]->end_index   = 0;
    nimf_engine_emit_preedit_changed (engine, target, "", anthy->preedit_attrs, 0);
    nimf_engine_emit_preedit_end (engine, target);

    g_free (commit_str);
  }

  anthy_reset_context(NIMF_ANTHY (engine)->context);
}

void
nimf_anthy_focus_in (NimfEngine  *engine,
                     NimfContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
nimf_anthy_focus_out (NimfEngine  *engine,
                      NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_engine_hide_candidate_window (engine);
  nimf_anthy_reset (engine, target);
}

static void
on_candidate_clicked (NimfEngine  *engine,
                      NimfContext *target,
                      gchar       *text,
                      gint         index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfAnthy *anthy = NIMF_ANTHY (engine);
  gchar     *preedit_str;
  gchar     *tmp;
  struct anthy_conv_stat conv_stat;

  tmp = g_utf8_substring (anthy->preedit1->str, 0, anthy->offset);

  g_string_assign (anthy->preedit1, "");
  g_string_append (anthy->preedit1, tmp);
  g_string_append (anthy->preedit1, text);

  preedit_str = g_strjoin (NULL, anthy->preedit1->str,
                                 anthy->preedit2->str, NULL);
  anthy->preedit_attrs[0]->start_index = 0;
  anthy->preedit_attrs[0]->end_index   = g_utf8_strlen (preedit_str, -1);
  anthy->preedit_attrs[1]->start_index = 0;
  anthy->preedit_attrs[1]->end_index   = 0;
  nimf_engine_emit_preedit_changed (engine, target, preedit_str,
                                    anthy->preedit_attrs,
                                    g_utf8_strlen (preedit_str, -1));
  nimf_engine_hide_candidate_window (engine);

  anthy_get_stat (anthy->context, &conv_stat);
  anthy_commit_segment(anthy->context, conv_stat.nr_segment - 1, index);

  g_free (tmp);
  g_free (preedit_str);
}

gboolean
nimf_anthy_filter_event (NimfEngine  *engine,
                         NimfContext *target,
                         NimfEvent   *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfAnthy   *anthy = NIMF_ANTHY (engine);
  const gchar *str;
  gchar       *preedit_str;

  if (event->key.type == NIMF_EVENT_KEY_RELEASE)
    return FALSE;

  if (nimf_engine_is_candidate_window_visible (engine))
  {
    switch (event->key.keyval)
    {
      case NIMF_KEY_Return:
      case NIMF_KEY_KP_Enter:
      case NIMF_KEY_space:
        {
          gint index = nimf_engine_get_selected_candidate_index (engine);

          if (G_LIKELY (index >= 0))
          {
            gchar *text = nimf_engine_get_selected_candidate_text (engine);
            on_candidate_clicked (engine, target, text, index);
            g_free (text);

            return TRUE;
          }
        }
        break;
      case NIMF_KEY_Up:
      case NIMF_KEY_KP_Up:
        nimf_engine_select_previous_candidate_item (engine);
        return TRUE;
      case NIMF_KEY_Down:
      case NIMF_KEY_KP_Down:
        nimf_engine_select_next_candidate_item (engine);
        return TRUE;
      case NIMF_KEY_Page_Up:
      case NIMF_KEY_KP_Page_Up:
        nimf_engine_select_page_up_candidate_item (engine);
        return TRUE;
      case NIMF_KEY_Page_Down:
      case NIMF_KEY_KP_Page_Down:
        nimf_engine_select_page_down_candidate_item (engine);
        return TRUE;
      default:
        break;
    }
  }

  if ((event->key.keyval != ',' && event->key.keyval != '.') &&
      (event->key.keyval <  'a' || event->key.keyval == 'l'  ||
       event->key.keyval == 'q' || event->key.keyval == 'v'  ||
       event->key.keyval == 'x' || event->key.keyval >  'z'  ||
       event->key.keyval == NIMF_KEY_Return))
  {
    if (anthy->preedit1->len > 0 || anthy->preedit2->len > 0)
    {
      nimf_anthy_reset (engine, target);

      if (event->key.keyval == NIMF_KEY_Return)
        return TRUE;
    }

    return FALSE;
  }

  if (anthy->preedit1->len == 0 && anthy->preedit2->len == 0)
    nimf_engine_emit_preedit_start (engine, target);

  g_string_append_c (anthy->preedit2, event->key.keyval);

  while (TRUE)
  {
    str = g_hash_table_lookup (nimf_anthy_romaji, anthy->preedit2->str);

    if (str)
    {
      if (str[0] != 0)
      {
        g_string_append (anthy->preedit1, str);
        g_string_assign (anthy->preedit2, "");
      }

      break;
    }
    else
    {
      if (anthy->preedit2->len > 1)
      {
        static gchar c[2] = {0};

        g_string_append_len (anthy->preedit1, anthy->preedit2->str,
                             anthy->preedit2->len - 1);
        c[0] = anthy->preedit2->str[anthy->preedit2->len - 1];
        g_string_assign (anthy->preedit2, c);
      }
      else
      {
        g_string_append (anthy->preedit1, anthy->preedit2->str);
        g_string_assign (anthy->preedit2, "");

        break;
      }
    }
  }

  struct anthy_conv_stat    conv_stat;
  struct anthy_segment_stat segment_stat;
  gint  i;
  gchar buf[4096];

  anthy_set_string (anthy->context, anthy->preedit1->str);
  anthy_get_stat (anthy->context, &conv_stat);

  /* calculate offset */
  anthy->offset = 0;

  for (i = 0; i < conv_stat.nr_segment - 1; i++)
  {
    anthy_get_segment_stat (anthy->context, i, &segment_stat);
    anthy->offset += segment_stat.seg_len;
  }

  preedit_str = g_strjoin (NULL, anthy->preedit1->str,
                                 anthy->preedit2->str, NULL);
  anthy->preedit_attrs[0]->start_index = 0;
  anthy->preedit_attrs[0]->end_index = anthy->offset;
  anthy->preedit_attrs[1]->start_index = anthy->offset;
  anthy->preedit_attrs[1]->end_index = g_utf8_strlen (preedit_str, -1);
  nimf_engine_emit_preedit_changed (engine, target, preedit_str,
                                    anthy->preedit_attrs,
                                    g_utf8_strlen (preedit_str, -1));

  if (conv_stat.nr_segment > 0)
  {
    gchar **candidates;
    gint    i;

    anthy_get_segment_stat (anthy->context, conv_stat.nr_segment - 1,
                            &segment_stat);

    candidates = g_malloc0_n (segment_stat.nr_candidate + 1, sizeof (gchar *));

    for (i = 0; i < segment_stat.nr_candidate; i++)
    {
      anthy_get_segment (anthy->context, conv_stat.nr_segment - 1, i, buf, 4096);
      candidates[i] = g_strdup (buf);
    }

    candidates[segment_stat.nr_candidate] = NULL;
    nimf_engine_update_candidate_window (engine, (const gchar **) candidates);
    nimf_engine_show_candidate_window (engine, target, FALSE);

    g_strfreev (candidates);
  }

  g_free (preedit_str);

  return TRUE;
}

static void
nimf_anthy_init (NimfAnthy *anthy)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  anthy->id       = g_strdup ("nimf-anthy");
  anthy->preedit1 = g_string_new ("");
  anthy->preedit2 = g_string_new ("");
  anthy->preedit_attrs  = g_malloc0_n (3, sizeof (NimfPreeditAttr *));
  anthy->preedit_attrs[0] = nimf_preedit_attr_new (NIMF_PREEDIT_ATTR_UNDERLINE, 0, 0);
  anthy->preedit_attrs[1] = nimf_preedit_attr_new (NIMF_PREEDIT_ATTR_HIGHLIGHT, 0, 0);
  anthy->preedit_attrs[2] = NULL;

  if (nimf_anthy_romaji)
  {
    g_hash_table_ref (nimf_anthy_romaji);
  }
  else
  {
    nimf_anthy_romaji = g_hash_table_new_full (g_str_hash, g_str_equal,
                                               NULL, g_free);
    g_hash_table_insert (nimf_anthy_romaji, "a", g_strdup ("あ"));
    g_hash_table_insert (nimf_anthy_romaji, "b", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "ba", g_strdup ("ば"));
    g_hash_table_insert (nimf_anthy_romaji, "be", g_strdup ("べ"));
    g_hash_table_insert (nimf_anthy_romaji, "bi", g_strdup ("び"));
    g_hash_table_insert (nimf_anthy_romaji, "bo", g_strdup ("ぼ"));
    g_hash_table_insert (nimf_anthy_romaji, "bu", g_strdup ("ぶ"));
    g_hash_table_insert (nimf_anthy_romaji, "by", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "bya", g_strdup ("びゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "byo", g_strdup ("びょ"));
    g_hash_table_insert (nimf_anthy_romaji, "byu", g_strdup ("びゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "c", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "ch", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "cha", g_strdup ("ちゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "chi", g_strdup ("ち"));
    g_hash_table_insert (nimf_anthy_romaji, "cho", g_strdup ("ちょ"));
    g_hash_table_insert (nimf_anthy_romaji, "chu", g_strdup ("ちゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "d", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "da", g_strdup ("だ"));
    g_hash_table_insert (nimf_anthy_romaji, "de", g_strdup ("で"));
    g_hash_table_insert (nimf_anthy_romaji, "do", g_strdup ("ど"));
    g_hash_table_insert (nimf_anthy_romaji, "e", g_strdup ("え"));
    g_hash_table_insert (nimf_anthy_romaji, "f", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "fu", g_strdup ("ふ"));
    g_hash_table_insert (nimf_anthy_romaji, "g", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "ga", g_strdup ("が"));
    g_hash_table_insert (nimf_anthy_romaji, "ge", g_strdup ("げ"));
    g_hash_table_insert (nimf_anthy_romaji, "gi", g_strdup ("ぎ"));
    g_hash_table_insert (nimf_anthy_romaji, "go", g_strdup ("ご"));
    g_hash_table_insert (nimf_anthy_romaji, "gu", g_strdup ("ぐ"));
    g_hash_table_insert (nimf_anthy_romaji, "gy", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "gya", g_strdup ("ぎゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "gyo", g_strdup ("ぎょ"));
    g_hash_table_insert (nimf_anthy_romaji, "gyu", g_strdup ("ぎゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "h", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "ha", g_strdup ("は"));
    g_hash_table_insert (nimf_anthy_romaji, "he", g_strdup ("へ"));
    g_hash_table_insert (nimf_anthy_romaji, "hi", g_strdup ("ひ"));
    g_hash_table_insert (nimf_anthy_romaji, "ho", g_strdup ("ほ"));
    g_hash_table_insert (nimf_anthy_romaji, "hy", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "hya", g_strdup ("ひゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "hyo", g_strdup ("ひょ"));
    g_hash_table_insert (nimf_anthy_romaji, "hyu", g_strdup ("ひゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "i", g_strdup ("い"));
    g_hash_table_insert (nimf_anthy_romaji, "j", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "ja", g_strdup ("じゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "ja", g_strdup ("ぢゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "ji", g_strdup ("じ"));
    g_hash_table_insert (nimf_anthy_romaji, "ji", g_strdup ("ぢ"));
    g_hash_table_insert (nimf_anthy_romaji, "jo", g_strdup ("じょ"));
    g_hash_table_insert (nimf_anthy_romaji, "jo", g_strdup ("ぢょ"));
    g_hash_table_insert (nimf_anthy_romaji, "ju", g_strdup ("じゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "ju", g_strdup ("ぢゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "k", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "ka", g_strdup ("か"));
    g_hash_table_insert (nimf_anthy_romaji, "ke", g_strdup ("け"));
    g_hash_table_insert (nimf_anthy_romaji, "ki", g_strdup ("き"));
    g_hash_table_insert (nimf_anthy_romaji, "ko", g_strdup ("こ"));
    g_hash_table_insert (nimf_anthy_romaji, "ku", g_strdup ("く"));
    g_hash_table_insert (nimf_anthy_romaji, "ky", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "kya", g_strdup ("きゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "kyo", g_strdup ("きょ"));
    g_hash_table_insert (nimf_anthy_romaji, "kyu", g_strdup ("きゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "m", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "ma", g_strdup ("ま"));
    g_hash_table_insert (nimf_anthy_romaji, "me", g_strdup ("め"));
    g_hash_table_insert (nimf_anthy_romaji, "mi", g_strdup ("み"));
    g_hash_table_insert (nimf_anthy_romaji, "mo", g_strdup ("も"));
    g_hash_table_insert (nimf_anthy_romaji, "mu", g_strdup ("む"));
    g_hash_table_insert (nimf_anthy_romaji, "my", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "mya", g_strdup ("みゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "myo", g_strdup ("みょ"));
    g_hash_table_insert (nimf_anthy_romaji, "myu", g_strdup ("みゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "n", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "na", g_strdup ("な"));
    g_hash_table_insert (nimf_anthy_romaji, "ne", g_strdup ("ね"));
    g_hash_table_insert (nimf_anthy_romaji, "ni", g_strdup ("に"));
    g_hash_table_insert (nimf_anthy_romaji, "nn", g_strdup ("ん"));
    g_hash_table_insert (nimf_anthy_romaji, "no", g_strdup ("の"));
    g_hash_table_insert (nimf_anthy_romaji, "nu", g_strdup ("ぬ"));
    g_hash_table_insert (nimf_anthy_romaji, "ny", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "nya", g_strdup ("にゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "nyo", g_strdup ("にょ"));
    g_hash_table_insert (nimf_anthy_romaji, "nyu", g_strdup ("にゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "o", g_strdup ("お"));
    g_hash_table_insert (nimf_anthy_romaji, "p", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "pa", g_strdup ("ぱ"));
    g_hash_table_insert (nimf_anthy_romaji, "pe", g_strdup ("ぺ"));
    g_hash_table_insert (nimf_anthy_romaji, "pi", g_strdup ("ぴ"));
    g_hash_table_insert (nimf_anthy_romaji, "po", g_strdup ("ぽ"));
    g_hash_table_insert (nimf_anthy_romaji, "pu", g_strdup ("ぷ"));
    g_hash_table_insert (nimf_anthy_romaji, "py", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "pya", g_strdup ("ぴゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "pyo", g_strdup ("ぴょ"));
    g_hash_table_insert (nimf_anthy_romaji, "pyu", g_strdup ("ぴゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "r", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "ra", g_strdup ("ら"));
    g_hash_table_insert (nimf_anthy_romaji, "re", g_strdup ("れ"));
    g_hash_table_insert (nimf_anthy_romaji, "ri", g_strdup ("り"));
    g_hash_table_insert (nimf_anthy_romaji, "ro", g_strdup ("ろ"));
    g_hash_table_insert (nimf_anthy_romaji, "ru", g_strdup ("る"));
    g_hash_table_insert (nimf_anthy_romaji, "ry", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "rya", g_strdup ("りゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "ryo", g_strdup ("りょ"));
    g_hash_table_insert (nimf_anthy_romaji, "ryu", g_strdup ("りゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "s", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "sa", g_strdup ("さ"));
    g_hash_table_insert (nimf_anthy_romaji, "se", g_strdup ("せ"));
    g_hash_table_insert (nimf_anthy_romaji, "sh", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "sha", g_strdup ("しゃ"));
    g_hash_table_insert (nimf_anthy_romaji, "shi", g_strdup ("し"));
    g_hash_table_insert (nimf_anthy_romaji, "sho", g_strdup ("しょ"));
    g_hash_table_insert (nimf_anthy_romaji, "shu", g_strdup ("しゅ"));
    g_hash_table_insert (nimf_anthy_romaji, "so", g_strdup ("そ"));
    g_hash_table_insert (nimf_anthy_romaji, "su", g_strdup ("す"));
    g_hash_table_insert (nimf_anthy_romaji, "t", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "ta", g_strdup ("た"));
    g_hash_table_insert (nimf_anthy_romaji, "te", g_strdup ("て"));
    g_hash_table_insert (nimf_anthy_romaji, "to", g_strdup ("と"));
    g_hash_table_insert (nimf_anthy_romaji, "ts", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "tsu", g_strdup ("つ"));
    g_hash_table_insert (nimf_anthy_romaji, "u", g_strdup ("う"));
    g_hash_table_insert (nimf_anthy_romaji, "w", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "wa", g_strdup ("わ"));
    g_hash_table_insert (nimf_anthy_romaji, "we", g_strdup ("ゑ"));
    g_hash_table_insert (nimf_anthy_romaji, "wi", g_strdup ("ゐ"));
    g_hash_table_insert (nimf_anthy_romaji, "wo", g_strdup ("を"));
    g_hash_table_insert (nimf_anthy_romaji, "y", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "ya", g_strdup ("や"));
    g_hash_table_insert (nimf_anthy_romaji, "ya", g_strdup ("や"));
    g_hash_table_insert (nimf_anthy_romaji, "yo", g_strdup ("よ"));
    g_hash_table_insert (nimf_anthy_romaji, "yo", g_strdup ("よ"));
    g_hash_table_insert (nimf_anthy_romaji, "yu", g_strdup ("ゆ"));
    g_hash_table_insert (nimf_anthy_romaji, "yu", g_strdup ("ゆ"));
    g_hash_table_insert (nimf_anthy_romaji, "z", g_strdup ("")); /* dummy */
    g_hash_table_insert (nimf_anthy_romaji, "za", g_strdup ("ざ"));
    g_hash_table_insert (nimf_anthy_romaji, "ze", g_strdup ("ぜ"));
    g_hash_table_insert (nimf_anthy_romaji, "zo", g_strdup ("ぞ"));
    g_hash_table_insert (nimf_anthy_romaji, "zu", g_strdup ("ず"));
    g_hash_table_insert (nimf_anthy_romaji, "zu", g_strdup ("づ"));
    g_hash_table_insert (nimf_anthy_romaji, ",", g_strdup ("、"));
    g_hash_table_insert (nimf_anthy_romaji, ".", g_strdup ("。"));
  }

  if (anthy_init () < 0)
    g_error (G_STRLOC ": %s: anthy is not initialized", G_STRFUNC);

  anthy->context = anthy_create_context ();
  nimf_anthy_ref_count++;
  anthy_context_set_encoding(anthy->context, ANTHY_UTF8_ENCODING);
}

static void
nimf_anthy_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfAnthy *anthy = NIMF_ANTHY (object);

  g_string_free (anthy->preedit1, TRUE);
  g_string_free (anthy->preedit2, TRUE);
  nimf_preedit_attr_freev (anthy->preedit_attrs);
  g_free (anthy->id);
  g_hash_table_unref (nimf_anthy_romaji);

  if (--nimf_anthy_ref_count == 0)
  {
    anthy_release_context (anthy->context);
    anthy_quit ();
  }

  G_OBJECT_CLASS (nimf_anthy_parent_class)->finalize (object);
}

void
nimf_anthy_get_preedit_string (NimfEngine        *engine,
                               gchar            **str,
                               NimfPreeditAttr ***attrs,
                               gint              *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfAnthy *anthy = NIMF_ANTHY (engine);

  if (str)
    *str = g_strjoin (NULL, anthy->preedit1->str, anthy->preedit2->str, NULL);

  if (attrs)
    *attrs = nimf_preedit_attrs_copy (anthy->preedit_attrs);

  if (cursor_pos)
    *cursor_pos = g_utf8_strlen (anthy->preedit1->str, -1) +
                  g_utf8_strlen (anthy->preedit2->str, -1);
}

const gchar *
nimf_anthy_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_ANTHY (engine)->id;
}

const gchar *
nimf_anthy_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_ANTHY (engine)->id;
}

static void
nimf_anthy_class_init (NimfAnthyClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  NimfEngineClass *engine_class = NIMF_ENGINE_CLASS (class);

  engine_class->filter_event       = nimf_anthy_filter_event;
  engine_class->get_preedit_string = nimf_anthy_get_preedit_string;
  engine_class->reset              = nimf_anthy_reset;
  engine_class->focus_in           = nimf_anthy_focus_in;
  engine_class->focus_out          = nimf_anthy_focus_out;

  engine_class->candidate_clicked  = on_candidate_clicked;

  engine_class->get_id             = nimf_anthy_get_id;
  engine_class->get_icon_name      = nimf_anthy_get_icon_name;

  object_class->finalize = nimf_anthy_finalize;
}

static void
nimf_anthy_class_finalize (NimfAnthyClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_anthy_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_anthy_get_type ();
}
