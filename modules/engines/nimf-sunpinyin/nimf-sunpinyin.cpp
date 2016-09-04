/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-sunpinyin.cpp
 * This file is part of Nimf.
 *
 * Copyright (C) 2015,2016 Hodong Kim <cogniti@gmail.com>
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
#include <sunpinyin.h>
#include <glib/gi18n.h>

G_BEGIN_DECLS

class NimfWinHandler : public CIMIWinHandler
{
public:
  NimfWinHandler(NimfEngine *engine);

  virtual ~NimfWinHandler()
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);
  }

  virtual void commit(const TWCHAR* wstr);
  virtual void updatePreedit(const IPreeditString* ppd);
  virtual void updateCandidates(const ICandidateList* pcl);
  virtual void updateStatus(int key, int value);

private:
  NimfEngine *m_engine;
};

#define NIMF_TYPE_SUNPINYIN             (nimf_sunpinyin_get_type ())
#define NIMF_SUNPINYIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_SUNPINYIN, NimfSunpinyin))
#define NIMF_SUNPINYIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_SUNPINYIN, NimfSunpinyinClass))
#define NIMF_IS_SUNPINYIN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_SUNPINYIN))
#define NIMF_IS_SUNPINYIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_SUNPINYIN))
#define NIMF_SUNPINYIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_SUNPINYIN, NimfSunpinyinClass))

typedef struct _NimfSunpinyin      NimfSunpinyin;
typedef struct _NimfSunpinyinClass NimfSunpinyinClass;

struct _NimfSunpinyin
{
  NimfEngine parent_instance;

  NimfCandidate     *candidate;
  gchar             *id;
  gchar             *preedit_string;
  NimfPreeditAttr  **preedit_attrs;
  NimfPreeditState   preedit_state;

  CIMIView       *view;
  CHotkeyProfile *hotkey_profile;
  NimfWinHandler *win_handler;

  gchar *commit_str;
  const IPreeditString *ppd;
  const ICandidateList *pcl;
  gint  current_page;
  gint  n_pages;
};

struct _NimfSunpinyinClass
{
  /*< private >*/
  NimfEngineClass parent_class;
};

GType nimf_sunpinyin_get_type (void) G_GNUC_CONST;

NimfWinHandler::NimfWinHandler(NimfEngine *engine)
  : m_engine(engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
NimfWinHandler::commit(const TWCHAR* wstr)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (m_engine);

  g_free (pinyin->commit_str);
  pinyin->commit_str = g_ucs4_to_utf8 (wstr, -1, NULL, NULL, NULL);
}

static void
nimf_sunpinyin_update_preedit (NimfEngine  *engine,
                               NimfContext *target,
                               gchar       *new_preedit,
                               int          cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  /* preedit-start */
  if (pinyin->preedit_state == NIMF_PREEDIT_STATE_END && new_preedit[0] != 0)
  {
    pinyin->preedit_state = NIMF_PREEDIT_STATE_START;
    nimf_engine_emit_preedit_start (engine, target);
  }

  /* preedit-changed */
  if (pinyin->preedit_string[0] != 0 || new_preedit[0] != 0)
  {
    g_free (pinyin->preedit_string);
    pinyin->preedit_string = new_preedit;
    pinyin->preedit_attrs[0]->end_index = g_utf8_strlen (pinyin->preedit_string, -1);
    nimf_engine_emit_preedit_changed (engine, target,
                                      pinyin->preedit_string,
                                      pinyin->preedit_attrs, cursor_pos);
  }
  else
    g_free (new_preedit);

  /* preedit-end */
  if (pinyin->preedit_state == NIMF_PREEDIT_STATE_START &&
      pinyin->preedit_string[0] == 0)
  {
    pinyin->preedit_state = NIMF_PREEDIT_STATE_END;
    nimf_engine_emit_preedit_end (engine, target);
  }
}

void
NimfWinHandler::updatePreedit(const IPreeditString* ppd)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (ppd)
    NIMF_SUNPINYIN (m_engine)->ppd = ppd;
  else
    NIMF_SUNPINYIN (m_engine)->ppd = NULL;
}

