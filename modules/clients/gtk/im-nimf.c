/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * im-nimf.c
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

#include "config.h"
#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>
#include <glib/gi18n.h>
#include <nimf.h>
#include <X11/XKBlib.h>
#if GTK_CHECK_VERSION (3, 6, 0)
  #include <gdk/gdkx.h>
#endif


#define NIMF_GTK_TYPE_IM_CONTEXT  (nimf_gtk_im_context_get_type ())
#define NIMF_GTK_IM_CONTEXT(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_GTK_TYPE_IM_CONTEXT, NimfGtkIMContext))

typedef struct _NimfGtkIMContext      NimfGtkIMContext;
typedef struct _NimfGtkIMContextClass NimfGtkIMContextClass;

struct _NimfGtkIMContext
{
  GtkIMContext  parent_instance;

  NimfIM       *im;
  GtkIMContext *simple;
  GdkWindow    *client_window;
  GSettings    *settings;
  gboolean      is_reset_on_gdk_button_press_event;
  gboolean      is_hook_gdk_event_key;
  gboolean      has_focus;
  gboolean      has_event_filter;
};

struct _NimfGtkIMContextClass
{
  GtkIMContextClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE (NimfGtkIMContext, nimf_gtk_im_context, GTK_TYPE_IM_CONTEXT);

static NimfEvent *
translate_gdk_event_key (GdkEventKey *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEvent *nimf_event = nimf_event_new (NIMF_EVENT_NOTHING);

  if (event->type == GDK_KEY_PRESS)
    nimf_event->key.type = NIMF_EVENT_KEY_PRESS;
  else
    nimf_event->key.type = NIMF_EVENT_KEY_RELEASE;

  nimf_event->key.state  = event->state;
  nimf_event->key.keyval = event->keyval;
  nimf_event->key.hardware_keycode = event->hardware_keycode;

  return nimf_event;
}

static NimfEvent *
translate_xkey_event (XEvent *xevent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GdkKeymap *keymap = gdk_keymap_get_for_display (gdk_display_get_default ());
  GdkModifierType consumed, state;

  NimfEvent *nimf_event = nimf_event_new (NIMF_EVENT_NOTHING);

  if (xevent->type == KeyPress)
    nimf_event->key.type = NIMF_EVENT_KEY_PRESS;
  else
    nimf_event->key.type = NIMF_EVENT_KEY_RELEASE;

  nimf_event->key.state = (NimfModifierType) xevent->xkey.state;

#if GTK_CHECK_VERSION (3, 6, 0)
  gint group = gdk_x11_keymap_get_group_for_state (keymap, xevent->xkey.state);
#else
  gint group = XkbGroupForCoreState (xevent->xkey.state);
#endif

  nimf_event->key.hardware_keycode = xevent->xkey.keycode;
  nimf_event->key.keyval = NIMF_KEY_VoidSymbol;

  gdk_keymap_translate_keyboard_state (keymap,
                                       nimf_event->key.hardware_keycode,
                                       nimf_event->key.state,
                                       group,
                                       &nimf_event->key.keyval,
                                       NULL, NULL, &consumed);

  state = nimf_event->key.state & ~consumed;
  nimf_event->key.state |= (NimfModifierType) state;

  return nimf_event;
}

static gboolean
nimf_gtk_im_context_filter_keypress (GtkIMContext *context,
                                     GdkEventKey  *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean   retval;
  NimfEvent *nimf_event;

  nimf_event = translate_gdk_event_key (event);
  retval = nimf_im_filter_event (NIMF_GTK_IM_CONTEXT (context)->im, nimf_event);
  nimf_event_free (nimf_event);

  if (retval == FALSE)
    return gtk_im_context_filter_keypress (NIMF_GTK_IM_CONTEXT (context)->simple, event);

  return retval;
}

static void
nimf_gtk_im_context_reset (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_im_reset (NIMF_GTK_IM_CONTEXT (context)->im);
  gtk_im_context_reset (NIMF_GTK_IM_CONTEXT (context)->simple);
}

static GdkFilterReturn
on_gdk_x_event (XEvent           *xevent,
                GdkEvent         *event,
                NimfGtkIMContext *context)
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
        NimfEvent *nimf_event = translate_xkey_event (xevent);
        retval = nimf_im_filter_event (context->im, nimf_event);
        nimf_event_free (nimf_event);
      }
      break;
    case ButtonPress:
      if (context->is_reset_on_gdk_button_press_event)
        nimf_im_reset (context->im);
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
nimf_gtk_im_context_set_client_window (GtkIMContext *context,
                                       GdkWindow    *window)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtkIMContext *a_context = NIMF_GTK_IM_CONTEXT (context);

  if (a_context->client_window)
  {
    g_object_unref (a_context->client_window);
    a_context->client_window = NULL;
  }

  if (window)
    a_context->client_window = g_object_ref (window);
}

