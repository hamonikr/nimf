/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-m17n.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2019 Hodong Kim <cogniti@gmail.com>
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
#include <m17n.h>
#include <glib/gi18n.h>

#define NIMF_TYPE_M17N              (nimf_m17n_get_type ())
#define NIMF_M17N(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), NIMF_TYPE_M17N, NimfM17n))
#define NIMF_M17N_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_M17N, NimfM17nClass))
#define NIMF_IS_M17N(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), NIMF_TYPE_M17N))
#define NIMF_IS_M17N_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_M17N))
#define NIMF_M17N_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), NIMF_TYPE_M17N, NimfM17nClass))

typedef struct _NimfM17n      NimfM17n;
typedef struct _NimfM17nClass NimfM17nClass;

struct _NimfM17n
{
  NimfEngine parent_instance;

  NimfCandidatable  *candidatable;
  gchar             *id;
  GSettings         *settings;
  gchar             *method;
  MInputMethod      *im;
  MInputContext     *ic;
  MConverter        *converter;
  gchar             *preedit;
  NimfPreeditState   preedit_state;
  NimfPreeditAttr  **preedit_attrs;
  gint               current_page;
  gint               n_pages;
};

struct _NimfM17nClass
{
  /*< private >*/
  NimfEngineClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE (NimfM17n, nimf_m17n, NIMF_TYPE_ENGINE);

static NimfServiceIM *nimf_service_im_target = NULL;

static void
nimf_m17n_update_preedit (NimfEngine    *engine,
                          NimfServiceIM *target,
                          gchar         *new_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfM17n *m17n = NIMF_M17N (engine);

  /* preedit-start */
  if (m17n->preedit_state == NIMF_PREEDIT_STATE_END &&
      new_preedit[0] != 0)
  {
    m17n->preedit_state = NIMF_PREEDIT_STATE_START;
    nimf_engine_emit_preedit_start (engine, target);
  }
  /* preedit-changed */
  if (m17n->preedit[0] != 0 || new_preedit[0] != 0)
  {
    g_free (m17n->preedit);
    m17n->preedit = new_preedit;
    m17n->preedit_attrs[0]->end_index = g_utf8_strlen (m17n->preedit, -1);
    nimf_engine_emit_preedit_changed (engine, target, m17n->preedit,
                                      m17n->preedit_attrs,
                                      m17n->ic->cursor_pos);
  }
  else
  {
    g_free (new_preedit);
  }
  /* preedit-end */
  if (m17n->preedit_state == NIMF_PREEDIT_STATE_START &&
      m17n->preedit[0] == 0)
  {
    m17n->preedit_state = NIMF_PREEDIT_STATE_END;
    nimf_engine_emit_preedit_end (engine, target);
  }
}

void
nimf_m17n_reset (NimfEngine    *engine,
                 NimfServiceIM *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfM17n *m17n = NIMF_M17N (engine);

  g_return_if_fail (m17n->im != NULL);

  nimf_candidatable_hide (m17n->candidatable);
  minput_filter (m17n->ic, Mnil, NULL);
  nimf_m17n_update_preedit (engine, target, g_strdup (""));
  minput_reset_ic (m17n->ic);
}

void
nimf_m17n_focus_in (NimfEngine    *engine,
                    NimfServiceIM *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_M17N (engine)->im != NULL);
}

void
nimf_m17n_focus_out (NimfEngine    *engine,
                     NimfServiceIM *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_M17N (engine)->im != NULL);

  nimf_m17n_reset (engine, target);
}

static gchar *
nimf_m17n_mtext_to_utf8 (NimfM17n *m17n, MText *mt)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (mt != NULL, NULL);

  gint   buf_len = (mtext_len (mt) + 1) * 6;
  gchar *buf = g_malloc0 (buf_len);

  mconv_reset_converter (m17n->converter);
  mconv_rebind_buffer   (m17n->converter, (unsigned char *) buf, buf_len);
  mconv_encode (m17n->converter, mt);
  buf[m17n->converter->nbytes] = '\0';

  return buf;
}