void
NimfWinHandler::updateCandidates(const ICandidateList* pcl)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NIMF_SUNPINYIN (m_engine)->pcl = pcl;
}

void
NimfWinHandler::updateStatus(int key, int value)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

G_DEFINE_DYNAMIC_TYPE (NimfSunpinyin, nimf_sunpinyin, NIMF_TYPE_ENGINE);

static void
nimf_sunpinyin_init (NimfSunpinyin *pinyin)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  pinyin->candidate = nimf_candidate_get_default ();
  pinyin->id = g_strdup ("nimf-sunpinyin");
  pinyin->preedit_string   = g_strdup ("");
  pinyin->preedit_attrs    = (NimfPreeditAttr **) g_malloc0_n (2, sizeof (NimfPreeditAttr *));
  pinyin->preedit_attrs[0] = nimf_preedit_attr_new (NIMF_PREEDIT_ATTR_UNDERLINE, 0, 0);
  pinyin->preedit_attrs[1] = NULL;

  CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
  factory.setPinyinScheme(CSunpinyinSessionFactory::QUANPIN);
  factory.setCandiWindowSize(10);
  pinyin->view = factory.createSession();

  if (!pinyin->view)
  {
    g_warning (G_STRLOC ": %s: factory.createSession() failed.\n"
               "You probably need to install sunpinyin-data", G_STRFUNC);
    return;
  }

  pinyin->hotkey_profile = new CHotkeyProfile();
  pinyin->view->setHotkeyProfile(pinyin->hotkey_profile);

  pinyin->win_handler = new NimfWinHandler(NIMF_ENGINE (pinyin));
  pinyin->view->attachWinHandler(pinyin->win_handler);
}

static void
nimf_sunpinyin_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (object);

  g_free (pinyin->id);
  g_free (pinyin->preedit_string);
  nimf_preedit_attr_freev (pinyin->preedit_attrs);
  g_free (pinyin->commit_str);

  if (pinyin->view)
  {
    CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
    factory.destroySession(pinyin->view);
  }

  delete pinyin->win_handler;
  delete pinyin->hotkey_profile;

  G_OBJECT_CLASS (nimf_sunpinyin_parent_class)->finalize (object);
}

const gchar *
nimf_sunpinyin_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_SUNPINYIN (engine)->id;
}

const gchar *
nimf_sunpinyin_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_SUNPINYIN (engine)->id;
}

static void
nimf_sunpinyin_update_page (NimfEngine  *engine,
                            NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  gint i;

  pinyin->n_pages = (pinyin->pcl->total() + 9) / 10;
  nimf_candidate_clear (pinyin->candidate, target);

  for (i = 0; i < pinyin->pcl->size(); i++)
  {
    const TWCHAR  *wstr;
    wstr = pinyin->pcl->candiString(i);

    if (wstr)
    {
      gchar *item = g_ucs4_to_utf8 (wstr, -1, NULL, NULL, NULL);
      nimf_candidate_append (pinyin->candidate, item, NULL);
      g_free (item);
    }
  }

  nimf_candidate_set_page_values (pinyin->candidate, target,
                                  pinyin->current_page, pinyin->n_pages, 10);
}

void nimf_sunpinyin_update (NimfEngine  *engine,
                            NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  g_return_if_fail (pinyin->view != NULL);

  /* commit */
  if (pinyin->commit_str)
  {
    nimf_engine_emit_commit (NIMF_ENGINE (pinyin), target, pinyin->commit_str);
    g_free (pinyin->commit_str);
    pinyin->commit_str = NULL;
  }
  /* update preedit */
  if (pinyin->ppd)
  {
    if (G_UNLIKELY (pinyin->ppd->size() >= 1019))
      pinyin->view->updateWindows(pinyin->view->clearIC());

    const TWCHAR *wstr = pinyin->ppd->string();
    /* nimf_sunpinyin_update_preedit takes text */
    nimf_sunpinyin_update_preedit (engine, target,
                                   g_ucs4_to_utf8 (wstr, -1, NULL, NULL, NULL),
                                   pinyin->ppd->caret());
    pinyin->ppd = NULL;
  }
  /* update candidate */
  if (pinyin->pcl)
  {
    nimf_sunpinyin_update_page (engine, target);

    if (pinyin->pcl->size() > 0)
    {
      nimf_candidate_show_window (pinyin->candidate, target, FALSE);
      nimf_candidate_select_first_item_in_page (pinyin->candidate);
    }
    else
    {
      nimf_candidate_hide_window (pinyin->candidate);
    }
  }
}

