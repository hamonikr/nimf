/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * im-nimf-qt5.cpp
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2018 Hodong Kim <cogniti@gmail.com>
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

#include <QTextFormat>
#include <QInputMethodEvent>
#include <QtGui/qpa/qplatforminputcontext.h>
#include <QtGui/qpa/qplatforminputcontextplugin_p.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include "nimf-im.h"

class NimfEventHandler : public QObject
{
  Q_OBJECT

public:
  NimfEventHandler(NimfIM *im)
  {
    m_im = im;
  };

  ~NimfEventHandler()
  {};

protected:
  bool eventFilter(QObject *obj, QEvent *event);

private:
  NimfIM *m_im;
};

bool NimfEventHandler::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::MouseButtonPress)
    nimf_im_reset (m_im);

  return QObject::eventFilter(obj, event);
}

class NimfInputContext : public QPlatformInputContext
{
  Q_OBJECT
public:
   NimfInputContext ();
  ~NimfInputContext ();

  virtual bool isValid () const;
  virtual void reset ();
  virtual void commit ();
  virtual void update (Qt::InputMethodQueries);
  virtual void invokeAction (QInputMethod::Action, int cursorPosition);
  virtual bool filterEvent (const QEvent *event);
  virtual QRectF keyboardRect () const;
  virtual bool isAnimating () const;
  virtual void showInputPanel ();
  virtual void hideInputPanel ();
  virtual bool isInputPanelVisible () const;
  virtual QLocale locale () const;
  virtual Qt::LayoutDirection inputDirection() const;
  virtual void setFocusObject (QObject *object);

  // nimf signal callbacks
  static void     on_preedit_start        (NimfIM      *im,
                                           gpointer     user_data);
  static void     on_preedit_end          (NimfIM      *im,
                                           gpointer     user_data);
  static void     on_preedit_changed      (NimfIM      *im,
                                           gpointer     user_data);
  static void     on_commit               (NimfIM      *im,
                                           const gchar *text,
                                           gpointer     user_data);
  static gboolean on_retrieve_surrounding (NimfIM      *im,
                                           gpointer     user_data);
  static gboolean on_delete_surrounding   (NimfIM      *im,
                                           gint         offset,
                                           gint         n_chars,
                                           gpointer     user_data);
  static void     on_beep                 (NimfIM      *im,
                                           gpointer     user_data);
  // settings
  static void on_changed_reset_on_mouse_button_press (GSettings *settings,
                                                      gchar     *key,
                                                      gpointer   user_data);
private:
  NimfIM           *m_im;
  NimfRectangle     m_cursor_area;
  GSettings        *m_settings;
  NimfEventHandler *m_handler;
};

/* nimf signal callbacks */
void
NimfInputContext::on_preedit_start (NimfIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
NimfInputContext::on_preedit_end (NimfIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
NimfInputContext::on_preedit_changed (NimfIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditAttr **preedit_attrs;
  gchar            *str;
  gint              cursor_pos;
  gint              i;

  nimf_im_get_preedit_string (im, &str, &preedit_attrs, &cursor_pos);
  QString preeditText = QString::fromUtf8 (str);
  g_free (str);
  QList <QInputMethodEvent::Attribute> attrs;

  // preedit text attribute
  for (i = 0; preedit_attrs[i] != NULL; i++)
  {
    QTextCharFormat format;

    switch (preedit_attrs[i]->type)
    {
      case NIMF_PREEDIT_ATTR_HIGHLIGHT:
        format.setBackground(Qt::green);
        format.setForeground(Qt::black);
        break;
      case NIMF_PREEDIT_ATTR_UNDERLINE:
        format.setUnderlineStyle(QTextCharFormat::DashUnderline);
        break;
      default:
        format.setUnderlineStyle(QTextCharFormat::DashUnderline);
        break;
    }

    QInputMethodEvent::Attribute attr (QInputMethodEvent::TextFormat,
                                       preedit_attrs[i]->start_index,
                                       preedit_attrs[i]->end_index - preedit_attrs[i]->start_index,
                                       format);
    attrs << attr;
  }
  // cursor attribute
  attrs << QInputMethodEvent::Attribute (QInputMethodEvent::Cursor,
                                         cursor_pos, true, 0);

  QInputMethodEvent event (preeditText, attrs);
  QObject *object = qApp->focusObject ();

  if (!object)
    return;

  QCoreApplication::sendEvent (object, &event);

  nimf_preedit_attr_freev (preedit_attrs);
}

void
NimfInputContext::on_commit (NimfIM      *im,
                             const gchar *text,
                             gpointer     user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  QString str = QString::fromUtf8 (text);
  QInputMethodEvent event;
  event.setCommitString (str);

  QObject *obj = qApp->focusObject();

  if (!obj)
    return;

  QCoreApplication::sendEvent (obj, &event);
}

gboolean
NimfInputContext::on_retrieve_surrounding (NimfIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return FALSE;
}

gboolean
NimfInputContext::on_delete_surrounding (NimfIM   *im,
                                         gint      offset,
                                         gint      n_chars,
                                         gpointer  user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return FALSE;
}

void
NimfInputContext::on_beep (NimfIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  QApplication::beep();
}

void
NimfInputContext::on_changed_reset_on_mouse_button_press (GSettings *settings,
                                                          gchar     *key,
                                                          gpointer   user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfInputContext *context = static_cast<NimfInputContext *>(user_data);

  if (g_settings_get_boolean (settings, key))
  {
    if (context->m_handler == NULL)
    {
      context->m_handler = new NimfEventHandler(context->m_im);
      qApp->installEventFilter(context->m_handler);
    }
  }
  else
  {
    if (context->m_handler)
    {
      qApp->removeEventFilter(context->m_handler);
      delete context->m_handler;
      context->m_handler = NULL;
    }
  }
}

NimfInputContext::NimfInputContext ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  m_settings = g_settings_new ("org.nimf.clients.qt5");
  m_im = nimf_im_new ();

  g_signal_connect (m_im, "preedit-start",
                    G_CALLBACK (NimfInputContext::on_preedit_start), this);
  g_signal_connect (m_im, "preedit-end",
                    G_CALLBACK (NimfInputContext::on_preedit_end), this);
  g_signal_connect (m_im, "preedit-changed",
                    G_CALLBACK (NimfInputContext::on_preedit_changed), this);
  g_signal_connect (m_im, "commit",
                    G_CALLBACK (NimfInputContext::on_commit), this);
  g_signal_connect (m_im, "retrieve-surrounding",
                    G_CALLBACK (NimfInputContext::on_retrieve_surrounding),
                    this);
  g_signal_connect (m_im, "delete-surrounding",
                    G_CALLBACK (NimfInputContext::on_delete_surrounding),
                    this);
  g_signal_connect (m_im, "beep",
                    G_CALLBACK (NimfInputContext::on_beep), this);

  g_signal_connect (m_settings, "changed::reset-on-mouse-button-press",
                    G_CALLBACK (NimfInputContext::on_changed_reset_on_mouse_button_press), this);
  m_handler = NULL;
  g_signal_emit_by_name (m_settings, "changed::reset-on-mouse-button-press",
                                     "reset-on-mouse-button-press");
}

NimfInputContext::~NimfInputContext ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (m_handler)
    delete m_handler;

  g_object_unref (m_im);
  g_object_unref (m_settings);
}

