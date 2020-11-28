/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-libhangul.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2020 Hodong Kim <cogniti@gmail.com>
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
#include <hangul.h>
#include <glib/gi18n.h>

#define NIMF_TYPE_LIBHANGUL             (nimf_libhangul_get_type ())
#define NIMF_LIBHANGUL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_LIBHANGUL, NimfLibhangul))
#define NIMF_LIBHANGUL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_LIBHANGUL, NimfLibhangulClass))
#define NIMF_IS_LIBHANGUL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_LIBHANGUL))
#define NIMF_IS_LIBHANGUL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_LIBHANGUL))
#define NIMF_LIBHANGUL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_LIBHANGUL, NimfLibhangulClass))

typedef struct _NimfLibhangul      NimfLibhangul;
typedef struct _NimfLibhangulClass NimfLibhangulClass;

struct _NimfLibhangul
{
  NimfEngine parent_instance;

  NimfCandidatable   *candidatable;
  HangulInputContext *context;
  gchar              *preedit_string;
  NimfPreeditAttr   **preedit_attrs;
  NimfPreeditState    preedit_state;
  gchar              *id;

  NimfKey           **hanja_keys;
  GSettings          *settings;
  gboolean            is_double_consonant_rule;
  gboolean            auto_reordering;
  gchar              *method;
  /* workaround: ignore reset called by commit callback in application */
  gboolean            ignore_reset_in_commit_cb;
  gboolean            is_committing;

  HanjaList          *hanja_list;
  gint                current_page;
  gint                n_pages;
};

struct _NimfLibhangulClass
{
  /*< private >*/
  NimfEngineClass parent_class;
};

typedef struct {
  const gchar *id;
  const gchar *name;
} Keyboard;

static const Keyboard keyboards[] = {
  {"2",   N_("Dubeolsik")},
  {"2y",  N_("Dubeolsik Yetgeul")},
  {"32",  N_("Sebeolsik Dubeol Layout")},
  {"39",  N_("Sebeolsik 390")},
  {"3f",  N_("Sebeolsik Final")},
  {"3s",  N_("Sebeolsik Noshift")},
  {"3y",  N_("Sebeolsik Yetgeul")},
  {"ro",  N_("Romaja")},
  {"ahn", N_("Ahnmatae")}
};

static HanjaTable *nimf_libhangul_hanja_table  = NULL;
static HanjaTable *nimf_libhangul_symbol_table = NULL;
static gint        nimf_libhangul_hanja_table_ref_count = 0;

G_DEFINE_DYNAMIC_TYPE (NimfLibhangul, nimf_libhangul, NIMF_TYPE_ENGINE);

static void
nimf_libhangul_update_preedit (NimfEngine    *engine,
                               NimfServiceIC *target,
                               gchar         *new_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  /* preedit-start */
  if (hangul->preedit_state == NIMF_PREEDIT_STATE_END && new_preedit[0] != 0)
  {
    hangul->preedit_state = NIMF_PREEDIT_STATE_START;
    nimf_engine_emit_preedit_start (engine, target);
  }
  /* preedit-changed */
  if (hangul->preedit_string[0] != 0 || new_preedit[0] != 0)
  {
    g_free (hangul->preedit_string);
    hangul->preedit_string = new_preedit;
    hangul->preedit_attrs[0]->end_index = g_utf8_strlen (hangul->preedit_string, -1);
    nimf_engine_emit_preedit_changed (engine, target, hangul->preedit_string,
                                      hangul->preedit_attrs,
                                      g_utf8_strlen (hangul->preedit_string,
                                                     -1));
  }
  else
    g_free (new_preedit);
  /* preedit-end */
  if (hangul->preedit_state == NIMF_PREEDIT_STATE_START &&
      hangul->preedit_string[0] == 0)
  {
    hangul->preedit_state = NIMF_PREEDIT_STATE_END;
    nimf_engine_emit_preedit_end (engine, target);
  }
}

void
nimf_libhangul_emit_commit (NimfEngine    *engine,
                            NimfServiceIC *target,
                            const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);
  hangul->is_committing = TRUE;
  nimf_engine_emit_commit (engine, target, text);
  hangul->is_committing = FALSE;
}