void
nimf_sunpinyin_reset (NimfEngine  *engine,
                      NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  g_return_if_fail (pinyin->view != NULL);

  pinyin->view->updateWindows(pinyin->view->clearIC());
  nimf_sunpinyin_update (engine, target);
}

void
nimf_sunpinyin_focus_in (NimfEngine  *engine,
                         NimfContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  g_return_if_fail (pinyin->view != NULL);

  /* FIXME: This is a workaround for a bug of nimf-sunpinyin
   * "focus-in" may be the next "focus-out". So I put the code that performs
   * the clearIC(). And if you remove the following code gnome-terminal may
   * stop when you click the candidate item or another gnome-terminal window.
   */
  pinyin->view->updateWindows(pinyin->view->clearIC());
  pinyin->view->updateWindows(CIMIView::PREEDIT_MASK |
                              CIMIView::CANDIDATE_MASK);
}

void
nimf_sunpinyin_focus_out (NimfEngine  *engine,
                          NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  nimf_sunpinyin_reset (engine, target);
}

static gint
nimf_sunpinyin_get_current_page (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_SUNPINYIN (engine)->current_page;
}

static gboolean
nimf_sunpinyin_page_up (NimfEngine *engine, NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  if (pinyin->current_page <= 1)
  {
    nimf_candidate_select_first_item_in_page (pinyin->candidate);
    return FALSE;
  }

  pinyin->current_page--;
  pinyin->view->onCandidatePageRequest(-1, true);
  nimf_sunpinyin_update_page (engine, target);
  nimf_candidate_select_last_item_in_page (pinyin->candidate);

  return TRUE;
}

static gboolean
nimf_sunpinyin_page_down (NimfEngine *engine, NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  if (pinyin->current_page >= pinyin->n_pages)
  {
    nimf_candidate_select_last_item_in_page (pinyin->candidate);
    return FALSE;
  }

  pinyin->current_page++;
  pinyin->view->onCandidatePageRequest(1, true);
  nimf_sunpinyin_update_page (engine, target);
  nimf_candidate_select_first_item_in_page (pinyin->candidate);

  return TRUE;
}

static void
nimf_sunpinyin_page_home (NimfEngine *engine, NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  if (pinyin->current_page <= 1)
  {
    nimf_candidate_select_first_item_in_page (pinyin->candidate);
    return;
  }

  pinyin->current_page = 1;
  pinyin->view->onCandidatePageRequest(0, false);
  nimf_sunpinyin_update_page (engine, target);
  nimf_candidate_select_first_item_in_page (pinyin->candidate);
}

static void
nimf_sunpinyin_page_end (NimfEngine *engine, NimfContext *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  if (pinyin->current_page >= pinyin->n_pages)
  {
    nimf_candidate_select_last_item_in_page (pinyin->candidate);
    return;
  }

  pinyin->current_page = pinyin->n_pages;
  pinyin->view->onCandidatePageRequest(pinyin->n_pages - 1, false);
  nimf_sunpinyin_update_page (engine, target);
  nimf_candidate_select_last_item_in_page (pinyin->candidate);
}

static void
on_candidate_scrolled (NimfEngine  *engine,
                       NimfContext *target,
                       gdouble      value)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  if ((gint) value == nimf_sunpinyin_get_current_page (engine))
    return;

  while (pinyin->n_pages > 1)
  {
    gint d = (gint) value - nimf_sunpinyin_get_current_page (engine);

    if (d > 0)
      nimf_sunpinyin_page_down (engine, target);
    else if (d < 0)
      nimf_sunpinyin_page_up (engine, target);
    else if (d == 0)
      break;
  }
}

