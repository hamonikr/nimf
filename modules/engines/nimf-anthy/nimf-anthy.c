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
  GHashTable       *romaji;

  anthy_context_t context;
};

struct _NimfAnthyClass
{
  /*< private >*/
  NimfEngineClass parent_class;
};

static gint anthy_ref_count = 0;

G_DEFINE_DYNAMIC_TYPE (NimfAnthy, nimf_anthy, NIMF_TYPE_ENGINE);

void
nimf_anthy_reset (NimfEngine  *engine,
                  NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfAnthy *anthy = NIMF_ANTHY (engine);

  nimf_engine_hide_candidate_window (engine);
  anthy->offset = 0;

  if (g_utf8_strlen (anthy->preedit1->str, -1) > 0 ||
      g_utf8_strlen (anthy->preedit2->str, -1) > 0)
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
  gchar      **strv = NULL;

  if (event->key.type == NIMF_EVENT_KEY_RELEASE)
    return FALSE;

  switch (event->key.keyval)
  {
    case NIMF_KEY_Return:
    case NIMF_KEY_KP_Enter:
    case NIMF_KEY_space:
      {
        if (nimf_engine_is_candidate_window_visible (engine) == FALSE)
          break;

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
      if (!nimf_engine_is_candidate_window_visible (engine))
        return FALSE;
      nimf_engine_select_previous_candidate_item (engine);
      return TRUE;
    case NIMF_KEY_Down:
    case NIMF_KEY_KP_Down:
      if (!nimf_engine_is_candidate_window_visible (engine))
        return FALSE;
      nimf_engine_select_next_candidate_item (engine);
      return TRUE;
    default:
      break;
  }

  if ((event->key.keyval != ',' && event->key.keyval != '.') &&
      (event->key.keyval <  'a' || event->key.keyval == 'l'  ||
       event->key.keyval == 'q' || event->key.keyval == 'v'  ||
       event->key.keyval == 'x' || event->key.keyval >  'z'  ||
       event->key.keyval == NIMF_KEY_Return))
  {
    if (g_utf8_strlen (anthy->preedit1->str, -1) > 0 ||
        g_utf8_strlen (anthy->preedit2->str, -1) > 0)
    {
      nimf_anthy_reset (engine, target);

      if (event->key.keyval == NIMF_KEY_Return)
        return TRUE;
    }

    return FALSE;
  }

  if (g_utf8_strlen (anthy->preedit1->str, -1) == 0 &&
      g_utf8_strlen (anthy->preedit2->str, -1) == 0)
    nimf_engine_emit_preedit_start (engine, target);

  g_string_append_unichar (anthy->preedit2, event->key.keyval);

  while (TRUE)
  {
    str = g_hash_table_lookup (anthy->romaji, anthy->preedit2->str);

    if (str)
    {
      if (str[0] != 0)
      {
        strv = g_strsplit (str, "\x1e", -1);
        g_string_append (anthy->preedit1, strv[0]);
        g_string_assign (anthy->preedit2, "");
      }

      break;
    }
    else
    {
      if (anthy->preedit2->len > 1)
      {
        gint  i;
        gchar c;

        for (i = 0; i < anthy->preedit2->len - 1; i++)
          g_string_append_c (anthy->preedit1, anthy->preedit2->str[i]);

        c = anthy->preedit2->str[anthy->preedit2->len - 1];
        g_string_assign (anthy->preedit2, "");
        g_string_append_c (anthy->preedit2, c);
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

  g_strfreev (strv);
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
  anthy->romaji   = g_hash_table_new_full (g_str_hash, g_str_equal,
                                          NULL, g_free);
  g_hash_table_insert (anthy->romaji, "a", g_strdup ("あ\x1eア"));
  g_hash_table_insert (anthy->romaji, "b", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "ba", g_strdup ("ば\x1eバ"));
  g_hash_table_insert (anthy->romaji, "be", g_strdup ("べ\x1eベ"));
  g_hash_table_insert (anthy->romaji, "bi", g_strdup ("び\x1eビ"));
  g_hash_table_insert (anthy->romaji, "bo", g_strdup ("ぼ\x1eボ"));
  g_hash_table_insert (anthy->romaji, "bu", g_strdup ("ぶ\x1eブ"));
  g_hash_table_insert (anthy->romaji, "by", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "bya", g_strdup ("びゃ\x1eビャ"));
  g_hash_table_insert (anthy->romaji, "byo", g_strdup ("びょ\x1eビョ"));
  g_hash_table_insert (anthy->romaji, "byu", g_strdup ("びゅ\x1eビュ"));
  g_hash_table_insert (anthy->romaji, "c", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "ch", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "cha", g_strdup ("ちゃ\x1eチャ"));
  g_hash_table_insert (anthy->romaji, "chi", g_strdup ("ち\x1eチ"));
  g_hash_table_insert (anthy->romaji, "cho", g_strdup ("ちょ\x1eチョ"));
  g_hash_table_insert (anthy->romaji, "chu", g_strdup ("ちゅ\x1eチュ"));
  g_hash_table_insert (anthy->romaji, "d", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "da", g_strdup ("だ\x1eダ"));
  g_hash_table_insert (anthy->romaji, "de", g_strdup ("で\x1eデ"));
  g_hash_table_insert (anthy->romaji, "do", g_strdup ("ど\x1eド"));
  g_hash_table_insert (anthy->romaji, "e", g_strdup ("え\x1eエ"));
  g_hash_table_insert (anthy->romaji, "f", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "fu", g_strdup ("ふ\x1eフ"));
  g_hash_table_insert (anthy->romaji, "g", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "ga", g_strdup ("が\x1eガ"));
  g_hash_table_insert (anthy->romaji, "ge", g_strdup ("げ\x1eゲ"));
  g_hash_table_insert (anthy->romaji, "gi", g_strdup ("ぎ\x1eギ"));
  g_hash_table_insert (anthy->romaji, "go", g_strdup ("ご\x1eゴ"));
  g_hash_table_insert (anthy->romaji, "gu", g_strdup ("ぐ\x1eグ"));
  g_hash_table_insert (anthy->romaji, "gy", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "gya", g_strdup ("ぎゃ\x1eギャ"));
  g_hash_table_insert (anthy->romaji, "gyo", g_strdup ("ぎょ\x1eギョ"));
  g_hash_table_insert (anthy->romaji, "gyu", g_strdup ("ぎゅ\x1eギュ"));
  g_hash_table_insert (anthy->romaji, "h", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "ha", g_strdup ("は\x1eハ"));
  g_hash_table_insert (anthy->romaji, "he", g_strdup ("へ\x1eヘ"));
  g_hash_table_insert (anthy->romaji, "hi", g_strdup ("ひ\x1eヒ"));
  g_hash_table_insert (anthy->romaji, "ho", g_strdup ("ほ\x1eホ"));
  g_hash_table_insert (anthy->romaji, "hy", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "hya", g_strdup ("ひゃ\x1eヒャ"));
  g_hash_table_insert (anthy->romaji, "hyo", g_strdup ("ひょ\x1eヒョ"));
  g_hash_table_insert (anthy->romaji, "hyu", g_strdup ("ひゅ\x1eヒュ"));
  g_hash_table_insert (anthy->romaji, "i", g_strdup ("い\x1eイ"));
  g_hash_table_insert (anthy->romaji, "j", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "ja", g_strdup ("じゃ\x1eジャ"));
  g_hash_table_insert (anthy->romaji, "ja", g_strdup ("ぢゃ\x1eヂャ"));
  g_hash_table_insert (anthy->romaji, "ji", g_strdup ("じ\x1eジ"));
  g_hash_table_insert (anthy->romaji, "ji", g_strdup ("ぢ\x1eヂ"));
  g_hash_table_insert (anthy->romaji, "jo", g_strdup ("じょ\x1eジョ"));
  g_hash_table_insert (anthy->romaji, "jo", g_strdup ("ぢょ\x1eヂョ"));
  g_hash_table_insert (anthy->romaji, "ju", g_strdup ("じゅ\x1eジュ"));
  g_hash_table_insert (anthy->romaji, "ju", g_strdup ("ぢゅ\x1eヂュ"));
  g_hash_table_insert (anthy->romaji, "k", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "ka", g_strdup ("か\x1eカ"));
  g_hash_table_insert (anthy->romaji, "ke", g_strdup ("け\x1eケ"));
  g_hash_table_insert (anthy->romaji, "ki", g_strdup ("き\x1eキ"));
  g_hash_table_insert (anthy->romaji, "ko", g_strdup ("こ\x1eコ"));
  g_hash_table_insert (anthy->romaji, "ku", g_strdup ("く\x1eク"));
  g_hash_table_insert (anthy->romaji, "ky", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "kya", g_strdup ("きゃ\x1eキャ"));
  g_hash_table_insert (anthy->romaji, "kyo", g_strdup ("きょ\x1eキョ"));
  g_hash_table_insert (anthy->romaji, "kyu", g_strdup ("きゅ\x1eキュ"));
  g_hash_table_insert (anthy->romaji, "m", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "ma", g_strdup ("ま\x1eマ"));
  g_hash_table_insert (anthy->romaji, "me", g_strdup ("め\x1eメ"));
  g_hash_table_insert (anthy->romaji, "mi", g_strdup ("み\x1eミ"));
  g_hash_table_insert (anthy->romaji, "mo", g_strdup ("も\x1eモ"));
  g_hash_table_insert (anthy->romaji, "mu", g_strdup ("む\x1eム"));
  g_hash_table_insert (anthy->romaji, "my", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "mya", g_strdup ("みゃ\x1eミャ"));
  g_hash_table_insert (anthy->romaji, "myo", g_strdup ("みょ\x1eミョ"));
  g_hash_table_insert (anthy->romaji, "myu", g_strdup ("みゅ\x1eミュ"));
  g_hash_table_insert (anthy->romaji, "n", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "na", g_strdup ("な\x1eナ"));
  g_hash_table_insert (anthy->romaji, "ne", g_strdup ("ね\x1eネ"));
  g_hash_table_insert (anthy->romaji, "ni", g_strdup ("に\x1eニ"));
  g_hash_table_insert (anthy->romaji, "nn", g_strdup ("ん\x1eン"));
  g_hash_table_insert (anthy->romaji, "no", g_strdup ("の\x1eノ"));
  g_hash_table_insert (anthy->romaji, "nu", g_strdup ("ぬ\x1eヌ"));
  g_hash_table_insert (anthy->romaji, "ny", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "nya", g_strdup ("にゃ\x1eニャ"));
  g_hash_table_insert (anthy->romaji, "nyo", g_strdup ("にょ\x1eニョ"));
  g_hash_table_insert (anthy->romaji, "nyu", g_strdup ("にゅ\x1eニュ"));
  g_hash_table_insert (anthy->romaji, "o", g_strdup ("お\x1eオ"));
  g_hash_table_insert (anthy->romaji, "p", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "pa", g_strdup ("ぱ\x1eパ"));
  g_hash_table_insert (anthy->romaji, "pe", g_strdup ("ぺ\x1eペ"));
  g_hash_table_insert (anthy->romaji, "pi", g_strdup ("ぴ\x1eピ"));
  g_hash_table_insert (anthy->romaji, "po", g_strdup ("ぽ\x1eポ"));
  g_hash_table_insert (anthy->romaji, "pu", g_strdup ("ぷ\x1eプ"));
  g_hash_table_insert (anthy->romaji, "py", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "pya", g_strdup ("ぴゃ\x1eピャ"));
  g_hash_table_insert (anthy->romaji, "pyo", g_strdup ("ぴょ\x1eピョ"));
  g_hash_table_insert (anthy->romaji, "pyu", g_strdup ("ぴゅ\x1eピュ"));
  g_hash_table_insert (anthy->romaji, "r", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "ra", g_strdup ("ら\x1eラ"));
  g_hash_table_insert (anthy->romaji, "re", g_strdup ("れ\x1eレ"));
  g_hash_table_insert (anthy->romaji, "ri", g_strdup ("り\x1eリ"));
  g_hash_table_insert (anthy->romaji, "ro", g_strdup ("ろ\x1eロ"));
  g_hash_table_insert (anthy->romaji, "ru", g_strdup ("る\x1eル"));
  g_hash_table_insert (anthy->romaji, "ry", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "rya", g_strdup ("りゃ\x1eリャ"));
  g_hash_table_insert (anthy->romaji, "ryo", g_strdup ("りょ\x1eリョ"));
  g_hash_table_insert (anthy->romaji, "ryu", g_strdup ("りゅ\x1eリュ"));
  g_hash_table_insert (anthy->romaji, "s", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "sa", g_strdup ("さ\x1eサ"));
  g_hash_table_insert (anthy->romaji, "se", g_strdup ("せ\x1eセ"));
  g_hash_table_insert (anthy->romaji, "sh", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "sha", g_strdup ("しゃ\x1eシャ"));
  g_hash_table_insert (anthy->romaji, "shi", g_strdup ("し\x1eシ"));
  g_hash_table_insert (anthy->romaji, "sho", g_strdup ("しょ\x1eショ"));
  g_hash_table_insert (anthy->romaji, "shu", g_strdup ("しゅ\x1eシュ"));
  g_hash_table_insert (anthy->romaji, "so", g_strdup ("そ\x1eソ"));
  g_hash_table_insert (anthy->romaji, "su", g_strdup ("す\x1eス"));
  g_hash_table_insert (anthy->romaji, "t", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "ta", g_strdup ("た\x1eタ"));
  g_hash_table_insert (anthy->romaji, "te", g_strdup ("て\x1eテ"));
  g_hash_table_insert (anthy->romaji, "to", g_strdup ("と\x1eト"));
  g_hash_table_insert (anthy->romaji, "ts", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "tsu", g_strdup ("つ\x1eツ"));
  g_hash_table_insert (anthy->romaji, "u", g_strdup ("う\x1eウ"));
  g_hash_table_insert (anthy->romaji, "w", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "wa", g_strdup ("わ\x1eワ"));
  g_hash_table_insert (anthy->romaji, "we", g_strdup ("ゑ\x1eヱ"));
  g_hash_table_insert (anthy->romaji, "wi", g_strdup ("ゐ\x1eヰ"));
  g_hash_table_insert (anthy->romaji, "wo", g_strdup ("を\x1eヲ"));
  g_hash_table_insert (anthy->romaji, "y", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "ya", g_strdup ("や\x1eャ"));
  g_hash_table_insert (anthy->romaji, "ya", g_strdup ("や\x1eヤ"));
  g_hash_table_insert (anthy->romaji, "yo", g_strdup ("よ\x1eョ"));
  g_hash_table_insert (anthy->romaji, "yo", g_strdup ("よ\x1eヨ"));
  g_hash_table_insert (anthy->romaji, "yu", g_strdup ("ゆ\x1eュ"));
  g_hash_table_insert (anthy->romaji, "yu", g_strdup ("ゆ\x1eユ"));
  g_hash_table_insert (anthy->romaji, "z", g_strdup ("")); /* dummy */
  g_hash_table_insert (anthy->romaji, "za", g_strdup ("ざ\x1eザ"));
  g_hash_table_insert (anthy->romaji, "ze", g_strdup ("ぜ\x1eゼ"));
  g_hash_table_insert (anthy->romaji, "zo", g_strdup ("ぞ\x1eゾ"));
  g_hash_table_insert (anthy->romaji, "zu", g_strdup ("ず\x1eズ"));
  g_hash_table_insert (anthy->romaji, "zu", g_strdup ("づ\x1eヅ"));
  g_hash_table_insert (anthy->romaji, ",", g_strdup ("、\x1e、"));
  g_hash_table_insert (anthy->romaji, ".", g_strdup ("。\x1e。"));

  if (anthy_init () < 0)
    g_error (G_STRLOC ": %s: anthy is not initialized", G_STRFUNC);

  anthy->context = anthy_create_context ();
  g_atomic_int_inc (&anthy_ref_count);
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
  g_hash_table_unref (anthy->romaji);

  if (g_atomic_int_dec_and_test (&anthy_ref_count))
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
    *cursor_pos = g_utf8_strlen (*str, -1);
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