void
nimf_libhangul_reset (NimfEngine    *engine,
                      NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  /* workaround: ignore reset called by commit callback in application */
  if (G_UNLIKELY (hangul->ignore_reset_in_commit_cb && hangul->is_committing))
    return;

  nimf_candidatable_hide (hangul->candidatable);

  const ucschar *flush;
  flush = hangul_ic_flush (hangul->context);

  if (flush[0] != 0)
  {
    gchar *text = g_ucs4_to_utf8 (flush, -1, NULL, NULL, NULL);
    nimf_libhangul_emit_commit (engine, target, text);
    g_free (text);
  }

  nimf_libhangul_update_preedit (engine, target, g_strdup (""));
}

void
nimf_libhangul_focus_in (NimfEngine    *engine,
                         NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));
}

void
nimf_libhangul_focus_out (NimfEngine    *engine,
                          NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  nimf_libhangul_reset (engine, target);
}

static void
on_candidate_clicked (NimfEngine    *engine,
                      NimfServiceIC *target,
                      gchar         *text,
                      gint           index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  if (text)
  {
    /* commit text inside hangul_ic disappears */
    hangul_ic_reset (hangul->context);

    if (hangul->preedit_string[0] == 0)
      nimf_engine_emit_delete_surrounding (engine, target, -1, 1);

    nimf_libhangul_emit_commit (engine, target, text);

    if (hangul->preedit_string[0] != 0)
      nimf_libhangul_update_preedit (engine, target, g_strdup (""));
  }

  nimf_candidatable_hide (hangul->candidatable);
}

static gint
nimf_libhangul_get_current_page (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_LIBHANGUL (engine)->current_page;
}

static void
nimf_libhangul_update_page (NimfEngine    *engine,
                            NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  if (hangul->hanja_list == NULL)
    return;

  gint i;
  gint list_len = hanja_list_get_size (hangul->hanja_list);
  nimf_candidatable_clear (hangul->candidatable, target);

  for (i = (hangul->current_page - 1) * 10;
       i < MIN (hangul->current_page * 10, list_len); i++)
  {
    const Hanja *hanja = hanja_list_get_nth (hangul->hanja_list, i);
    const char  *item1 = hanja_get_value    (hanja);
    const char  *item2 = hanja_get_comment  (hanja);
    nimf_candidatable_append (hangul->candidatable, item1, item2);
  }

  nimf_candidatable_set_page_values (hangul->candidatable, target,
                                     hangul->current_page, hangul->n_pages, 10);
}

static gboolean
nimf_libhangul_page_up (NimfEngine *engine, NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  if (hangul->hanja_list == NULL)
    return FALSE;

  if (hangul->current_page <= 1)
  {
    nimf_candidatable_select_first_item_in_page (hangul->candidatable);
    return FALSE;
  }

  hangul->current_page--;
  nimf_libhangul_update_page (engine, target);
  nimf_candidatable_select_last_item_in_page (hangul->candidatable);

  return TRUE;
}

static gboolean
nimf_libhangul_page_down (NimfEngine *engine, NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  if (hangul->hanja_list == NULL)
    return FALSE;

  if (hangul->current_page == hangul->n_pages)
  {
    nimf_candidatable_select_last_item_in_page (hangul->candidatable);
    return FALSE;
  }

  hangul->current_page++;
  nimf_libhangul_update_page (engine, target);
  nimf_candidatable_select_first_item_in_page (hangul->candidatable);

  return TRUE;
}

static void
nimf_libhangul_page_home (NimfEngine *engine, NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  if (hangul->hanja_list == NULL)
    return;

  if (hangul->current_page <= 1)
  {
    nimf_candidatable_select_first_item_in_page (hangul->candidatable);
    return;
  }

  hangul->current_page = 1;
  nimf_libhangul_update_page (engine, target);
  nimf_candidatable_select_first_item_in_page (hangul->candidatable);
}

static void
nimf_libhangul_page_end (NimfEngine *engine, NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  if (hangul->hanja_list == NULL)
    return;

  if (hangul->current_page == hangul->n_pages)
  {
    nimf_candidatable_select_last_item_in_page (hangul->candidatable);
    return;
  }

  hangul->current_page = hangul->n_pages;
  nimf_libhangul_update_page (engine, target);
  nimf_candidatable_select_last_item_in_page (hangul->candidatable);
}