bool
NimfInputContext::isValid () const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return true;
}

void
NimfInputContext::reset ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  nimf_im_reset (m_im);
}

void
NimfInputContext::commit ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  nimf_im_reset (m_im);
}

void
NimfInputContext::update (Qt::InputMethodQueries queries) /* FIXME */
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (queries & Qt::ImCursorRectangle)
  {
    QWidget *widget = qApp->focusWidget ();

    if (widget == NULL)
      return;

    QRect  rect  = widget->inputMethodQuery(Qt::ImCursorRectangle).toRect();
    QPoint point = widget->mapToGlobal (QPoint (0, 0));
    rect.translate (point);

    if (m_cursor_area.x      != rect.x ()     ||
        m_cursor_area.y      != rect.y ()     ||
        m_cursor_area.width  != rect.width () ||
        m_cursor_area.height != rect.height ())
    {
      m_cursor_area.x      = rect.x ();
      m_cursor_area.y      = rect.y ();
      m_cursor_area.width  = rect.width ();
      m_cursor_area.height = rect.height ();

      nimf_im_set_cursor_location (m_im, &m_cursor_area);
    }
  }
}

void
NimfInputContext::invokeAction(QInputMethod::Action, int cursorPosition)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

bool
NimfInputContext::filterEvent (const QEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!qApp->focusObject() || !inputMethodAccepted()))
    return false;

  gboolean         retval;
  const QKeyEvent *key_event = static_cast<const QKeyEvent *>( event );
  NimfEvent       *nimf_event;
  NimfEventType    type = NIMF_EVENT_NOTHING;

  switch (event->type ())
  {
#undef KeyPress
    case QEvent::KeyPress:
      type = NIMF_EVENT_KEY_PRESS;
      break;
#undef KeyRelease
    case QEvent::KeyRelease:
      type = NIMF_EVENT_KEY_RELEASE;
      break;
    default:
      return false;
  }

  nimf_event = nimf_event_new (type);
  nimf_event->key.state            = key_event->nativeModifiers  ();
  nimf_event->key.keyval           = key_event->nativeVirtualKey ();
  nimf_event->key.hardware_keycode = key_event->nativeScanCode   (); /* FIXME: guint16 quint32 */

  retval = nimf_im_filter_event (m_im, nimf_event);
  nimf_event_free (nimf_event);

  return retval;
}

QRectF
NimfInputContext::keyboardRect() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return QRectF ();
}

bool
NimfInputContext::isAnimating() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return false;
}

void
NimfInputContext::showInputPanel()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
NimfInputContext::hideInputPanel()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

bool
NimfInputContext::isInputPanelVisible() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return false;
}

QLocale
NimfInputContext::locale() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return QLocale ();
}

Qt::LayoutDirection
NimfInputContext::inputDirection() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return Qt::LayoutDirection ();
}

void
NimfInputContext::setFocusObject (QObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!object || !inputMethodAccepted())
    nimf_im_focus_out (m_im);

  QPlatformInputContext::setFocusObject (object);

  if (object && inputMethodAccepted())
    nimf_im_focus_in (m_im);

  update (Qt::ImCursorRectangle);
}

/*
 * class NimfInputContextPlugin
 */
class NimfInputContextPlugin : public QPlatformInputContextPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID
    QPlatformInputContextFactoryInterface_iid
    FILE "./nimf.json")

public:
  NimfInputContextPlugin ()
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);
  }

  ~NimfInputContextPlugin ()
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);
  }

  virtual QStringList keys () const
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    return QStringList () <<  "nimf";
  }

  virtual QPlatformInputContext *create (const QString     &key,
                                         const QStringList &paramList)
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    return new NimfInputContext ();
  }
};

#include "im-nimf-qt5.moc"
