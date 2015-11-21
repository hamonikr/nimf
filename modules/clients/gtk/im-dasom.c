/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * im-dasom.c
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

#include "config.h"
#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>
#include <glib/gi18n.h>
#include "dasom.h"
#include <string.h>
#include <gdk/gdkkeysyms.h>

#define DASOM_TYPE_GTK_IM_CONTEXT  (dasom_gtk_im_context_get_type ())
#define DASOM_GTK_IM_CONTEXT(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_GTK_IM_CONTEXT, DasomGtkIMContext))

typedef struct _DasomGtkIMContext      DasomGtkIMContext;
typedef struct _DasomGtkIMContextClass DasomGtkIMContextClass;

struct _DasomGtkIMContext
{
  GtkIMContext  parent_instance;
  DasomIM      *im;
  GdkWindow    *client_window;
  GdkRectangle  cursor_area;
  GSettings    *settings;
  gboolean      is_reset_on_gdk_button_press_event;
  gboolean      is_hook_gdk_event_key;
  gboolean      has_focus;
};

struct _DasomGtkIMContextClass
{
  GtkIMContextClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE (DasomGtkIMContext, dasom_gtk_im_context, GTK_TYPE_IM_CONTEXT);

static DasomEvent *
translate_gdk_event_key (GdkEventKey *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomEventType type;

  if (event->type == GDK_KEY_PRESS)
    type = DASOM_EVENT_KEY_PRESS;
  else
    type = DASOM_EVENT_KEY_RELEASE;

  DasomEvent *dasom_event = dasom_event_new (type);
  dasom_event->key.state = event->state;
  dasom_event->key.keyval = event->keyval;
  dasom_event->key.hardware_keycode = event->hardware_keycode;

  return dasom_event;
}

static DasomEvent *
translate_xkey_event (XEvent *xevent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomEventType type = DASOM_EVENT_NOTHING;

  if (xevent->type == KeyPress)
    type = DASOM_EVENT_KEY_PRESS;
  else
    type = DASOM_EVENT_KEY_RELEASE;

  DasomEvent *dasom_event = dasom_event_new (type);
  dasom_event->key.state  = xevent->xkey.state;
  dasom_event->key.keyval = XLookupKeysym (&xevent->xkey,
                              (!(xevent->xkey.state & ShiftMask) !=
                               !(xevent->xkey.state & LockMask)) ? 1 : 0);
  dasom_event->key.hardware_keycode = xevent->xkey.keycode;

  return dasom_event;
}

static gboolean
dasom_gtk_im_context_filter_keypress (GtkIMContext *context,
                                      GdkEventKey  *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;
  DasomEvent *dasom_event = translate_gdk_event_key (event);

  retval = dasom_im_filter_event (DASOM_GTK_IM_CONTEXT (context)->im,
                                  dasom_event);
  dasom_event_free (dasom_event);

  return retval;
}

static void
dasom_gtk_im_context_reset (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_im_reset (DASOM_GTK_IM_CONTEXT (context)->im);
}

static GdkFilterReturn
on_gdk_x_event (XEvent            *xevent,
                GdkEvent          *event,
                DasomGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s: %p, %" G_GINT64_FORMAT, G_STRFUNC, context,
           g_get_real_time ());

  gboolean retval = FALSE;

  if (context->has_focus == FALSE || context->client_window == NULL)
    return GDK_FILTER_CONTINUE;

  switch (xevent->type)
  {
    case KeyPress:
    case KeyRelease:
      if (context->is_hook_gdk_event_key)
      {
        DasomEvent *d_event = translate_xkey_event (xevent);
        retval = dasom_im_filter_event (context->im, d_event);
        dasom_event_free (d_event);
      }
      break;
    case ButtonPress:
      if (context->is_reset_on_gdk_button_press_event)
        dasom_im_reset (context->im);
      break;
    default:
      break;
  }

  if (retval == FALSE)
    return GDK_FILTER_CONTINUE;
  else
    return GDK_FILTER_REMOVE;
}

static void
dasom_gtk_im_context_set_client_window (GtkIMContext *context,
                                        GdkWindow    *window)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomGtkIMContext *ds_context = DASOM_GTK_IM_CONTEXT (context);

  if (ds_context->client_window)
  {
    g_object_unref (ds_context->client_window);
    ds_context->client_window = NULL;
  }

  if (window)
    ds_context->client_window = g_object_ref (window);
}