static void
on_candidate_scrolled (NimfEngine    *engine,
                       NimfServiceIC *target,
                       gdouble        value)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  if ((gint) value == nimf_libhangul_get_current_page (engine))
    return;

  while (hangul->n_pages > 1)
  {
    gint d = (gint) value - nimf_libhangul_get_current_page (engine);

    if (d > 0)
      nimf_libhangul_page_down (engine, target);
    else if (d < 0)
      nimf_libhangul_page_up (engine, target);
    else if (d == 0)
      break;
  }
}

static gboolean
nimf_libhangul_filter_leading_consonant (NimfEngine    *engine,
                                         NimfServiceIC *target,
                                         guint          keyval)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  const ucschar *ucs_preedit;
  ucs_preedit = hangul_ic_get_preedit_string (hangul->context);

  /* check ㄱ ㄷ ㅂ ㅅ ㅈ */
  if ((keyval == 'r' && ucs_preedit[0] == 0x3131 && ucs_preedit[1] == 0) ||
      (keyval == 'e' && ucs_preedit[0] == 0x3137 && ucs_preedit[1] == 0) ||
      (keyval == 'q' && ucs_preedit[0] == 0x3142 && ucs_preedit[1] == 0) ||
      (keyval == 't' && ucs_preedit[0] == 0x3145 && ucs_preedit[1] == 0) ||
      (keyval == 'w' && ucs_preedit[0] == 0x3148 && ucs_preedit[1] == 0))
  {
    gchar *preedit = g_ucs4_to_utf8 (ucs_preedit, -1, NULL, NULL, NULL);
    nimf_libhangul_emit_commit (engine, target, preedit);
    g_free (preedit);
    nimf_engine_emit_preedit_changed (engine, target, hangul->preedit_string,
                                      hangul->preedit_attrs,
                                      g_utf8_strlen (hangul->preedit_string,
                                                     -1));
    return TRUE;
  }

  return FALSE;
}

