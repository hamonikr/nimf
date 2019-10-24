/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * im-nimf-qt5.cpp
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2019 Hodong Kim <cogniti@gmail.com>
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
#include <gio/gio.h>

#ifdef USE_DLFCN

#include <dlfcn.h>

typedef struct
{
  NimfIM * (* im_new)                 (void);
  void     (* im_focus_in)            (NimfIM              *im);
  void     (* im_focus_out)           (NimfIM              *im);
  void     (* im_reset)               (NimfIM              *im);
  gboolean (* im_filter_event)        (NimfIM              *im,
                                       NimfEvent           *event);
  void     (* im_get_preedit_string)  (NimfIM              *im,
                                       gchar              **str,
                                       NimfPreeditAttr   ***attrs,
                                       gint                *cursor_pos);
  void     (* im_set_cursor_location) (NimfIM              *im,
                                       const NimfRectangle *area);
  void     (* im_set_use_preedit)     (NimfIM              *im,
                                       gboolean             use_preedit);
  void     (* im_set_surrounding)     (NimfIM              *im,
                                       const char          *text,
                                       gint                 len,
                                       gint                 cursor_index);
  NimfEvent * (* event_new)           (NimfEventType     type);
  void        (* event_free)          (NimfEvent        *event);
  void        (* preedit_attr_freev)  (NimfPreeditAttr **attrs);
} NimfAPI;

typedef struct
{
  void (* free) (gpointer mem);
} GLibAPI;

typedef struct
{
  gulong (* signal_connect_data) (gpointer       instance,
                                  const gchar   *detailed_signal,
                                  GCallback      c_handler,
                                  gpointer       data,
                                  GClosureNotify destroy_data,
                                  GConnectFlags  connect_flags);
  void   (* signal_emit_by_name) (gpointer       instance,
                                  const gchar   *detailed_signal,
                                  ...);
  void   (* object_unref)        (gpointer       object);
} GObjectAPI;

typedef struct
{
  GSettings * (* settings_new)         (const gchar   *schema_id);
  gboolean    (* settings_get_boolean) (GSettings     *settings,
                                        const gchar   *key);
  GSettingsSchemaSource *
              (* settings_schema_source_get_default) (void);
  GSettingsSchema *
              (* settings_schema_source_lookup)
                                        (GSettingsSchemaSource *source,
                                         const gchar           *schema_id,
                                         gboolean               recursive);
  void        (* settings_schema_unref) (GSettingsSchema       *schema);
} GIOAPI;

void *libnimf    = NULL;
void *libglib    = NULL;
void *libgobject = NULL;
void *libgio     = NULL;
NimfAPI    *nimf_api    = NULL;
GLibAPI    *glib_api    = NULL;
GObjectAPI *gobject_api = NULL;
GIOAPI     *gio_api     = NULL;

#endif

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
#ifndef USE_DLFCN
    nimf_im_reset (m_im);
#else
    nimf_api->im_reset (m_im);
#endif

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
  GSettings        *m_settings;
  NimfEventHandler *m_handler;
  NimfRectangle     m_cursor_area;
};

/* nimf signal callbacks */
void
NimfInputContext::on_preedit_start (NimfIM *im, gpointer user_data)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif
}

void
NimfInputContext::on_preedit_end (NimfIM *im, gpointer user_data)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif
}

void
NimfInputContext::on_preedit_changed (NimfIM *im, gpointer user_data)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

  NimfPreeditAttr **preedit_attrs;
  gchar            *str;
  gint              cursor_pos;
  gint              offset = 0;
  guint             i, j, len;

#ifndef USE_DLFCN
  nimf_im_get_preedit_string (im, &str, &preedit_attrs, &cursor_pos);
#else
  nimf_api->im_get_preedit_string (im, &str, &preedit_attrs, &cursor_pos);
#endif

  QString preeditText = QString::fromUtf8 (str);
#ifndef USE_DLFCN
  g_free (str);
#else
  glib_api->free (str);
#endif
  QList <QInputMethodEvent::Attribute> attrs;

  for (i = 0; i < (guint) preeditText.size(); i++)
  {
    if (preeditText.at(i).isLowSurrogate())
    {
      offset++;
      continue;
    }

    QTextCharFormat format;

    for (j = 0; preedit_attrs[j]; j++)
    {
      switch (preedit_attrs[j]->type)
      {
        case NIMF_PREEDIT_ATTR_HIGHLIGHT:
          if (preedit_attrs[j]->start_index <= i - offset &&
              preedit_attrs[j]->end_index   >  i - offset)
          {
            format.setBackground(Qt::green);
            format.setForeground(Qt::black);
          }
          break;
        case NIMF_PREEDIT_ATTR_UNDERLINE:
          if (preedit_attrs[j]->start_index <= i - offset &&
              preedit_attrs[j]->end_index   >  i - offset)
            format.setUnderlineStyle(QTextCharFormat::DashUnderline);
          break;
        default:
          break;
      }
    }

    preeditText.at(i).isHighSurrogate() ? len = 2 : len = 1;
    QInputMethodEvent::Attribute attr (QInputMethodEvent::TextFormat,
                                       i, len, format);
    attrs << attr;
  }