gboolean
nimf_sunpinyin_filter_event (NimfEngine  *engine,
                             NimfContext *target,
                             NimfEvent   *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  g_return_val_if_fail (pinyin->view != NULL, FALSE);

  gboolean retval = FALSE;

  if (event->key.type == NIMF_EVENT_KEY_RELEASE)
    return FALSE;

  if (nimf_candidate_is_window_visible (pinyin->candidate))
  {
    switch (event->key.keyval)
    {
      case NIMF_KEY_Return:
      case NIMF_KEY_KP_Enter:
      case NIMF_KEY_space:
        {
          gint index = nimf_candidate_get_selected_index (pinyin->candidate);

          if (G_LIKELY (index >= 0))
          {
            pinyin->view->onCandidateSelectRequest(index);
            nimf_sunpinyin_update (engine, target);

            return TRUE;
          }
        }
        break;
        case NIMF_KEY_Up:
        case NIMF_KEY_KP_Up:
          nimf_candidate_select_previous_item (pinyin->candidate);
          return TRUE;
        case NIMF_KEY_Down:
        case NIMF_KEY_KP_Down:
          nimf_candidate_select_next_item (pinyin->candidate);
          return TRUE;
        case NIMF_KEY_Page_Up:
        case NIMF_KEY_KP_Page_Up:
          nimf_sunpinyin_page_up (engine, target);
          return TRUE;
        case NIMF_KEY_Page_Down:
        case NIMF_KEY_KP_Page_Down:
          nimf_sunpinyin_page_down (engine, target);
          return TRUE;
        case NIMF_KEY_Home:
          nimf_sunpinyin_page_home (engine, target);
          return TRUE;
        case NIMF_KEY_End:
          nimf_sunpinyin_page_end (engine, target);
          return TRUE;
        case NIMF_KEY_Escape:
          nimf_candidate_hide_window (pinyin->candidate);
          return TRUE;
      default:
        break;
    }
  }

  pinyin->current_page = 1;
  retval = pinyin->view->onKeyEvent(CKeyEvent(event->key.keyval,
                                              event->key.keyval,
                                              event->key.state));
  nimf_sunpinyin_update (engine, target);

  return retval;
}

void
nimf_sunpinyin_get_preedit_string (NimfEngine        *engine,
                                   gchar            **str,
                                   NimfPreeditAttr ***attrs,
                                   gint              *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  if (str)
    *str = g_strdup (pinyin->preedit_string);

  if (attrs)
    *attrs = nimf_preedit_attrs_copy (pinyin->preedit_attrs);

  if (cursor_pos)
    *cursor_pos = g_utf8_strlen (pinyin->preedit_string, -1);
}

static void
on_candidate_clicked (NimfEngine  *engine,
                      NimfContext *target,
                      gchar       *text,
                      gint         index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NIMF_SUNPINYIN (engine)->view->onCandidateSelectRequest(index);
  nimf_sunpinyin_update (engine, target);
}

static void
nimf_sunpinyin_class_init (NimfSunpinyinClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass    *object_class    = G_OBJECT_CLASS (klass);
  NimfEngineClass *engine_class    = NIMF_ENGINE_CLASS (klass);

  engine_class->get_id             = nimf_sunpinyin_get_id;
  engine_class->get_icon_name      = nimf_sunpinyin_get_icon_name;
  engine_class->focus_in           = nimf_sunpinyin_focus_in;
  engine_class->focus_out          = nimf_sunpinyin_focus_out;
  engine_class->reset              = nimf_sunpinyin_reset;
  engine_class->filter_event       = nimf_sunpinyin_filter_event;
  engine_class->get_preedit_string = nimf_sunpinyin_get_preedit_string;
  engine_class->candidate_page_up   = nimf_sunpinyin_page_up;
  engine_class->candidate_page_down = nimf_sunpinyin_page_down;
  engine_class->candidate_clicked   = on_candidate_clicked;
  engine_class->candidate_scrolled  = on_candidate_scrolled;

  object_class->finalize           = nimf_sunpinyin_finalize;
}

static void
nimf_sunpinyin_class_finalize (NimfSunpinyinClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_sunpinyin_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_sunpinyin_get_type ();
}

G_END_DECLS
