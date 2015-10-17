/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * im-dasom-qt5.cpp
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

#include <QTextFormat>
#include <QInputMethodEvent>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatforminputcontextplugin_p.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include "dasom-im.h"

class DasomInputContext : public QPlatformInputContext
{
  Q_OBJECT
public:
   DasomInputContext ();
  ~DasomInputContext ();

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

  // dasom signal callbacks
  static void     on_preedit_start        (DasomIM     *im,
                                           gpointer     user_data);
  static void     on_preedit_end          (DasomIM     *im,
                                           gpointer     user_data);
  static void     on_preedit_changed      (DasomIM     *im,
                                           gpointer     user_data);
  static void     on_commit               (DasomIM     *im,
                                           const gchar *text,
                                           gpointer     user_data);
  static gboolean on_retrieve_surrounding (DasomIM     *im,
                                           gpointer     user_data);
  static gboolean on_delete_surrounding   (DasomIM     *im,
                                           gint         offset,
                                           gint         n_chars,
                                           gpointer     user_data);
private:
  DasomIM        *m_im;
  DasomRectangle  m_cursor_area;
};

/* dasom signal callbacks */
void
DasomInputContext::on_preedit_start (DasomIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
DasomInputContext::on_preedit_end (DasomIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
DasomInputContext::on_preedit_changed (DasomIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar *str;
  gint   cursor_pos;
  dasom_im_get_preedit_string (im, &str, &cursor_pos);

  QString preeditText = QString::fromUtf8 (str);
  g_free (str);

  QList <QInputMethodEvent::Attribute> attrs;

  QTextCharFormat format = QTextCharFormat();
  format.setUnderlineStyle (QTextCharFormat::DashUnderline);
  // preedit text attribute
  attrs << QInputMethodEvent::Attribute (QInputMethodEvent::TextFormat,
                                         0, preeditText.length (),
                                         format);
  // cursor attribute
  attrs << QInputMethodEvent::Attribute (QInputMethodEvent::Cursor,
                                         cursor_pos, true, 0);

  QInputMethodEvent event (preeditText, attrs);
  QObject *object = qApp->focusObject ();

  if (!object)
    return;

  QCoreApplication::sendEvent (object, &event);
}

void
DasomInputContext::on_commit (DasomIM     *im,
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
DasomInputContext::on_retrieve_surrounding (DasomIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return FALSE;
}

gboolean
DasomInputContext::on_delete_surrounding (DasomIM *im,
                                          gint     offset,
                                          gint     n_chars,
                                          gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return FALSE;
}

DasomInputContext::DasomInputContext ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  m_im = dasom_im_new ();
  g_signal_connect (m_im, "preedit-start",
                    G_CALLBACK (DasomInputContext::on_preedit_start), this);
  g_signal_connect (m_im, "preedit-end",
                    G_CALLBACK (DasomInputContext::on_preedit_end), this);
  g_signal_connect (m_im, "preedit-changed",
                    G_CALLBACK (DasomInputContext::on_preedit_changed), this);
  g_signal_connect (m_im, "commit",
                    G_CALLBACK (DasomInputContext::on_commit), this);
  g_signal_connect (m_im, "retrieve-surrounding",
                    G_CALLBACK (DasomInputContext::on_retrieve_surrounding),
                    this);
  g_signal_connect (m_im, "delete-surrounding",
                    G_CALLBACK (DasomInputContext::on_delete_surrounding),
                    this);
}

DasomInputContext::~DasomInputContext ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_object_unref (m_im);
}

bool
DasomInputContext::isValid () const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return true;
}

void
DasomInputContext::reset ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  dasom_im_reset (m_im);
}

void
DasomInputContext::commit ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  dasom_im_reset (m_im);
}

void
DasomInputContext::update (Qt::InputMethodQueries queries) /* FIXME */
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

      dasom_im_set_cursor_location (m_im, &m_cursor_area);
    }
  }
}

void
DasomInputContext::invokeAction(QInputMethod::Action, int cursorPosition)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

bool
DasomInputContext::filterEvent (const QEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean         retval;
  const QKeyEvent *key_event = static_cast<const QKeyEvent *>( event );
  DasomEvent      *dasom_event;
  DasomEventType   type = DASOM_EVENT_NOTHING;

  switch (event->type ())
  {
#undef KeyPress
    case QEvent::KeyPress:
      type = DASOM_EVENT_KEY_PRESS;
      break;
#undef KeyRelease
    case QEvent::KeyRelease:
      type = DASOM_EVENT_KEY_RELEASE;
      break;
    case QEvent::MouseButtonPress:
      dasom_im_reset (m_im);
      return false;
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
      return false;
    default:
      break;
  }

  dasom_event = dasom_event_new (type);
  dasom_event->key.state            = key_event->nativeModifiers  ();
  dasom_event->key.keyval           = key_event->nativeVirtualKey ();
  dasom_event->key.hardware_keycode = key_event->nativeScanCode   (); /* FIXME: guint16 quint32 */

  retval = dasom_im_filter_event (m_im, dasom_event);
  dasom_event_free (dasom_event);

  return retval;
}

QRectF
DasomInputContext::keyboardRect() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return QRectF ();
}

bool
DasomInputContext::isAnimating() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return false;
}

void
DasomInputContext::showInputPanel()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
DasomInputContext::hideInputPanel()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

bool
DasomInputContext::isInputPanelVisible() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return false;
}

QLocale
DasomInputContext::locale() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return QLocale ();
}

Qt::LayoutDirection
DasomInputContext::inputDirection() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return Qt::LayoutDirection ();
}

void
DasomInputContext::setFocusObject (QObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  QPlatformInputContext::setFocusObject (object);

  if (object)
    dasom_im_focus_in (m_im);
  else
    dasom_im_focus_out (m_im);

  update (Qt::ImCursorRectangle);
}

/*
 * class DasomInputContextPlugin
 */
class DasomInputContextPlugin : public QPlatformInputContextPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID
    QPlatformInputContextFactoryInterface_iid
    FILE "./dasom.json")

public:
  DasomInputContextPlugin ()
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);
  }

  ~DasomInputContextPlugin ()
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);
  }

  virtual QStringList keys () const
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    return QStringList () <<  "dasom";
  }

  virtual QPlatformInputContext *create (const QString     &key,
                                         const QStringList &paramList)
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    return new DasomInputContext ();
  }
};

#include "im-dasom-qt5.moc"