gboolean
nimf_libhangul_filter_event (NimfEngine    *engine,
                             NimfServiceIC *target,
                             NimfEvent     *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  guint    keyval;
  gboolean retval = FALSE;

  NimfLibhangul *hangul = NIMF_LIBHANGUL (engine);

  if (event->key.type   == NIMF_EVENT_KEY_RELEASE ||
      event->key.keyval == NIMF_KEY_Shift_L       ||
      event->key.keyval == NIMF_KEY_Shift_R)
    return FALSE;

  if (event->key.state & (NIMF_CONTROL_MASK | NIMF_MOD1_MASK))
  {
    nimf_libhangul_reset (engine, target);
    return FALSE;
  }

  if (G_UNLIKELY (nimf_event_matches (event,
                  (const NimfKey **) hangul->hanja_keys)))
  {
    if (nimf_candidatable_is_visible (hangul->candidatable) == FALSE)
    {
      gchar item[4];
      const char *key = hangul->preedit_string;
      gboolean use_preedit;

      if (hangul->preedit_string[0] == 0)
      {
        gchar *text;
        gint   cursor_pos;

        nimf_engine_get_surrounding (engine, target, &text, &cursor_pos);

        if (text && cursor_pos > 0)
        {
          gchar *p = g_utf8_offset_to_pointer (text, cursor_pos - 1);
          g_utf8_strncpy (item, p, 1);

          if (g_utf8_validate (item, -1, NULL))
            key = item;
        }

        g_free (text);
      }

      hanja_list_delete (hangul->hanja_list);
      nimf_candidatable_clear (hangul->candidatable, target);
      hangul->hanja_list = hanja_table_match_exact (nimf_libhangul_hanja_table, key);

      if (hangul->hanja_list == NULL)
        hangul->hanja_list = hanja_table_match_exact (nimf_libhangul_symbol_table, key);

      hangul->n_pages = (hanja_list_get_size (hangul->hanja_list) + 9) / 10;
      hangul->current_page = 1;
      nimf_libhangul_update_page (engine, target);
      use_preedit = nimf_service_ic_get_use_preedit (target);

      if (!use_preedit)
        nimf_candidatable_set_auxiliary_text (hangul->candidatable,
                                              key, g_utf8_strlen (key, -1));

      nimf_candidatable_show (hangul->candidatable, target, !use_preedit);
      nimf_candidatable_select_first_item_in_page (hangul->candidatable);
    }
    else
    {
      nimf_candidatable_hide (hangul->candidatable);
      nimf_candidatable_clear (hangul->candidatable, target);
      hanja_list_delete (hangul->hanja_list);
      hangul->hanja_list = NULL;
      hangul->current_page = 0;
      hangul->n_pages = 0;
    }

    return TRUE;
  }

  if (nimf_candidatable_is_visible (hangul->candidatable))
  {
    switch (event->key.keyval)
    {
      case NIMF_KEY_Return:
      case NIMF_KEY_KP_Enter:
        {
          gchar *text;

          text = nimf_candidatable_get_selected_text (hangul->candidatable);
          on_candidate_clicked (engine, target, text, -1);

          g_free (text);
        }
        break;
      case NIMF_KEY_Up:
      case NIMF_KEY_KP_Up:
        nimf_candidatable_select_previous_item (hangul->candidatable);
        break;
      case NIMF_KEY_Down:
      case NIMF_KEY_KP_Down:
        nimf_candidatable_select_next_item (hangul->candidatable);
        break;
      case NIMF_KEY_Page_Up:
      case NIMF_KEY_KP_Page_Up:
        nimf_libhangul_page_up (engine, target);
        break;
      case NIMF_KEY_Page_Down:
      case NIMF_KEY_KP_Page_Down:
        nimf_libhangul_page_down (engine, target);
        break;
      case NIMF_KEY_Home:
        nimf_libhangul_page_home (engine, target);
        break;
      case NIMF_KEY_End:
        nimf_libhangul_page_end (engine, target);
        break;
      case NIMF_KEY_Escape:
        nimf_candidatable_hide (hangul->candidatable);
        break;
      case NIMF_KEY_0:
      case NIMF_KEY_1:
      case NIMF_KEY_2:
      case NIMF_KEY_3:
      case NIMF_KEY_4:
      case NIMF_KEY_5:
      case NIMF_KEY_6:
      case NIMF_KEY_7:
      case NIMF_KEY_8:
      case NIMF_KEY_9:
      case NIMF_KEY_KP_0:
      case NIMF_KEY_KP_1:
      case NIMF_KEY_KP_2:
      case NIMF_KEY_KP_3:
      case NIMF_KEY_KP_4:
      case NIMF_KEY_KP_5:
      case NIMF_KEY_KP_6:
      case NIMF_KEY_KP_7:
      case NIMF_KEY_KP_8:
      case NIMF_KEY_KP_9:
        {
          if (hangul->hanja_list == NULL || hangul->current_page < 1)
            break;

          gint i, n;
          gint list_len = hanja_list_get_size (hangul->hanja_list);

          if (event->key.keyval >= NIMF_KEY_0 &&
              event->key.keyval <= NIMF_KEY_9)
            n = (event->key.keyval - NIMF_KEY_0 + 9) % 10;
          else if (event->key.keyval >= NIMF_KEY_KP_0 &&
                   event->key.keyval <= NIMF_KEY_KP_9)
            n = (event->key.keyval - NIMF_KEY_KP_0 + 9) % 10;
          else
            break;

          i = (hangul->current_page - 1) * 10 + n;

          if (i < MIN (hangul->current_page * 10, list_len))
          {
            const Hanja *hanja = hanja_list_get_nth (hangul->hanja_list, i);
            const char  *text = hanja_get_value (hanja);
            on_candidate_clicked (engine, target, (gchar *) text, -1);
          }
        }
        break;
      default:
        break;
    }

    return TRUE;
  }

  const ucschar *ucs_commit;
  const ucschar *ucs_preedit;

  if (G_UNLIKELY (event->key.keyval == NIMF_KEY_BackSpace))
  {
    retval = hangul_ic_backspace (hangul->context);

    if (retval)
    {
      ucs_preedit = hangul_ic_get_preedit_string (hangul->context);
      gchar *new_preedit = g_ucs4_to_utf8 (ucs_preedit, -1, NULL, NULL, NULL);
      nimf_libhangul_update_preedit (engine, target, new_preedit);
    }

    return retval;
  }

  if (G_UNLIKELY (g_strcmp0 (hangul->method, "ro") == 0))
    keyval = event->key.keyval;
  else
    keyval = nimf_event_keycode_to_qwerty_keyval (event);

  if (!hangul->is_double_consonant_rule &&
      (g_strcmp0 (hangul->method, "2") == 0) &&
      nimf_libhangul_filter_leading_consonant (engine, target, keyval))
    return TRUE;

  retval = hangul_ic_process (hangul->context, keyval);

  ucs_commit  = hangul_ic_get_commit_string  (hangul->context);
  ucs_preedit = hangul_ic_get_preedit_string (hangul->context);

  gchar *new_commit  = g_ucs4_to_utf8 (ucs_commit,  -1, NULL, NULL, NULL);

  if (ucs_commit[0] != 0)
    nimf_libhangul_emit_commit (engine, target, new_commit);

  g_free (new_commit);

  gchar *new_preedit = g_ucs4_to_utf8 (ucs_preedit, -1, NULL, NULL, NULL);
  nimf_libhangul_update_preedit (engine, target, new_preedit);

  if (!retval)
  {
    switch (keyval)
    {
      case '_':
      case '-':
      case '+':
      case '=':
      case '{':
      case '[':
      case '}':
      case ']':
      case ':':
      case ';':
      case '\"':
      case '\'':
      case '<':
      case ',':
      case '>':
      case '.':
      case '?':
      case '/':
        nimf_libhangul_emit_commit (engine, target, (char *) &keyval);
        retval = TRUE;
      default:
        break;
    }
  }

  return retval;
}