static void
dasom_gtk_im_context_get_preedit_string (GtkIMContext   *context,
                                         gchar         **str,
                                         PangoAttrList **attrs,
                                         gint           *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  PangoAttribute *attr;

  dasom_im_get_preedit_string (DASOM_GTK_IM_CONTEXT (context)->im,
                               str,
                               cursor_pos);

  if (attrs)
  {
    *attrs = pango_attr_list_new ();

    attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);

    if (str)
    {
      attr->start_index = 0;
      attr->end_index   = strlen (*str);
    }

    pango_attr_list_change (*attrs, attr);
  }
}

static void
dasom_gtk_im_context_focus_in (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomGtkIMContext *ds_context = DASOM_GTK_IM_CONTEXT (context);
  ds_context->has_focus = TRUE;
  dasom_im_focus_in (ds_context->im);
}

static void
dasom_gtk_im_context_focus_out (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomGtkIMContext *ds_context = DASOM_GTK_IM_CONTEXT (context);
  dasom_im_focus_out (ds_context->im);
  ds_context->has_focus = FALSE;
}

static void
dasom_gtk_im_context_set_cursor_location (GtkIMContext *context,
                                          GdkRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomGtkIMContext *dasom_context = DASOM_GTK_IM_CONTEXT (context);

  if (memcmp (&dasom_context->cursor_area, area, sizeof (GdkRectangle)) == 0)
    return;

  dasom_context->cursor_area = *area;
  GdkRectangle root_area = *area;

  if (dasom_context->client_window)
  {
    gdk_window_get_root_coords (dasom_context->client_window,
                                area->x,
                                area->y,
                                &root_area.x,
                                &root_area.y);

    dasom_im_set_cursor_location (DASOM_GTK_IM_CONTEXT (context)->im,
                                  (const DasomRectangle *) &root_area);
  }
}

static void
dasom_gtk_im_context_set_use_preedit (GtkIMContext *context,
                                      gboolean      use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_im_set_use_preedit (DASOM_GTK_IM_CONTEXT (context)->im, use_preedit);
}

static gboolean
dasom_gtk_im_context_get_surrounding (GtkIMContext  *context,
                                      gchar        **text,
                                      gint          *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return dasom_im_get_surrounding (DASOM_GTK_IM_CONTEXT (context)->im,
                                   text,
                                   cursor_index);
}

static void
dasom_gtk_im_context_set_surrounding (GtkIMContext *context,
                                      const char   *text,
                                      gint          len,
                                      gint          cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_im_set_surrounding (DASOM_GTK_IM_CONTEXT (context)->im,
                            text,
                            len,
                            cursor_index);
}

GtkIMContext *
dasom_gtk_im_context_new (void)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return (GtkIMContext *) g_object_new (DASOM_TYPE_GTK_IM_CONTEXT, NULL);
}

static void
on_commit (DasomIM           *im,
           const gchar       *text,
           DasomGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_signal_emit_by_name (context, "commit", text);
}

static gboolean
on_delete_surrounding (DasomIM           *im,
                       gint               offset,
                       gint               n_chars,
                       DasomGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval;
  g_signal_emit_by_name (context,
                         "delete-surrounding", offset, n_chars, &retval);
  return retval;
}

static void
on_preedit_changed (DasomIM           *im,
                    DasomGtkIMContext *context)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-changed");
}

static void
on_preedit_end (DasomIM           *im,
                DasomGtkIMContext *context)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-end");
}

static void
on_preedit_start (DasomIM           *im,
                  DasomGtkIMContext *context)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-start");
}

static gboolean
on_retrieve_surrounding (DasomIM           *im,
                         DasomGtkIMContext *context)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval;
  g_signal_emit_by_name (context, "retrieve-surrounding", &retval);

  return retval;
}

static void
on_changed_reset_on_gdk_button_press_event (GSettings         *settings,
                                            gchar             *key,
                                            DasomGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->is_reset_on_gdk_button_press_event =
    g_settings_get_boolean (context->settings, key);
}

static void
on_changed_hook_gdk_event_key (GSettings         *settings,
                               gchar             *key,
                               DasomGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->is_hook_gdk_event_key = g_settings_get_boolean (context->settings,
                                                           key);
}