static void
nimf_m17n_update_candidate (NimfEngine    *engine,
                            NimfServiceIM *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfM17n *m17n = NIMF_M17N (engine);

  nimf_candidatable_clear (m17n->candidatable, target);

  MPlist *page;
  m17n->current_page = m17n->ic->candidate_index / 10 + 1;
  m17n->n_pages = 0;

  for (page = m17n->ic->candidate_list;
       page && mplist_key (page) != Mnil;
       page = mplist_next (page))
  {
    MSymbol type = mplist_key (page);
    m17n->n_pages++;

    if (m17n->current_page != m17n->n_pages)
      continue;

    if (type == Mplist)
    {
      MPlist *items;

      for (items = mplist_value (page);
           items && mplist_key (items) != Mnil;
           items = mplist_next (items))
      {
        gchar *item;

        item = nimf_m17n_mtext_to_utf8 (m17n, mplist_value (items));
        nimf_candidatable_append (m17n->candidatable, item, NULL);

        g_free (item);
      }
    }
    else if (type == Mtext)
    {
      gchar *items;
      gint   i, len;

      items = nimf_m17n_mtext_to_utf8 (m17n, (MText *) mplist_value (page));
      len   = g_utf8_strlen (items, -1);

      for (i = 0; i < len; i++)
      {
        gchar  item[4];
        gchar *p = g_utf8_offset_to_pointer (items, i);
        g_utf8_strncpy (item, p, 1);
        nimf_candidatable_append (m17n->candidatable, item, NULL);
      }

      g_free (items);
    }
  }

  nimf_candidatable_select_item_by_index_in_page (m17n->candidatable,
                                                  m17n->ic->candidate_index % 10);
  nimf_candidatable_set_page_values (m17n->candidatable, target,
                                     m17n->current_page, m17n->n_pages, 10);
}

static void
nimf_m17n_page_up (NimfM17n *m17n)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (--m17n->current_page < 1)
    m17n->current_page = m17n->n_pages;
}

static void
nimf_m17n_page_down (NimfM17n *m17n)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (++m17n->current_page > m17n->n_pages)
    m17n->current_page = 1;
}