#ifndef USE_DLFCN
  nimf_preedit_attr_freev (preedit_attrs);
#else
  nimf_api->preedit_attr_freev (preedit_attrs);
#endif

  // cursor attribute
  attrs << QInputMethodEvent::Attribute (QInputMethodEvent::Cursor,
                                         cursor_pos + offset, true, 0);

  QInputMethodEvent event (preeditText, attrs);
  QObject *object = qApp->focusObject ();

  if (!object)
    return;

  QCoreApplication::sendEvent (object, &event);
}

void
NimfInputContext::on_commit (NimfIM      *im,
                             const gchar *text,
                             gpointer     user_data)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

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
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

  QObject *object = qApp->focusObject();

  if (!object)
    return FALSE;

  NimfInputContext *context = static_cast<NimfInputContext *>(user_data);

  QInputMethodQueryEvent surrounding_query (Qt::ImSurroundingText);
  QInputMethodQueryEvent position_query    (Qt::ImCursorPosition);

  QCoreApplication::sendEvent (object, &surrounding_query);
  QCoreApplication::sendEvent (object, &position_query);

  QString string = surrounding_query.value (Qt::ImSurroundingText).toString();
  uint pos = position_query.value (Qt::ImCursorPosition).toUInt();

#ifndef USE_DLFCN
  nimf_im_set_surrounding (context->m_im,
                           string.toUtf8().constData(), -1, pos);
#else
  nimf_api->im_set_surrounding (context->m_im,
                                string.toUtf8().constData(), -1, pos);
#endif

  return TRUE;
}

gboolean
NimfInputContext::on_delete_surrounding (NimfIM   *im,
                                         gint      offset,
                                         gint      n_chars,
                                         gpointer  user_data)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

  QObject *object = qApp->focusObject();

  if (!object)
    return FALSE;

  QInputMethodEvent event;
  event.setCommitString ("", offset, n_chars);
  QCoreApplication::sendEvent (object, &event);

  return TRUE;
}

void
NimfInputContext::on_beep (NimfIM *im, gpointer user_data)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

  QApplication::beep();
}

void
NimfInputContext::on_changed_reset_on_mouse_button_press (GSettings *settings,
                                                          gchar     *key,
                                                          gpointer   user_data)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

  NimfInputContext *context = static_cast<NimfInputContext *>(user_data);

#ifndef USE_DLFCN
  if (g_settings_get_boolean (settings, key))
#else
  if (gio_api->settings_get_boolean (settings, key))
#endif
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
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

  m_im                 = NULL;
  m_settings           = NULL;
  m_handler            = NULL;
  m_cursor_area.x      = 0;
  m_cursor_area.y      = 0;
  m_cursor_area.width  = 0;
  m_cursor_area.height = 0;

#ifndef USE_DLFCN
  m_im = nimf_im_new ();
  m_settings = g_settings_new ("org.nimf.clients.qt5");
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
  g_signal_emit_by_name (m_settings, "changed::reset-on-mouse-button-press",
                                     "reset-on-mouse-button-press");
#else
  if (!nimf_api || !glib_api || !gobject_api || !gio_api)
  {
    qWarning("The libraries for nimf are not ready.");
    return;
  }

  GSettingsSchemaSource *source;
  GSettingsSchema       *schema;

  source = gio_api->settings_schema_source_get_default ();
  schema = gio_api->settings_schema_source_lookup (source, "org.nimf.clients.qt5", TRUE);

  if (schema == NULL)
  {
    qWarning("org.nimf.clients.qt5 schema is not found.");
    return;
  }

  gio_api->settings_schema_unref (schema);

  m_im = nimf_api->im_new ();
  m_settings = gio_api->settings_new ("org.nimf.clients.qt5");
  gobject_api->signal_connect_data (m_im, "preedit-start",
                                    G_CALLBACK (NimfInputContext::on_preedit_start), this, NULL, (GConnectFlags) 0);
  gobject_api->signal_connect_data (m_im, "preedit-end",
                                    G_CALLBACK (NimfInputContext::on_preedit_end), this, NULL, (GConnectFlags) 0);
  gobject_api->signal_connect_data (m_im, "preedit-changed",
                                    G_CALLBACK (NimfInputContext::on_preedit_changed), this, NULL, (GConnectFlags) 0);
  gobject_api->signal_connect_data (m_im, "commit",
                                    G_CALLBACK (NimfInputContext::on_commit), this, NULL, (GConnectFlags) 0);
  gobject_api->signal_connect_data (m_im, "retrieve-surrounding",
                                    G_CALLBACK (NimfInputContext::on_retrieve_surrounding),
                                    this, NULL, (GConnectFlags) 0);
  gobject_api->signal_connect_data (m_im, "delete-surrounding",
                                    G_CALLBACK (NimfInputContext::on_delete_surrounding),
                                    this, NULL, (GConnectFlags) 0);
  gobject_api->signal_connect_data (m_im, "beep",
                                    G_CALLBACK (NimfInputContext::on_beep), this, NULL, (GConnectFlags) 0);
  gobject_api->signal_connect_data (m_settings, "changed::reset-on-mouse-button-press",
                                    G_CALLBACK (NimfInputContext::on_changed_reset_on_mouse_button_press), this, NULL, (GConnectFlags) 0);
  gobject_api->signal_emit_by_name (m_settings, "changed::reset-on-mouse-button-press",
                                    "reset-on-mouse-button-press");