static void
nimf_gtk_im_context_get_preedit_string (GtkIMContext   *context,
                                        gchar         **str,
                                        PangoAttrList **attrs,
                                        gint           *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditAttr **preedit_attrs;
  gchar *preedit_str;

  nimf_im_get_preedit_string (NIMF_GTK_IM_CONTEXT (context)->im,
                              &preedit_str, &preedit_attrs, cursor_pos);

  if (str)
    *str = preedit_str;

  if (attrs)
  {
    PangoAttribute *attr;
    gchar          *ptr1;
    gchar          *ptr2;
    gint            i;

    *attrs = pango_attr_list_new ();

    for (i = 0; preedit_attrs[i] != NULL; i++)
    {
      ptr1 = g_utf8_offset_to_pointer (preedit_str, preedit_attrs[i]->start_index);
      ptr2 = g_utf8_offset_to_pointer (preedit_str, preedit_attrs[i]->end_index);

      switch (preedit_attrs[i]->type)
      {
        case NIMF_PREEDIT_ATTR_UNDERLINE:
          attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
          break;
        case NIMF_PREEDIT_ATTR_HIGHLIGHT:
          attr = pango_attr_background_new (0, 0xffff, 0);
          attr->start_index = ptr1 - preedit_str;
          attr->end_index   = ptr2 - preedit_str;
          pango_attr_list_insert (*attrs, attr);

          attr = pango_attr_foreground_new (0, 0, 0);
          break;
        default:
          attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
          break;
      }

      attr->start_index = ptr1 - preedit_str;
      attr->end_index   = ptr2 - preedit_str;
      pango_attr_list_insert (*attrs, attr);
    }
  }

  nimf_preedit_attr_freev (preedit_attrs);
}

static void
nimf_gtk_im_context_focus_in (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtkIMContext *a_context = NIMF_GTK_IM_CONTEXT (context);
  a_context->has_focus = TRUE;
  nimf_im_focus_in (a_context->im);
}

static void
nimf_gtk_im_context_focus_out (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtkIMContext *a_context = NIMF_GTK_IM_CONTEXT (context);
  nimf_im_focus_out (a_context->im);
  a_context->has_focus = FALSE;
}

static void
nimf_gtk_im_context_set_cursor_location (GtkIMContext *context,
                                         GdkRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtkIMContext *nimf_context = NIMF_GTK_IM_CONTEXT (context);

  GdkRectangle root_area = *area;

  if (nimf_context->client_window)
    gdk_window_get_root_coords (nimf_context->client_window,
                                area->x,
                                area->y,
                                &root_area.x,
                                &root_area.y);

  nimf_im_set_cursor_location (NIMF_GTK_IM_CONTEXT (context)->im,
                               (const NimfRectangle *) &root_area);
}

static void
nimf_gtk_im_context_set_use_preedit (GtkIMContext *context,
                                     gboolean      use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_im_set_use_preedit (NIMF_GTK_IM_CONTEXT (context)->im, use_preedit);
}

static void
nimf_gtk_im_context_set_surrounding (GtkIMContext *context,
                                     const char   *text,
                                     gint          len,
                                     gint          cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_im_set_surrounding (NIMF_GTK_IM_CONTEXT (context)->im,
                           text, len, g_utf8_strlen (text, cursor_index));
}

GtkIMContext *
nimf_gtk_im_context_new (void)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (NIMF_GTK_TYPE_IM_CONTEXT, NULL);
}

static void
on_commit (NimfIM           *im,
           const gchar      *text,
           NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_signal_emit_by_name (context, "commit", text);
}

static gboolean
on_delete_surrounding (NimfIM           *im,
                       gint              offset,
                       gint              n_chars,
                       NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval;
  g_signal_emit_by_name (context,
                         "delete-surrounding", offset, n_chars, &retval);
  return retval;
}

static void
on_preedit_changed (NimfIM           *im,
                    NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-changed");
}

static void
on_preedit_end (NimfIM           *im,
                NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-end");
}

static void
on_preedit_start (NimfIM           *im,
                  NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-start");
}

static gboolean
on_retrieve_surrounding (NimfIM           *im,
                         NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval;
  g_signal_emit_by_name (context, "retrieve-surrounding", &retval);

  return retval;
}

static void
on_beep (NimfIM           *im,
         NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gdk_display_beep (gdk_display_get_default ());
}