static void
dasom_gtk_im_context_init (DasomGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->im = dasom_im_new ();

  g_signal_connect (context->im, "commit",
                    G_CALLBACK (on_commit), context);
  g_signal_connect (context->im, "delete-surrounding",
                    G_CALLBACK (on_delete_surrounding), context);
  g_signal_connect (context->im, "preedit-changed",
                    G_CALLBACK (on_preedit_changed), context);
  g_signal_connect (context->im, "preedit-end",
                    G_CALLBACK (on_preedit_end), context);
  g_signal_connect (context->im, "preedit-start",
                    G_CALLBACK (on_preedit_start), context);
  g_signal_connect (context->im, "retrieve-surrounding",
                    G_CALLBACK (on_retrieve_surrounding), context);

  context->settings = g_settings_new ("org.freedesktop.Dasom.clients.gtk");

  context->is_reset_on_gdk_button_press_event =
    g_settings_get_boolean (context->settings,
                            "reset-on-gdk-button-press-event");

  context->is_hook_gdk_event_key =
    g_settings_get_boolean (context->settings, "hook-gdk-event-key");

  g_signal_connect (context->settings,
                    "changed::reset-on-gdk-button-press-event",
                    G_CALLBACK (on_changed_reset_on_gdk_button_press_event),
                    context);
  g_signal_connect (context->settings, "changed::hook-gdk-event-key",
                    G_CALLBACK (on_changed_hook_gdk_event_key), context);

  gdk_window_add_filter (NULL, (GdkFilterFunc) on_gdk_x_event, context);;
}

static void
dasom_gtk_im_context_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomGtkIMContext *context = DASOM_GTK_IM_CONTEXT (object);

  gdk_window_remove_filter (NULL, (GdkFilterFunc) on_gdk_x_event, context);

  g_object_unref (context->im);
  g_object_unref (context->settings);

  if (context->client_window)
    g_object_unref (context->client_window);

  G_OBJECT_CLASS (dasom_gtk_im_context_parent_class)->finalize (object);
}

static void
dasom_gtk_im_context_class_init (DasomGtkIMContextClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (klass);

  im_context_class->set_client_window   = dasom_gtk_im_context_set_client_window;
  im_context_class->get_preedit_string  = dasom_gtk_im_context_get_preedit_string;
  im_context_class->filter_keypress     = dasom_gtk_im_context_filter_keypress;
  im_context_class->focus_in            = dasom_gtk_im_context_focus_in;
  im_context_class->focus_out           = dasom_gtk_im_context_focus_out;
  im_context_class->reset               = dasom_gtk_im_context_reset;
  im_context_class->set_cursor_location = dasom_gtk_im_context_set_cursor_location;
  im_context_class->set_use_preedit     = dasom_gtk_im_context_set_use_preedit;
  im_context_class->set_surrounding     = dasom_gtk_im_context_set_surrounding;
  im_context_class->get_surrounding     = dasom_gtk_im_context_get_surrounding;

  object_class->finalize = dasom_gtk_im_context_finalize;
}

static void
dasom_gtk_im_context_class_finalize (DasomGtkIMContextClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static const GtkIMContextInfo dasom_info = {
  PACKAGE,          /* ID */
  N_("Dasom"),      /* Human readable name */
  GETTEXT_PACKAGE,  /* Translation domain */
  DASOM_LOCALE_DIR, /* Directory for bindtextdomain */
  "ko:ja:zh"        /* Languages for which this module is the default */
};

static const GtkIMContextInfo *info_list[] = {
  &dasom_info
};

G_MODULE_EXPORT void im_module_init (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_gtk_im_context_register_type (type_module);
}

G_MODULE_EXPORT void im_module_exit (void)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

G_MODULE_EXPORT void im_module_list (const GtkIMContextInfo ***contexts,
                                     int                      *n_contexts)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  *contexts = info_list;
  *n_contexts = G_N_ELEMENTS (info_list);
}

G_MODULE_EXPORT GtkIMContext *im_module_create (const gchar *context_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (g_strcmp0 (context_id, PACKAGE) == 0)
    return dasom_gtk_im_context_new ();
  else
    return NULL;
}