#endif
}

NimfInputContext::~NimfInputContext ()
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

  if (m_handler)
    delete m_handler;

  if (m_im)
#ifndef USE_DLFCN
    g_object_unref (m_im);
#else
    gobject_api->object_unref (m_im);
#endif

  if (m_settings)
#ifndef USE_DLFCN
    g_object_unref (m_settings);
#else
    gobject_api->object_unref (m_settings);
#endif
}

bool
NimfInputContext::isValid () const
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

  if (m_im == NULL)
    return false;

  return true;
}

void
NimfInputContext::reset ()
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_im_reset (m_im);
#else
  nimf_api->im_reset (m_im);
#endif
}

void
NimfInputContext::commit ()
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_im_reset (m_im);
#else
  nimf_api->im_reset (m_im);
#endif
}

void
NimfInputContext::update (Qt::InputMethodQueries queries)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

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

#ifndef USE_DLFCN
      nimf_im_set_cursor_location (m_im, &m_cursor_area);
#else
      nimf_api->im_set_cursor_location (m_im, &m_cursor_area);
#endif
    }
  }
}

void
NimfInputContext::invokeAction(QInputMethod::Action, int cursorPosition)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif
}

bool
NimfInputContext::filterEvent (const QEvent *event)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

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

#ifndef USE_DLFCN
  nimf_event = nimf_event_new (type);
#else
  nimf_event = nimf_api->event_new (type);
#endif

  nimf_event->key.state            = key_event->nativeModifiers  ();
  nimf_event->key.keyval           = key_event->nativeVirtualKey ();
  nimf_event->key.hardware_keycode = key_event->nativeScanCode   ();

#ifndef USE_DLFCN
  retval = nimf_im_filter_event (m_im, nimf_event);
  nimf_event_free (nimf_event);
#else
  retval = nimf_api->im_filter_event (m_im, nimf_event);
  nimf_api->event_free (nimf_event);
#endif

  return retval;
}

QRectF
NimfInputContext::keyboardRect() const
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif
  return QRectF ();
}

bool
NimfInputContext::isAnimating() const
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif
  return false;
}

void
NimfInputContext::showInputPanel()
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif
}

void
NimfInputContext::hideInputPanel()
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif
}

bool
NimfInputContext::isInputPanelVisible() const
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif
  return false;
}

QLocale
NimfInputContext::locale() const
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif
  return QLocale ();
}

Qt::LayoutDirection
NimfInputContext::inputDirection() const
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif
  return Qt::LayoutDirection ();
}