static void
on_changed_method (GSettings     *settings,
                   gchar         *key,
                   NimfLibhangul *hangul)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_free (hangul->method);
  hangul->method = g_settings_get_string (settings, key);
  hangul_ic_select_keyboard (hangul->context, hangul->method);
  hangul_ic_set_option (hangul->context, HANGUL_IC_OPTION_AUTO_REORDER,
                        hangul->auto_reordering);
}

static void
on_changed_auto_reordering (GSettings     *settings,
                            gchar         *key,
                            NimfLibhangul *hangul)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  hangul->auto_reordering = g_settings_get_boolean (settings, key);
  hangul_ic_set_option (hangul->context, HANGUL_IC_OPTION_AUTO_REORDER,
                        hangul->auto_reordering);
}

static void
on_changed_keys (GSettings     *settings,
                 gchar         *key,
                 NimfLibhangul *hangul)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **keys = g_settings_get_strv (settings, key);

  if (g_strcmp0 (key, "hanja-keys") == 0)
  {
    nimf_key_freev (hangul->hanja_keys);
    hangul->hanja_keys = nimf_key_newv ((const gchar **) keys);
  }

  g_strfreev (keys);
}

static void
on_changed_double_consonant_rule (GSettings     *settings,
                                  gchar         *key,
                                  NimfLibhangul *hangul)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  hangul->is_double_consonant_rule = g_settings_get_boolean (settings, key);
}

static void
on_changed_ignore_reset_in_commit_cb (GSettings     *settings,
                                      gchar         *key,
                                      NimfLibhangul *hangul)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  hangul->ignore_reset_in_commit_cb = g_settings_get_boolean (settings, key);
}

static void
nimf_libhangul_init (NimfLibhangul *hangul)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **hanja_keys;

  hangul->settings = g_settings_new ("org.nimf.engines.nimf-libhangul");
  hangul->method = g_settings_get_string (hangul->settings, "get-method-infos");
  hangul->is_double_consonant_rule =
    g_settings_get_boolean (hangul->settings, "double-consonant-rule");
  hangul->auto_reordering =
    g_settings_get_boolean (hangul->settings, "auto-reordering");
  hangul->ignore_reset_in_commit_cb =
    g_settings_get_boolean (hangul->settings, "ignore-reset-in-commit-cb");

  hanja_keys = g_settings_get_strv (hangul->settings, "hanja-keys");
  hangul->hanja_keys = nimf_key_newv ((const gchar **) hanja_keys);
  hangul->context = hangul_ic_new (hangul->method);

  hangul->id = g_strdup ("nimf-libhangul");
  hangul->preedit_string = g_strdup ("");
  hangul->preedit_attrs  = g_malloc0_n (2, sizeof (NimfPreeditAttr *));
  hangul->preedit_attrs[0] = nimf_preedit_attr_new (NIMF_PREEDIT_ATTR_UNDERLINE, 0, 0);
  hangul->preedit_attrs[1] = NULL;

  if (nimf_libhangul_hanja_table_ref_count == 0)
  {
    nimf_libhangul_hanja_table  = hanja_table_load (NULL);
    nimf_libhangul_symbol_table = hanja_table_load (MSSYMBOL_PATH);
  }

  nimf_libhangul_hanja_table_ref_count++;

  g_strfreev (hanja_keys);

  hangul_ic_set_option (hangul->context, HANGUL_IC_OPTION_AUTO_REORDER,
                        hangul->auto_reordering);

  g_signal_connect_data (hangul->settings, "changed::get-method-infos",
    G_CALLBACK (on_changed_method), hangul, NULL, G_CONNECT_AFTER);
  g_signal_connect_data (hangul->settings, "changed::hanja-keys",
    G_CALLBACK (on_changed_keys), hangul, NULL, G_CONNECT_AFTER);
  g_signal_connect_data (hangul->settings, "changed::double-consonant-rule",
    G_CALLBACK (on_changed_double_consonant_rule), hangul, NULL, G_CONNECT_AFTER);
  g_signal_connect_data (hangul->settings, "changed::auto-reordering",
    G_CALLBACK (on_changed_auto_reordering), hangul, NULL, G_CONNECT_AFTER);
  g_signal_connect_data (hangul->settings, "changed::ignore-reset-in-commit-cb",
    G_CALLBACK (on_changed_ignore_reset_in_commit_cb), hangul, NULL, G_CONNECT_AFTER);
}

