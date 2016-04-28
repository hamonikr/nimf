/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
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

  gchar            *id;
  gchar            *zh_name;
  gboolean          is_english_mode;
  gchar            *preedit_string;
  NimfPreeditState  preedit_state;

  GSettings       *settings;
  NimfKey         **zh_en_keys;

  CIMIView       *view;
  CHotkeyProfile *hotkey_profile;
  NimfWinHandler *win_handler;

  gchar *commit_str;
  const IPreeditString *ppd;
  const ICandidateList *pcl;
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
nimf_sunpinyin_update_preedit (NimfEngine     *engine,
                               NimfConnection *target,
                               guint16         client_id,
                               gchar          *new_preedit,
                               int             cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  /* preedit-start */
  if (pinyin->preedit_state == NIMF_PREEDIT_STATE_END && new_preedit[0] != 0)
  {
    pinyin->preedit_state = NIMF_PREEDIT_STATE_START;
    nimf_engine_emit_preedit_start (engine, target, client_id);
  }

  /* preedit-changed */
  if (pinyin->preedit_string[0] != 0 || new_preedit[0] != 0)
  {
    g_free (pinyin->preedit_string);
    pinyin->preedit_string = new_preedit;
    nimf_engine_emit_preedit_changed (engine, target, client_id,
                                      pinyin->preedit_string, cursor_pos);
  }
  else
    g_free (new_preedit);

  /* preedit-end */
  if (pinyin->preedit_state == NIMF_PREEDIT_STATE_START &&
      pinyin->preedit_string[0] == 0)
  {
    pinyin->preedit_state = NIMF_PREEDIT_STATE_END;
    nimf_engine_emit_preedit_end (engine, target, client_id);
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

  if (pcl)
    NIMF_SUNPINYIN (m_engine)->pcl = pcl;
  else
    NIMF_SUNPINYIN (m_engine)->pcl = NULL;
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

  gchar **zh_en_keys;

  pinyin->settings = g_settings_new ("org.nimf.engines.sunpinyin");
  zh_en_keys = g_settings_get_strv (pinyin->settings, "zh-en-keys");
  pinyin->zh_en_keys = nimf_key_newv ((const gchar **) zh_en_keys);
  g_strfreev (zh_en_keys);

  pinyin->id = g_strdup ("nimf-sunpinyin");
  pinyin->zh_name = g_strdup ("zh");
  pinyin->is_english_mode = TRUE;
  pinyin->preedit_string = g_strdup ("");

  CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
  factory.setPinyinScheme(CSunpinyinSessionFactory::QUANPIN);
  factory.setCandiWindowSize(10);
  pinyin->view = factory.createSession();

  if (!pinyin->view)
  {
    g_warning (G_STRLOC ": %s: factory.createSession() failed", G_STRFUNC);
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
  g_free (pinyin->zh_name);
  g_free (pinyin->preedit_string);
  g_free (pinyin->commit_str);
  nimf_key_freev (pinyin->zh_en_keys);
  g_object_unref (pinyin->settings);

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

  return NIMF_SUNPINYIN (engine)->zh_name;
}

void
nimf_sunpinyin_reset (NimfEngine     *engine,
                      NimfConnection *target,
                      guint16         client_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  pinyin->view->updateWindows(pinyin->view->clearIC());
}

void
nimf_sunpinyin_focus_in (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

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
nimf_sunpinyin_focus_out (NimfEngine     *engine,
                          NimfConnection *target,
                          guint16         client_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  nimf_sunpinyin_reset (engine, target, client_id);
}

void nimf_sunpinyin_update (NimfEngine     *engine,
                            NimfConnection *target,
                            guint16         client_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  /* commit */
  if (pinyin->commit_str)
  {
    nimf_engine_emit_commit (NIMF_ENGINE (pinyin), target, client_id,
                             pinyin->commit_str);
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
    nimf_sunpinyin_update_preedit (engine, target, client_id,
                                   g_ucs4_to_utf8 (wstr, -1, NULL, NULL, NULL),
                                   pinyin->ppd->caret());
    pinyin->ppd = NULL;
  }
  /* update candidate */
  if (pinyin->pcl)
  {
    const TWCHAR *wstr;
    gchar **strv = (gchar **) g_malloc0 ((pinyin->pcl->size() + 1) * sizeof (gchar *));
    gint i;

    for (i = 0; i < pinyin->pcl->size(); i++)
    {
      wstr = pinyin->pcl->candiString(i);

      if (wstr)
      {
        gchar *text = g_ucs4_to_utf8 (wstr, -1, NULL, NULL, NULL);
        strv[i] = text;
      }
    }

    nimf_engine_update_candidate_window (engine, (const gchar **) strv);
    g_strfreev (strv);

    if (pinyin->pcl->size() > 0)
      nimf_engine_show_candidate_window (engine, target, client_id);
    else
      nimf_engine_hide_candidate_window (engine);

    pinyin->pcl = NULL;
  }
}

gboolean
nimf_sunpinyin_filter_event (NimfEngine     *engine,
                             NimfConnection *target,
                             guint16         client_id,
                             NimfEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;
  NimfSunpinyin *pinyin = NIMF_SUNPINYIN (engine);

  if (event->key.type == NIMF_EVENT_KEY_RELEASE)
    return FALSE;

  if (nimf_event_matches (event, (const NimfKey **) pinyin->zh_en_keys))
  {
    nimf_sunpinyin_reset (engine, target, client_id);
    pinyin->is_english_mode = !pinyin->is_english_mode;
    nimf_engine_emit_engine_changed (engine, target);
    return TRUE;
  }

  if (pinyin->is_english_mode)
    return FALSE;

  switch (event->key.keyval)
  {
    case NIMF_KEY_Return:
    case NIMF_KEY_KP_Enter:
    case NIMF_KEY_space:
      {
        gint index = nimf_engine_get_selected_candidate_index (engine);

        if (G_LIKELY (index >= 0))
        {
          pinyin->view->onCandidateSelectRequest(index);
          nimf_sunpinyin_update (engine, target, client_id);

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

  retval = pinyin->view->onKeyEvent(CKeyEvent(event->key.keyval,
                                              event->key.keyval,
                                              event->key.state));
  nimf_sunpinyin_update (engine, target, client_id);

  return retval;
}

static void
on_candidate_clicked (NimfEngine     *engine,
                      NimfConnection *target,
                      guint16         client_id,
                      gchar          *text,
                      gint            index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NIMF_SUNPINYIN (engine)->view->onCandidateSelectRequest(index);
  nimf_sunpinyin_update (engine, target, client_id);
}

void
nimf_sunpinyin_set_english_mode (NimfEngine *engine,
                                 gboolean    is_english_mode)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NIMF_SUNPINYIN (engine)->is_english_mode = is_english_mode;
}

gboolean
nimf_sunpinyin_get_english_mode (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_SUNPINYIN (engine)->is_english_mode;
}

static void
nimf_sunpinyin_class_init (NimfSunpinyinClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  NimfEngineClass *engine_class = NIMF_ENGINE_CLASS (klass);

  engine_class->get_id            = nimf_sunpinyin_get_id;
  engine_class->get_icon_name     = nimf_sunpinyin_get_icon_name;
  engine_class->set_english_mode  = nimf_sunpinyin_set_english_mode;
  engine_class->get_english_mode  = nimf_sunpinyin_get_english_mode;
  engine_class->candidate_clicked = on_candidate_clicked;

  engine_class->focus_in          = nimf_sunpinyin_focus_in;
  engine_class->focus_out         = nimf_sunpinyin_focus_out;
  engine_class->reset             = nimf_sunpinyin_reset;
  engine_class->filter_event      = nimf_sunpinyin_filter_event;

  object_class->finalize          = nimf_sunpinyin_finalize;
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