gboolean
nimf_m17n_filter_event (NimfEngine    *engine,
                        NimfServiceIM *target,
                        NimfEvent     *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfM17n *m17n = NIMF_M17N (engine);

  g_return_val_if_fail (m17n->im != NULL, FALSE);

  if (event->key.type   == NIMF_EVENT_KEY_RELEASE ||
      event->key.keyval == NIMF_KEY_Shift_L       ||
      event->key.keyval == NIMF_KEY_Shift_R)
    return FALSE;

  nimf_service_im_target = target;
  guint keyval = event->key.keyval;
  gboolean move = FALSE;

  if (nimf_candidatable_is_visible (m17n->candidatable))
  {
    switch (keyval)
    {
      case NIMF_KEY_Up:
      case NIMF_KEY_KP_Up:
        keyval = NIMF_KEY_Left;
        break;
      case NIMF_KEY_Down:
      case NIMF_KEY_KP_Down:
        keyval = NIMF_KEY_Right;
        break;
      case NIMF_KEY_Left:
      case NIMF_KEY_KP_Left:
      case NIMF_KEY_Page_Up:
      case NIMF_KEY_KP_Page_Up:
        keyval = NIMF_KEY_Up;
        nimf_m17n_page_up (m17n);
        break;
      case NIMF_KEY_Right:
      case NIMF_KEY_KP_Right:
      case NIMF_KEY_Page_Down:
      case NIMF_KEY_KP_Page_Down:
        keyval = NIMF_KEY_Down;
        nimf_m17n_page_down (m17n);
        break;
      case NIMF_KEY_KP_0:
        keyval = NIMF_KEY_0;
        break;
      case NIMF_KEY_KP_1:
        keyval = NIMF_KEY_1;
        break;
      case NIMF_KEY_KP_2:
        keyval = NIMF_KEY_2;
        break;
      case NIMF_KEY_KP_3:
        keyval = NIMF_KEY_3;
        break;
      case NIMF_KEY_KP_4:
        keyval = NIMF_KEY_4;
        break;
      case NIMF_KEY_KP_5:
        keyval = NIMF_KEY_5;
        break;
      case NIMF_KEY_KP_6:
        keyval = NIMF_KEY_6;
        break;
      case NIMF_KEY_KP_7:
        keyval = NIMF_KEY_7;
        break;
      case NIMF_KEY_KP_8:
        keyval = NIMF_KEY_8;
        break;
      case NIMF_KEY_KP_9:
        keyval = NIMF_KEY_9;
        break;
      default:
        move = TRUE;
        break;
    }
  }

  const gchar *keysym_name;
  gboolean retval;

  keysym_name = nimf_keyval_to_keysym_name (keyval);
  MSymbol symbol;

  if (keysym_name)
  {
    GString *string;
    string = g_string_new ("");

    if (event->key.state & NIMF_HYPER_MASK)
      g_string_append (string, "H-");

    if (event->key.state & NIMF_SUPER_MASK)
      g_string_append (string, "s-");

    if (event->key.state & NIMF_MOD5_MASK)
      g_string_append (string, "G-");

    if (event->key.state & NIMF_MOD1_MASK)
      g_string_append (string, "A-");

    if (event->key.state & NIMF_META_MASK)
      g_string_append (string, "M-");

    if (event->key.state & NIMF_CONTROL_MASK)
      g_string_append (string, "C-");

    if (event->key.state & NIMF_SHIFT_MASK)
      g_string_append (string, "S-");

    g_string_append (string, keysym_name);
    symbol = msymbol (string->str);
    g_string_free (string, TRUE);
  }
  else
  {
    g_warning (G_STRLOC ": %s: keysym name not found", G_STRFUNC);
    symbol = Mnil;
  }

  retval = minput_filter (m17n->ic, symbol, NULL);

  if (!retval)
  {
    MText *produced;
    produced = mtext ();
    retval = !minput_lookup (m17n->ic, symbol, NULL, produced);

    if (mtext_len (produced) > 0)
    {
      gchar *buf;
      buf = nimf_m17n_mtext_to_utf8 (m17n, produced);

      if (m17n->converter->nbytes > 0)
        nimf_engine_emit_commit (engine, target, (const gchar *) buf);

      g_free (buf);
    }

    m17n_object_unref (produced);
  }

  if (m17n->ic->preedit_changed)
  {
    gchar *new_preedit = nimf_m17n_mtext_to_utf8 (m17n, m17n->ic->preedit);
    nimf_m17n_update_preedit (engine, target, new_preedit);
  }

  if (m17n->ic->status_changed)
  {
    gchar *status;
    status = nimf_m17n_mtext_to_utf8 (m17n, m17n->ic->status);

    if (status && strlen (status))
      g_debug ("Minput_status_draw: %s", status);

    g_free (status);
  }

  if (m17n->ic->candidate_list && m17n->ic->candidate_show)
  {
    nimf_m17n_update_candidate (engine, target);

    if (!nimf_candidatable_is_visible (m17n->candidatable))
      nimf_candidatable_show (m17n->candidatable, target, FALSE);
    else if (move)
      nimf_candidatable_show (m17n->candidatable, target, FALSE);
  }
  else
  {
    nimf_candidatable_hide (m17n->candidatable);
  }

  nimf_service_im_target = NULL;

  return retval;
}

static void
on_get_surrounding_text (MInputContext *context,
                         MSymbol        command)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfM17n *m17n = context->arg;

  if (!NIMF_IS_M17N (m17n) || !nimf_service_im_target)
    return;

  gchar *text;
  gint   cursor_pos, nchars, nbytes;
  MText *mt, *surround = NULL;
  int    len, pos;

  nimf_engine_get_surrounding (NIMF_ENGINE (m17n),
                               nimf_service_im_target,
                               &text,
                               &cursor_pos);
  if (text == NULL)
    return;

  nchars = g_utf8_strlen (text, -1);
  nbytes = strlen (text);

  mt = mconv_decode_buffer (Mcoding_utf_8,
                            (const unsigned char *) text, nbytes);
  g_free (text);

  len = (long) mplist_value (m17n->ic->plist);

  if (len < 0)
  {
    pos = cursor_pos + len;

    if (pos < 0)
      pos = 0;

    surround = mtext_duplicate (mt, pos, cursor_pos);
  }
  else if (len > 0)
  {
    pos = cursor_pos + len;

    if (pos > nchars)
      pos = nchars;

    surround = mtext_duplicate (mt, cursor_pos, pos);
  }

  if (!surround)
    surround = mtext ();

  m17n_object_unref (mt);
  mplist_set (m17n->ic->plist, Mtext, surround);
  m17n_object_unref (surround);
}

static void
on_delete_surrounding_text (MInputContext *context,
                            MSymbol        command)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfM17n *m17n = context->arg;

  if (!NIMF_IS_M17N (m17n) || !nimf_service_im_target)
    return;

  int len = (long) mplist_value (m17n->ic->plist);

  if (len < 0)
    nimf_engine_emit_delete_surrounding (NIMF_ENGINE (m17n),
                                         nimf_service_im_target, len, -len);
  else
    nimf_engine_emit_delete_surrounding (NIMF_ENGINE (m17n),
                                         nimf_service_im_target, 0, len);
}

