/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-chewing.c
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
#include <chewing.h>
#include <glib/gi18n.h>

#define NIMF_TYPE_CHEWING             (nimf_chewing_get_type ())
#define NIMF_CHEWING(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_CHEWING, NimfChewing))
#define NIMF_CHEWING_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_CHEWING, NimfChewingClass))
#define NIMF_IS_CHEWING(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_CHEWING))
#define NIMF_IS_CHEWING_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_CHEWING))
#define NIMF_CHEWING_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_CHEWING, NimfChewingClass))

typedef struct _NimfChewing      NimfChewing;
typedef struct _NimfChewingClass NimfChewingClass;

struct _NimfChewing
{
  NimfEngine parent_instance;

  NimfCandidate    *candidate;
  gchar            *id;
  GString          *preedit;
  NimfPreeditAttr **preedit_attrs;
  NimfPreeditState  preedit_state;
  ChewingContext   *context;
};

struct _NimfChewingClass
{
  /*< private >*/
  NimfEngineClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE (NimfChewing, nimf_chewing, NIMF_TYPE_ENGINE);

void
nimf_chewing_reset (NimfEngine  *engine,
                    NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfChewing *chewing = NIMF_CHEWING (engine);

  nimf_candidate_hide_window (chewing->candidate);

  if (chewing->preedit->len > 0)
  {
    nimf_engine_emit_commit (engine, target, chewing->preedit->str);
    g_string_assign (chewing->preedit, "");
    chewing->preedit_attrs[0]->start_index = 0;
    chewing->preedit_attrs[0]->end_index   = 0;
    nimf_engine_emit_preedit_changed (engine, target, "",
                                      chewing->preedit_attrs, 0);
    nimf_engine_emit_preedit_end (engine, target);
  }

  chewing_Reset (NIMF_CHEWING (engine)->context);
}

void
nimf_chewing_focus_in (NimfEngine  *engine,
                       NimfContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
nimf_chewing_focus_out (NimfEngine  *engine,
                        NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_candidate_hide_window (NIMF_CHEWING (engine)->candidate);
  nimf_chewing_reset (engine, target);
}

static void nimf_chewing_update (NimfEngine  *engine,
                                 NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfChewing *chewing = NIMF_CHEWING (engine);

  if (chewing_commit_Check (chewing->context))
    nimf_engine_emit_commit (engine, target,
                             chewing_commit_String_static (chewing->context));

  g_string_assign (chewing->preedit, "");

  const gchar *zuin_str = NULL;
  gint pos = 0;
  gint zuin_str_utf8_len = 0;

  if (chewing_buffer_Check (chewing->context))
    g_string_append (chewing->preedit,
                     chewing_buffer_String_static (chewing->context));

  pos = chewing_cursor_Current (chewing->context);
  zuin_str = chewing_bopomofo_String_static (chewing->context);

  if (zuin_str)
  {
    const gchar *tmp;
    tmp = g_utf8_offset_to_pointer (chewing->preedit->str, pos);
    g_string_insert (chewing->preedit, tmp - chewing->preedit->str, zuin_str);
    zuin_str_utf8_len = g_utf8_strlen (zuin_str, -1);
  }

  if (chewing->preedit_state == NIMF_PREEDIT_STATE_END &&
      chewing->preedit->len > 0)
  {
    chewing->preedit_state = NIMF_PREEDIT_STATE_START;
    nimf_engine_emit_preedit_start (engine, target);
  }

  chewing->preedit_attrs[0]->start_index = 0;
  chewing->preedit_attrs[0]->end_index   = g_utf8_strlen (chewing->preedit->str, -1);
  chewing->preedit_attrs[1]->start_index = pos;
  chewing->preedit_attrs[1]->end_index   = chewing->preedit_attrs[1]->start_index + zuin_str_utf8_len;
  nimf_engine_emit_preedit_changed (engine, target, chewing->preedit->str,
                                    chewing->preedit_attrs, pos);

  if (chewing->preedit_state == NIMF_PREEDIT_STATE_START &&
      chewing->preedit->len == 0)
  {
    chewing->preedit_state = NIMF_PREEDIT_STATE_END;
    nimf_engine_emit_preedit_end (engine, target);
  }

  nimf_candidate_clear (chewing->candidate, target);
  chewing_cand_Enumerate (chewing->context);

  gint i;
  for (i = 0; chewing_cand_hasNext (chewing->context) &&
              i < chewing_cand_ChoicePerPage (chewing->context); i++)
  {
    nimf_candidate_append (chewing->candidate,
                           chewing_cand_String_static (chewing->context), NULL);
  }

  nimf_candidate_set_page_values (chewing->candidate, target,
                                  chewing_cand_CurrentPage (chewing->context) + 1,
                                  chewing_cand_TotalPage   (chewing->context), 10);

  if (chewing_cand_TotalChoice (chewing->context) > 0)
    nimf_candidate_show_window (chewing->candidate, target, FALSE);
  else
    nimf_candidate_hide_window (chewing->candidate);
}

static void
on_candidate_clicked (NimfEngine  *engine,
                      NimfContext *target,
                      gchar       *text,
                      gint         index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfChewing *chewing = NIMF_CHEWING (engine);

  chewing_handle_Default (chewing->context, (index + 1) % 10 + 48);
  nimf_chewing_update (engine, target);
}

static void
on_candidate_scrolled (NimfEngine  *engine,
                       NimfContext *target,
                       gdouble      value)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfChewing *chewing = NIMF_CHEWING (engine);

  if ((gint) value == chewing_cand_CurrentPage (chewing->context) + 1)
    return;

  while (chewing_cand_TotalPage (chewing->context) > 1)
  {
    gint d = (gint) value - (chewing_cand_CurrentPage (chewing->context) + 1);

    if (d > 0)
      chewing_handle_PageDown (chewing->context);
    else if (d < 0)
      chewing_handle_PageUp (chewing->context);
    else if (d == 0)
      break;
  }

  nimf_chewing_update (engine, target);
}

gboolean
nimf_chewing_filter_event (NimfEngine  *engine,
                           NimfContext *target,
                           NimfEvent   *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfChewing *chewing = NIMF_CHEWING (engine);

  if (event->key.type == NIMF_EVENT_KEY_RELEASE)
    return FALSE;

  if ((event->key.state & NIMF_MODIFIER_MASK) == 0 ||
      (event->key.state & NIMF_MODIFIER_MASK) == NIMF_MOD2_MASK)
  {
    switch (event->key.keyval)
    {
      case NIMF_KEY_space:
        chewing_handle_Space (chewing->context);
        break;
      case NIMF_KEY_Escape:
        chewing_handle_Esc (chewing->context);
        break;
      case NIMF_KEY_Return:
      case NIMF_KEY_KP_Enter:
        chewing_handle_Enter (chewing->context);
        break;
      case NIMF_KEY_Delete:
      case NIMF_KEY_KP_Delete:
        chewing_handle_Del (chewing->context);
        break;
      case NIMF_KEY_BackSpace:
        chewing_handle_Backspace (chewing->context);
        break;
      case NIMF_KEY_Tab:
        chewing_handle_Tab (chewing->context);
        break;
      case NIMF_KEY_Left:
      case NIMF_KEY_KP_Left:
        chewing_handle_Left (chewing->context);
        break;
      case NIMF_KEY_Right:
      case NIMF_KEY_KP_Right:
        chewing_handle_Right (chewing->context);
        break;
      case NIMF_KEY_Up:
      case NIMF_KEY_KP_Up:
        chewing_handle_Up (chewing->context);
        break;
      case NIMF_KEY_Home:
        chewing_handle_Home (chewing->context);
        break;
      case NIMF_KEY_End:
        chewing_handle_End (chewing->context);
        break;
      case NIMF_KEY_Page_Up:
      case NIMF_KEY_KP_Page_Up:
        chewing_handle_PageUp (chewing->context);
        break;
      case NIMF_KEY_Page_Down:
      case NIMF_KEY_KP_Page_Down:
        chewing_handle_PageDown (chewing->context);
        break;
      case NIMF_KEY_Down:
      case NIMF_KEY_KP_Down:
        chewing_handle_Down (chewing->context);
        break;
      case NIMF_KEY_Caps_Lock:
        chewing_handle_Capslock (chewing->context);
        break;
      case NIMF_KEY_KP_Multiply:
      case NIMF_KEY_KP_Add:
      case NIMF_KEY_KP_Subtract:
      case NIMF_KEY_KP_Decimal:
      case NIMF_KEY_KP_Divide:
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
        chewing_handle_Numlock (chewing->context, event->key.keyval);
        break;
      default:
        chewing_handle_Default (chewing->context, event->key.keyval);
        break;
    }
  }
  else if ((event->key.state & NIMF_MODIFIER_MASK) == NIMF_SHIFT_MASK ||
           (event->key.state & NIMF_MODIFIER_MASK) == (NIMF_SHIFT_MASK | NIMF_MOD2_MASK))
  {
    switch (event->key.keyval)
    {
      case NIMF_KEY_Left:
      case NIMF_KEY_KP_Left:
        chewing_handle_ShiftLeft (chewing->context);
        break;
      case NIMF_KEY_Right:
      case NIMF_KEY_KP_Right:
        chewing_handle_ShiftRight (chewing->context);
        break;
      case NIMF_KEY_space:
        chewing_handle_ShiftSpace (chewing->context);
        break;
      default:
        chewing_handle_Default (chewing->context, event->key.keyval);
        break;
    }
  }
  else if ((event->key.state & NIMF_MODIFIER_MASK) == NIMF_CONTROL_MASK ||
           (event->key.state & NIMF_MODIFIER_MASK) == (NIMF_CONTROL_MASK | NIMF_MOD2_MASK))
  {
    if ((event->key.keyval >= NIMF_KEY_0 &&
         event->key.keyval <= NIMF_KEY_9) ||
        (event->key.keyval >= NIMF_KEY_KP_0 &&
         event->key.keyval <= NIMF_KEY_KP_9))
      chewing_handle_CtrlNum (chewing->context, event->key.keyval);
    else
      chewing_handle_Default (chewing->context, event->key.keyval);
  }

  nimf_chewing_update (engine, target);

  if (chewing_keystroke_CheckAbsorb (chewing->context))
    return TRUE;
  else if (chewing_keystroke_CheckIgnore (chewing->context))
    return FALSE;

  return TRUE;
}

static void
nimf_chewing_init (NimfChewing *chewing)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gint keys[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
  chewing->candidate = nimf_candidate_get_default ();
  chewing->id      = g_strdup ("nimf-chewing");
  chewing->preedit = g_string_new ("");
  chewing->preedit_attrs  = g_malloc0_n (3, sizeof (NimfPreeditAttr *));
  chewing->preedit_attrs[0] = nimf_preedit_attr_new (NIMF_PREEDIT_ATTR_UNDERLINE, 0, 0);
  chewing->preedit_attrs[1] = nimf_preedit_attr_new (NIMF_PREEDIT_ATTR_HIGHLIGHT, 0, 0);
  chewing->preedit_attrs[2] = NULL;

  chewing->context = chewing_new ();
  chewing_set_candPerPage          (chewing->context, 10);
  chewing_set_maxChiSymbolLen      (chewing->context, 16);
  chewing_set_addPhraseDirection   (chewing->context, FALSE);
  chewing_set_phraseChoiceRearward (chewing->context, FALSE);
  chewing_set_autoShiftCur         (chewing->context, FALSE);
  chewing_set_spaceAsSelection     (chewing->context, TRUE);
  chewing_set_escCleanAllBuf       (chewing->context, TRUE);
  chewing_set_selKey               (chewing->context, keys, 10);
}

static void
nimf_chewing_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfChewing *chewing = NIMF_CHEWING (object);