void
NimfInputContext::setFocusObject (QObject *object)
{
#ifndef USE_DLFCN
  g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

  if (!object || !inputMethodAccepted())
#ifndef USE_DLFCN
    nimf_im_focus_out (m_im);
#else
    nimf_api->im_focus_out (m_im);
#endif

  QPlatformInputContext::setFocusObject (object);

  if (object && inputMethodAccepted())
#ifndef USE_DLFCN
    nimf_im_focus_in (m_im);
#else
    nimf_api->im_focus_in (m_im);
#endif

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
#ifndef USE_DLFCN
    g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

#ifdef USE_DLFCN
    libnimf    = dlopen ("libnimf.so.1",        RTLD_LAZY);
    libglib    = dlopen ("libglib-2.0.so.0",    RTLD_LAZY);
    libgobject = dlopen ("libgobject-2.0.so.0", RTLD_LAZY);
    libgio     = dlopen ("libgio-2.0.so.0",     RTLD_LAZY);

    if (libnimf)
    {
      nimf_api = new NimfAPI;
      nimf_api->im_new                 = reinterpret_cast<NimfIM* (*) ()> (dlsym (libnimf, "nimf_im_new"));
      nimf_api->im_focus_in            = reinterpret_cast<void (*) (NimfIM*)> (dlsym (libnimf, "nimf_im_focus_in"));
      nimf_api->im_focus_out           = reinterpret_cast<void (*) (NimfIM*)> (dlsym (libnimf, "nimf_im_focus_out"));
      nimf_api->im_reset               = reinterpret_cast<void (*) (NimfIM*)> (dlsym (libnimf, "nimf_im_reset"));
      nimf_api->im_filter_event        = reinterpret_cast<gboolean (*) (NimfIM*, NimfEvent*)> (dlsym (libnimf, "nimf_im_filter_event"));
      nimf_api->im_get_preedit_string  = reinterpret_cast<void (*) (NimfIM*, gchar**, NimfPreeditAttr***, gint*)> (dlsym (libnimf, "nimf_im_get_preedit_string"));
      nimf_api->im_set_cursor_location = reinterpret_cast<void (*) (NimfIM*, const NimfRectangle*)> (dlsym (libnimf, "nimf_im_set_cursor_location"));
      nimf_api->im_set_use_preedit     = reinterpret_cast<void (*) (NimfIM*, gboolean)> (dlsym (libnimf, "nimf_im_set_use_preedit"));
      nimf_api->im_set_surrounding     = reinterpret_cast<void (*) (NimfIM*, const char*, gint, gint)> (dlsym (libnimf, "nimf_im_set_surrounding"));
      nimf_api->event_new              = reinterpret_cast<NimfEvent * (*) (NimfEventType)> (dlsym (libnimf, "nimf_event_new"));
      nimf_api->event_free             = reinterpret_cast<void (*) (NimfEvent*)> (dlsym (libnimf, "nimf_event_free"));
      nimf_api->preedit_attr_freev     = reinterpret_cast<void (*) (NimfPreeditAttr**)> (dlsym (libnimf, "nimf_preedit_attr_freev"));
    }

    if (libglib)
    {
      glib_api = new GLibAPI;
      glib_api->free = reinterpret_cast<void (*) (gpointer)> (dlsym (libglib, "g_free"));
    }

    if (libgobject)
    {
      gobject_api = new GObjectAPI;
      gobject_api->signal_connect_data = reinterpret_cast<gulong (*) (gpointer, const gchar*, GCallback, gpointer, GClosureNotify, GConnectFlags)> (dlsym (libgobject, "g_signal_connect_data"));
      gobject_api->signal_emit_by_name = reinterpret_cast<void (*) (gpointer, const gchar*, ...)> (dlsym (libgobject, "g_signal_emit_by_name"));
      gobject_api->object_unref        = reinterpret_cast<void (*) (gpointer)> (dlsym (libgobject, "g_object_unref"));
    }

    if (libgio)
    {
      gio_api = new GIOAPI;
      gio_api->settings_new          = reinterpret_cast<GSettings* (*) (const gchar*)> (dlsym (libgio, "g_settings_new"));
      gio_api->settings_get_boolean  = reinterpret_cast<gboolean (*) (GSettings*, const gchar*)> (dlsym (libgio, "g_settings_get_boolean"));
      gio_api->settings_schema_source_get_default
                                     = reinterpret_cast<GSettingsSchemaSource* (*) ()> (dlsym (libgio, "g_settings_schema_source_get_default"));
      gio_api->settings_schema_source_lookup
                                     = reinterpret_cast<GSettingsSchema* (*) (GSettingsSchemaSource *, const gchar*, gboolean)> (dlsym (libgio, "g_settings_schema_source_lookup"));
      gio_api->settings_schema_unref = reinterpret_cast<void (*) (GSettingsSchema*)> (dlsym (libgio, "g_settings_schema_unref"));
    }
#endif
  }

  ~NimfInputContextPlugin ()
  {
#ifndef USE_DLFCN
    g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

#ifdef USE_DLFCN
    delete nimf_api;
    delete glib_api;
    delete gobject_api;
    delete gio_api;

    nimf_api    = NULL;
    glib_api    = NULL;
    gobject_api = NULL;
    gio_api     = NULL;

    if (libnimf)
    {
      dlclose (libnimf);
      libnimf = NULL;
    }

    if (libglib)
    {
      dlclose (libglib);
      libglib = NULL;
    }

    if (libgobject)
    {
      dlclose (libgobject);
      libgobject = NULL;
    }

    if (libgio)
    {
      dlclose (libgio);
      libgio = NULL;
    }
#endif
  }

  virtual QStringList keys () const
  {
#ifndef USE_DLFCN
    g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

    return QStringList () <<  "nimf";
  }

  virtual QPlatformInputContext *create (const QString     &key,
                                         const QStringList &paramList)
  {
#ifndef USE_DLFCN
    g_debug (G_STRLOC ": %s", G_STRFUNC);
#endif

    return new NimfInputContext ();
  }
};

#include "im-nimf-qt5.moc"