static void
nimf_m17n_open_im (NimfM17n *m17n)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **strv;

  m17n->preedit = g_strdup ("");
  m17n->preedit_attrs[0] = nimf_preedit_attr_new (NIMF_PREEDIT_ATTR_UNDERLINE, 0, 0);
  m17n->preedit_attrs[0]->type = NIMF_PREEDIT_ATTR_UNDERLINE;
  m17n->preedit_attrs[0]->start_index = 0;
  m17n->preedit_attrs[0]->end_index   = 0;
  m17n->preedit_attrs[1] = NULL;

  M17N_INIT();

  strv = g_strsplit (m17n->method, ":", 2);

  if (g_strv_length (strv) > 1)
  {
    m17n->im = minput_open_im (msymbol (strv[0]), msymbol (strv[1]), NULL);

    if (m17n->im)
    {
      mplist_put (m17n->im->driver.callback_list,
                  Minput_get_surrounding_text, on_get_surrounding_text);
      mplist_put (m17n->im->driver.callback_list,
                  Minput_delete_surrounding_text, on_delete_surrounding_text);
      m17n->ic = minput_create_ic (m17n->im, m17n);
      m17n->converter = mconv_buffer_converter (Mcoding_utf_8, NULL, 0);
    }
  }

  g_strfreev (strv);

  g_return_if_fail (m17n->im != NULL);
}

static void
nimf_m17n_close_im (NimfM17n *m17n)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (m17n->converter)
    mconv_free_converter (m17n->converter);

  if (m17n->ic)
    minput_destroy_ic    (m17n->ic);

  if (m17n->im)
    minput_close_im      (m17n->im);


  m17n->converter = NULL;
  m17n->ic        = NULL;
  m17n->im        = NULL;

  M17N_FINI ();

  g_free (m17n->preedit);
}

static void
on_changed_method (GSettings *settings,
                   gchar     *key,
                   NimfM17n  *m17n)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_free (m17n->method);
  m17n->method = g_settings_get_string (settings, key);

  if (m17n->ic)
    minput_reset_ic (m17n->ic);

  nimf_m17n_close_im (m17n);
  nimf_m17n_open_im  (m17n);
}

static void
nimf_m17n_init (NimfM17n *m17n)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  m17n->id       = g_strdup ("nimf-m17n");
  m17n->settings = g_settings_new ("org.nimf.engines.nimf-m17n");
  m17n->method   = g_settings_get_string (m17n->settings, "get-input-methods");
  m17n->preedit_attrs = g_malloc_n (2, sizeof (NimfPreeditAttr *));

  nimf_m17n_open_im (m17n);

  g_signal_connect (m17n->settings, "changed::get-input-methods",
                    G_CALLBACK (on_changed_method), m17n);
}

static void
nimf_m17n_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfM17n *m17n = NIMF_M17N (object);

  nimf_m17n_close_im (m17n);

  nimf_preedit_attr_freev (m17n->preedit_attrs);
  g_free (m17n->id);
  g_free (m17n->method);
  g_object_unref (m17n->settings);

  G_OBJECT_CLASS (nimf_m17n_parent_class)->finalize (object);
}

const gchar *
nimf_m17n_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_M17N (engine)->id;
}

const gchar *
nimf_m17n_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_M17N (engine)->id;
}

static void
nimf_m17n_constructed (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfM17n *m17n = NIMF_M17N (object);

  m17n->candidatable = nimf_engine_get_candidatable (NIMF_ENGINE (m17n));
}

static void
nimf_m17n_class_init (NimfM17nClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass    *object_class = G_OBJECT_CLASS (class);
  NimfEngineClass *engine_class = NIMF_ENGINE_CLASS (class);

  engine_class->filter_event  = nimf_m17n_filter_event;
  engine_class->reset         = nimf_m17n_reset;
  engine_class->focus_in      = nimf_m17n_focus_in;
  engine_class->focus_out     = nimf_m17n_focus_out;

  engine_class->get_id        = nimf_m17n_get_id;
  engine_class->get_icon_name = nimf_m17n_get_icon_name;

  object_class->constructed = nimf_m17n_constructed;
  object_class->finalize    = nimf_m17n_finalize;
}