  g_string_free (chewing->preedit, TRUE);
  nimf_preedit_attr_freev (chewing->preedit_attrs);
  g_free (chewing->id);
  chewing_delete (chewing->context);

  G_OBJECT_CLASS (nimf_chewing_parent_class)->finalize (object);
}

void
nimf_chewing_get_preedit_string (NimfEngine        *engine,
                                 gchar            **str,
                                 NimfPreeditAttr ***attrs,
                                 gint              *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfChewing *chewing = NIMF_CHEWING (engine);

  if (str)
    *str = g_strdup (chewing->preedit->str);

  if (attrs)
    *attrs = nimf_preedit_attrs_copy (chewing->preedit_attrs);

  if (cursor_pos)
    *cursor_pos = chewing_cursor_Current (chewing->context);
}

const gchar *
nimf_chewing_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_CHEWING (engine)->id;
}

const gchar *
nimf_chewing_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_CHEWING (engine)->id;
}

static void
nimf_chewing_class_init (NimfChewingClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  NimfEngineClass *engine_class = NIMF_ENGINE_CLASS (class);

  engine_class->filter_event       = nimf_chewing_filter_event;
  engine_class->get_preedit_string = nimf_chewing_get_preedit_string;
  engine_class->reset              = nimf_chewing_reset;
  engine_class->focus_in           = nimf_chewing_focus_in;
  engine_class->focus_out          = nimf_chewing_focus_out;

  engine_class->candidate_clicked  = on_candidate_clicked;
  engine_class->candidate_scrolled = on_candidate_scrolled;

  engine_class->get_id             = nimf_chewing_get_id;
  engine_class->get_icon_name      = nimf_chewing_get_icon_name;

  object_class->finalize = nimf_chewing_finalize;
}

static void
nimf_chewing_class_finalize (NimfChewingClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_chewing_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_chewing_get_type ();
}
