/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * im-nimf-qt4.cpp
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2017 Hodong Kim <cogniti@gmail.com>
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

#include <QApplication>
#include <QTextFormat>
#include <QInputContext>
#include <QInputContextPlugin>
#include <nimf.h>

class NimfInputContext : public QInputContext
{
   Q_OBJECT
public:
   NimfInputContext ();
  ~NimfInputContext ();

  virtual QString identifierName ();
  virtual QString language       ();

  virtual void    reset          ();
  virtual void    update         ();
  virtual bool    isComposing    () const;
  virtual void    setFocusWidget (QWidget *w);
  virtual bool    filterEvent    (const QEvent *event);

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
private:
  NimfIM        *m_im;
  bool           m_isComposing;
  NimfRectangle  m_cursor_area;
};

/* nimf signal callbacks */
void
NimfInputContext::on_preedit_start (NimfIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfInputContext *context = static_cast<NimfInputContext *>(user_data);
  context->m_isComposing = true;
}

void
NimfInputContext::on_preedit_end (NimfIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfInputContext *context = static_cast<NimfInputContext *>(user_data);
  context->m_isComposing = false;
}

void
NimfInputContext::on_preedit_changed (NimfIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfInputContext *context = static_cast<NimfInputContext *>(user_data);

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
                                       QVariant (format));
    attrs << attr;
  }

  nimf_preedit_attr_freev (preedit_attrs);

  // cursor attribute
  attrs << QInputMethodEvent::Attribute (QInputMethodEvent::Cursor,
                                         cursor_pos, true, 0);

  QInputMethodEvent event (preeditText, attrs);
  context->sendEvent (event);
}

void
NimfInputContext::on_commit (NimfIM      *im,
                             const gchar *text,
                             gpointer     user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfInputContext *context = static_cast<NimfInputContext *>(user_data);
  QString str = QString::fromUtf8 (text);
  QInputMethodEvent event;
  event.setCommitString (str);
  context->sendEvent (event);
}

gboolean
NimfInputContext::on_retrieve_surrounding (NimfIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  // TODO
  return FALSE;
}

gboolean
NimfInputContext::on_delete_surrounding (NimfIM   *im,
                                         gint      offset,
                                         gint      n_chars,
                                         gpointer  user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  // TODO
  return FALSE;
}

void
NimfInputContext::on_beep (NimfIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  QApplication::beep();
}

NimfInputContext::NimfInputContext ()
  : m_isComposing(false)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

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
}

NimfInputContext::~NimfInputContext ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_object_unref (m_im);
}

QString
NimfInputContext::identifierName ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return QString ("nimf");
}

QString
NimfInputContext::language ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return QString ("");
}

void
NimfInputContext::reset ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_im_reset (m_im);
}

void
NimfInputContext::update ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  QWidget *widget = focusWidget ();

  if (widget)
  {
    QRect  rect  = widget->inputMethodQuery(Qt::ImMicroFocus).toRect();
    QPoint point = widget->mapToGlobal (QPoint(0,0));
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

bool
NimfInputContext::isComposing () const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return m_isComposing;
}

void
NimfInputContext::setFocusWidget (QWidget *w)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!w)
    nimf_im_focus_out (m_im);

  QInputContext::setFocusWidget (w);

  if (w)
    nimf_im_focus_in (m_im);

  update ();
}

bool
NimfInputContext::filterEvent (const QEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

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
    case QEvent::MouseButtonPress:
      /* TODO: Provide as a option */
      nimf_im_reset (m_im);
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

/*
 * class NimfInputContextPlugin
 */
class NimfInputContextPlugin : public QInputContextPlugin
{
  Q_OBJECT
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

    return QStringList () << "nimf";
  }

  virtual QInputContext *create (const QString &key)
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    return new NimfInputContext ();
  }

  virtual QStringList languages (const QString &key)
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    return QStringList () << "ko" << "zh" << "ja";
  }

  virtual QString displayName (const QString &key)
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    return QString ("Nimf");
  }

  virtual QString description (const QString &key)
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    return QString ("nimf Qt4 im module");
  }
};

Q_EXPORT_PLUGIN2 (NimfInputContextPlugin, NimfInputContextPlugin)

#include "im-nimf-qt4.moc"