static void
nimf_m17n_class_finalize (NimfM17nClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

typedef struct {
  const gchar *code;
  const gchar *name;
} Country;


static const Country country[] = {
  {"am",  N_("Amharic")},
  {"ar",  N_("Arabic")},
  {"as",  N_("Assamese")},
  {"ath", N_("Athabaskan languages")},
  {"be",  N_("Belarusian")},
  {"bla", N_("Siksika")},
  {"bn",  N_("Bengali")},
  {"bo",  N_("Tibetan")},
  {"cmc", N_("Chamic Languages")},
  {"cr",  N_("Cree")},
  {"cs",  N_("Czech")},
  {"da",  N_("Danish")},
  {"dv",  N_("Divehi")},
  {"el",  N_("Greek")},
  {"en",  N_("English")},
  {"eo",  N_("Esperanto")},
  {"fa",  N_("Persian")},
  {"fr",  N_("French")},
  {"grc", N_("Ancient Greek")},
  {"gu",  N_("Gujarati")},
  {"he",  N_("Hebrew")},
  {"hi",  N_("Hindi")},
  {"hr",  N_("Croatian")},
  {"hu",  N_("Hungarian")},
  {"hy",  N_("Armenian")},
  {"ii",  N_("Sichuan Yi")},
  {"iu",  N_("Inuktitut")},
  {"ja",  N_("Japanese")},
  {"ka",  N_("Georgian")},
  {"kk",  N_("Kazakh")},
  {"km",  N_("Khmer")},
  {"kn",  N_("Kannada")},
  {"ko",  N_("Korean")},
  {"ks",  N_("Kashmiri")},
  {"lo",  N_("Lao")},
  {"mai", N_("Maithili")},
  {"ml",  N_("Malayalam")},
  {"mr",  N_("Marathi")},
  {"my",  N_("Burmese")},
  {"ne",  N_("Nepali")},
  {"nsk", N_("Naskapi language")},
  {"oj",  N_("Ojibwa")},
  {"or",  N_("Oriya")},
  {"pa",  N_("Punjabi")},
  {"ps",  N_("Pushto")},
  {"ru",  N_("Russian")},
  {"sa",  N_("Sanskrit")},
  {"sd",  N_("Sindhi")},
  {"si",  N_("Sinhalese")},
  {"sk",  N_("Slovak")},
  {"sr",  N_("Serbian")},
  {"sv",  N_("Swedish")},
  {"tai", N_("Tai Viet")},
  {"ta",  N_("Tamil")},
  {"te",  N_("Telugu")},
  {"th",  N_("Thai")},
  {"t",   N_("Etc")},
  {"ug",  N_("Uighur")},
  {"uk",  N_("Ukrainian")},
  {"ur",  N_("Urdu")},
  {"uz",  N_("Uzbek")},
  {"vi",  N_("Vietnamese")},
  {"yi",  N_("Yiddish")},
  {"zh",  N_("Chinese")}
};

gchar **
nimf_m17n_get_input_methods ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **methods;
  GHashTable *table;
  MPlist *imlist, *pl;
  gint i;
  M17N_INIT();
  table = g_hash_table_new (g_str_hash, g_str_equal);

  for (i = 0; i < G_N_ELEMENTS (country); i++)
    g_hash_table_insert (table, (gpointer) country[i].code,
                                (gpointer) gettext (country[i].name));

  methods = g_malloc0_n (1, sizeof (gchar *));
  imlist = minput_list (Mnil);

  i = 0;
  for (pl = imlist; mplist_key (pl) != Mnil; pl = mplist_next (pl))
  {
    MPlist *p = mplist_value (pl);
    MSymbol lang, name, sane;

    lang = mplist_value (p);
    p = mplist_next (p);
    name = mplist_value (p);
    p = mplist_next (p);
    sane = mplist_value (p);

    if (sane == Mt)
    {
      const gchar *code = msymbol_name (lang);
      const gchar *language = g_hash_table_lookup (table, code);

      if (!language)
        language = code;

      methods[i] = g_strjoin (":", language, code, msymbol_name (name), NULL);
      methods = g_realloc_n (methods, sizeof (gchar *), i + 2);
      methods[i + 1] = NULL;
      i++;
    }
  }

  m17n_object_unref (imlist);
  g_hash_table_destroy (table);
  M17N_FINI();

  return methods;
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_m17n_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_m17n_get_type ();
}