static void
nimf_gtk_im_context_update_event_filter (NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (context->is_reset_on_gdk_button_press_event ||
      context->is_hook_gdk_event_key)
  {
    if (context->has_event_filter == FALSE)
    {
      context->has_event_filter = TRUE;
      gdk_window_add_filter (NULL, (GdkFilterFunc) on_gdk_x_event, context);
    }
  }
  else
  {
    if (context->has_event_filter == TRUE)
    {
      context->has_event_filter = FALSE;
      gdk_window_remove_filter (NULL, (GdkFilterFunc) on_gdk_x_event, context);
    }
  }
}

static void
on_changed_reset_on_gdk_button_press_event (GSettings        *settings,
                                            gchar            *key,
                                            NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->is_reset_on_gdk_button_press_event =
    g_settings_get_boolean (context->settings, key);

  nimf_gtk_im_context_update_event_filter (context);
}

static void
on_changed_hook_gdk_event_key (GSettings        *settings,
                               gchar            *key,
                               NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->is_hook_gdk_event_key =
    g_settings_get_boolean (context->settings, key);

  nimf_gtk_im_context_update_event_filter (context);
}

static void
nimf_gtk_im_context_init (NimfGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->im = nimf_im_new ();
  context->simple = gtk_im_context_simple_new ();

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
  g_signal_connect (context->im, "beep",
                    G_CALLBACK (on_beep), context);

  g_signal_connect (context->simple, "commit",
                    G_CALLBACK (on_commit), context);
  g_signal_connect (context->simple, "delete-surrounding",
                    G_CALLBACK (on_delete_surrounding), context);
  g_signal_connect (context->simple, "preedit-changed",
                    G_CALLBACK (on_preedit_changed), context);
  g_signal_connect (context->simple, "preedit-end",
                    G_CALLBACK (on_preedit_end), context);
  g_signal_connect (context->simple, "preedit-start",
                    G_CALLBACK (on_preedit_start), context);
  g_signal_connect (context->simple, "retrieve-surrounding",
                    G_CALLBACK (on_retrieve_surrounding), context);

  context->settings = g_settings_new ("org.nimf.clients.gtk");

  context->is_reset_on_gdk_button_press_event =
    g_settings_get_boolean (context->settings,
                            "reset-on-gdk-button-press-event");

  context->is_hook_gdk_event_key =
    g_settings_get_boolean (context->settings, "hook-gdk-event-key");

  nimf_gtk_im_context_update_event_filter (context);

  g_signal_connect (context->settings,
                    "changed::reset-on-gdk-button-press-event",
                    G_CALLBACK (on_changed_reset_on_gdk_button_press_event),
                    context);
  g_signal_connect (context->settings, "changed::hook-gdk-event-key",
                    G_CALLBACK (on_changed_hook_gdk_event_key), context);
}

static void
nimf_gtk_im_context_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtkIMContext *context = NIMF_GTK_IM_CONTEXT (object);

  if (context->has_event_filter)
    gdk_window_remove_filter (NULL, (GdkFilterFunc) on_gdk_x_event, context);

  g_object_unref (context->im);
  g_object_unref (context->simple);
  g_object_unref (context->settings);

  if (context->client_window)
    g_object_unref (context->client_window);

  G_OBJECT_CLASS (nimf_gtk_im_context_parent_class)->finalize (object);
}

static void
nimf_gtk_im_context_class_init (NimfGtkIMContextClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (class);

  im_context_class->set_client_window   = nimf_gtk_im_context_set_client_window;
  im_context_class->get_preedit_string  = nimf_gtk_im_context_get_preedit_string;
  im_context_class->filter_keypress     = nimf_gtk_im_context_filter_keypress;
  im_context_class->focus_in            = nimf_gtk_im_context_focus_in;
  im_context_class->focus_out           = nimf_gtk_im_context_focus_out;
  im_context_class->reset               = nimf_gtk_im_context_reset;
  im_context_class->set_cursor_location = nimf_gtk_im_context_set_cursor_location;
  im_context_class->set_use_preedit     = nimf_gtk_im_context_set_use_preedit;
  im_context_class->set_surrounding     = nimf_gtk_im_context_set_surrounding;

  object_class->finalize = nimf_gtk_im_context_finalize;
}

static void
nimf_gtk_im_context_class_finalize (NimfGtkIMContextClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static const GtkIMContextInfo nimf_info = {
  PACKAGE,          /* ID */
  N_("Nimf"),       /* Human readable name */
  GETTEXT_PACKAGE,  /* Translation domain */
  NIMF_LOCALE_DIR,  /* Directory for bindtextdomain */
  "ko:ja:zh"        /* Languages for which this module is the default */
};

static const GtkIMContextInfo *info_list[] = {
  &nimf_info
};

G_MODULE_EXPORT void im_module_init (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_gtk_im_context_register_type (type_module);
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
    return nimf_gtk_im_context_new ();
  else
    return NULL;
}