static void
nimf_libhangul_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (object);

  if (--nimf_libhangul_hanja_table_ref_count == 0)
  {
    hanja_table_delete (nimf_libhangul_hanja_table);
    hanja_table_delete (nimf_libhangul_symbol_table);
  }

  hanja_list_delete (hangul->hanja_list);
  hangul_ic_delete (hangul->context);
  g_free (hangul->preedit_string);
  nimf_preedit_attr_freev (hangul->preedit_attrs);
  g_free (hangul->id);
  g_free (hangul->method);
  nimf_key_freev (hangul->hanja_keys);
  g_object_unref (hangul->settings);

  G_OBJECT_CLASS (nimf_libhangul_parent_class)->finalize (object);
}

const gchar *
nimf_libhangul_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_LIBHANGUL (engine)->id;
}

const gchar *
nimf_libhangul_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_LIBHANGUL (engine)->id;
}

void
nimf_libhangul_set_method (NimfEngine *engine, const gchar *method_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_settings_set_string (NIMF_LIBHANGUL (engine)->settings,
                         "get-method-infos", method_id);
}

static void
nimf_libhangul_constructed (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfLibhangul *hangul = NIMF_LIBHANGUL (object);

  hangul->candidatable = nimf_engine_get_candidatable (NIMF_ENGINE (hangul));
}

static void
nimf_libhangul_class_init (NimfLibhangulClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  NimfEngineClass *engine_class = NIMF_ENGINE_CLASS (class);

  engine_class->filter_event       = nimf_libhangul_filter_event;
  engine_class->reset              = nimf_libhangul_reset;
  engine_class->focus_in           = nimf_libhangul_focus_in;
  engine_class->focus_out          = nimf_libhangul_focus_out;

  engine_class->candidate_page_up   = nimf_libhangul_page_up;
  engine_class->candidate_page_down = nimf_libhangul_page_down;
  engine_class->candidate_clicked   = on_candidate_clicked;
  engine_class->candidate_scrolled  = on_candidate_scrolled;

  engine_class->get_id             = nimf_libhangul_get_id;
  engine_class->get_icon_name      = nimf_libhangul_get_icon_name;
  engine_class->set_method         = nimf_libhangul_set_method;

  object_class->constructed = nimf_libhangul_constructed;
  object_class->finalize    = nimf_libhangul_finalize;
}

static void
nimf_libhangul_class_finalize (NimfLibhangulClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

NimfMethodInfo **
nimf_libhangul_get_method_infos ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMethodInfo **infos;
  gint             n_methods = G_N_ELEMENTS (keyboards);
  gint             i;

  infos = g_malloc (sizeof (NimfMethodInfo *) * n_methods + 1);

  for (i = 0; i < n_methods; i++)
  {
    infos[i] = nimf_method_info_new ();
    infos[i]->method_id = g_strdup (keyboards[i].id);
    infos[i]->label     = g_strdup (gettext (keyboards[i].name));
    infos[i]->group     = NULL;
  }

  infos[n_methods] = NULL;

  return infos;
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_libhangul_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_libhangul_get_type ();
}
