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

#define DASOM_TYPE_GTK_IM_CONTEXT  (dasom_gtk_im_context_get_type ())
#define DASOM_GTK_IM_CONTEXT(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_GTK_IM_CONTEXT, DasomGtkIMContext))

typedef struct _DasomGtkIMContext      DasomGtkIMContext;
typedef struct _DasomGtkIMContextClass DasomGtkIMContextClass;

struct _DasomGtkIMContext
{
  GtkIMContext  parent_instance;
  DasomIM      *im;
  GdkWindow    *client_window;
};

struct _DasomGtkIMContextClass
{
  GtkIMContextClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE (DasomGtkIMContext, dasom_gtk_im_context, GTK_TYPE_IM_CONTEXT);

static void
dasom_gtk_im_context_set_client_window (GtkIMContext *context,
                                        GdkWindow    *window)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomGtkIMContext *dasom_context = DASOM_GTK_IM_CONTEXT (context);

  if (dasom_context->client_window)
  {
    g_object_unref (dasom_context->client_window);
    dasom_context->client_window = NULL;
  }

  if (window)
    dasom_context->client_window = g_object_ref (window);
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

static DasomEvent *
translate_gdk_event (GdkEventKey *event)
{
  DasomEventType type = DASOM_EVENT_NOTHING;

  switch (event->type)
  {
    case GDK_KEY_PRESS:
      type = DASOM_EVENT_KEY_PRESS;
      break;
    case GDK_KEY_RELEASE:
      type = DASOM_EVENT_KEY_RELEASE;
      break;
    default :
      g_error ("unknown event type");
      break;
  }

  DasomEvent *dasom_event = dasom_event_new (type);

  dasom_event->key.state = event->state;
  dasom_event->key.keyval = event->keyval;
  dasom_event->key.hardware_keycode = event->hardware_keycode;

  return dasom_event;
}

static gboolean
dasom_gtk_im_context_filter_keypress (GtkIMContext *context,
                                      GdkEventKey  *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;
  DasomEvent *dasom_event = translate_gdk_event (event);

  retval = dasom_im_filter_event (DASOM_GTK_IM_CONTEXT (context)->im,
                                  dasom_event);

  /* Do not free GdkEventKey */
  /* FIXME: whether to free dasom event or not */
  return retval;
}

static void
dasom_gtk_im_context_focus_in (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_im_focus_in (DASOM_GTK_IM_CONTEXT (context)->im);
}

static void
dasom_gtk_im_context_focus_out (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_im_focus_out (DASOM_GTK_IM_CONTEXT (context)->im);
}

static void
dasom_gtk_im_context_reset (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_im_reset (DASOM_GTK_IM_CONTEXT (context)->im);
}

static void
dasom_gtk_im_context_set_cursor_location (GtkIMContext *context,
                                          GdkRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  dasom_im_set_cursor_location (DASOM_GTK_IM_CONTEXT (context)->im,
                                (DasomRectangle *) area);
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
  g_debug (G_STRLOC ": %s: START", G_STRFUNC);
  g_signal_emit_by_name (context, "commit", text);
  g_debug (G_STRLOC ": %s: END", G_STRFUNC);
}

static void
on_delete_surrounding (DasomIM           *im,
                       gint               offset,
                       gint               n_chars,
                       DasomGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "delete-surrounding", offset, n_chars);
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

static void
on_retrieve_surrounding (DasomIM           *im,
                         DasomGtkIMContext *context)

{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "retrieve-surrounding");
}

static void
dasom_gtk_im_context_init (DasomGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->im = dasom_im_new ();

  g_signal_connect (context->im,
                    "commit",
                    G_CALLBACK (on_commit),
                    context);
  g_signal_connect (context->im,
                    "delete-surrounding",
                    G_CALLBACK (on_delete_surrounding),
                    context);
  g_signal_connect (context->im,
                    "preedit-changed",
                    G_CALLBACK (on_preedit_changed),
                    context);
  g_signal_connect (context->im,
                    "preedit-end",
                    G_CALLBACK (on_preedit_end),
                    context);
  g_signal_connect (context->im,
                    "preedit-start",
                    G_CALLBACK (on_preedit_start),
                    context);
  g_signal_connect (context->im,
                    "retrieve-surrounding",
                    G_CALLBACK (on_retrieve_surrounding),
                    context);
}

static void
dasom_gtk_im_context_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomGtkIMContext *context = DASOM_GTK_IM_CONTEXT (object);
  g_object_unref (context->im);

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
