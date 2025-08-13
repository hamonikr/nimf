/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * im-nimf-gtk4-new.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2019-2025 Kevin Kim at HamoniKR <chaeya@gmail.com>
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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <nimf.h>
#include <gio/gio.h>

#define NIMF_GTK4_TYPE_IM_CONTEXT  (nimf_gtk4_im_context_get_type ())
#define NIMF_GTK4_IM_CONTEXT(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_GTK4_TYPE_IM_CONTEXT, NimfGtk4IMContext))
#define NIMF_GTK4_IM_CONTEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_GTK4_TYPE_IM_CONTEXT, NimfGtk4IMContextClass))
#define NIMF_GTK4_IS_IM_CONTEXT(obj)  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_GTK4_TYPE_IM_CONTEXT))
#define NIMF_GTK4_IS_IM_CONTEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_GTK4_TYPE_IM_CONTEXT))

typedef struct _NimfGtk4IMContext      NimfGtk4IMContext;
typedef struct _NimfGtk4IMContextClass NimfGtk4IMContextClass;

struct _NimfGtk4IMContext
{
  GtkIMContext  parent_instance;

  NimfIM       *im;
  GtkIMContext *simple;
  GdkSurface   *client_surface;
  GSettings    *settings;
  GtkEventController *key_controller;
  gboolean      is_reset_on_button_press_event;
  gboolean      has_focus;
};

struct _NimfGtk4IMContextClass
{
  GtkIMContextClass parent_class;
};

/* Static declarations */
static GType nimf_gtk4_im_context_get_type (void);
static void nimf_gtk4_im_context_class_init (NimfGtk4IMContextClass *klass);
static void nimf_gtk4_im_context_init (NimfGtk4IMContext *context);
static void nimf_gtk4_im_context_finalize (GObject *object);

/* IM Context method implementations */
static void nimf_gtk4_im_context_set_client_widget (GtkIMContext *context, GtkWidget *widget);
static void nimf_gtk4_im_context_get_preedit_string (GtkIMContext *context, gchar **str, PangoAttrList **attrs, gint *cursor_pos);
static gboolean nimf_gtk4_im_context_filter_keypress (GtkIMContext *context, GdkEvent *event);
static void nimf_gtk4_im_context_focus_in (GtkIMContext *context);
static void nimf_gtk4_im_context_focus_out (GtkIMContext *context);
static void nimf_gtk4_im_context_reset (GtkIMContext *context);
static void nimf_gtk4_im_context_set_cursor_location (GtkIMContext *context, GdkRectangle *area);
static void nimf_gtk4_im_context_set_use_preedit (GtkIMContext *context, gboolean use_preedit);
static void nimf_gtk4_im_context_set_surrounding (GtkIMContext *context, const gchar *text, gint len, gint cursor_index);
static gboolean nimf_gtk4_im_context_get_surrounding (GtkIMContext *context, gchar **text, gint *cursor_index);

G_DEFINE_TYPE (NimfGtk4IMContext, nimf_gtk4_im_context, GTK_TYPE_IM_CONTEXT)

/* Signal handlers */
static void
on_commit (NimfIM *im, const gchar *text, NimfGtk4IMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "commit", text);
}

static void
on_preedit_start (NimfIM *im, NimfGtk4IMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-start");
}

static void
on_preedit_end (NimfIM *im, NimfGtk4IMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-end");
}

static void
on_preedit_changed (NimfIM *im, NimfGtk4IMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-changed");
}

static void
on_retrieve_surrounding (NimfIM *im, NimfGtk4IMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  gboolean retval;
  g_signal_emit_by_name (context, "retrieve-surrounding", &retval);
}

static void
on_delete_surrounding (NimfIM *im, gint offset, gint n_chars, NimfGtk4IMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  gboolean retval;
  g_signal_emit_by_name (context, "delete-surrounding", offset, n_chars, &retval);
}

static void
on_beep (NimfIM *im, NimfGtk4IMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  gdk_display_beep (gdk_display_get_default ());
}

/* Key event handling */
static NimfEvent *
translate_gdk_event_key (GdkEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEvent *nimf_event = nimf_event_new (NIMF_EVENT_NOTHING);

  if (gdk_event_get_event_type (event) == GDK_KEY_PRESS)
    nimf_event->key.type = NIMF_EVENT_KEY_PRESS;
  else
    nimf_event->key.type = NIMF_EVENT_KEY_RELEASE;

  nimf_event->key.state = gdk_event_get_modifier_state (event);
  nimf_event->key.keyval = gdk_key_event_get_keyval (event);
  nimf_event->key.hardware_keycode = gdk_key_event_get_keycode (event);

  return nimf_event;
}

static gboolean
on_key_pressed (GtkEventControllerKey *controller,
                guint                  keyval,
                guint                  keycode,
                GdkModifierType        state,
                NimfGtk4IMContext     *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_debug ("GTK4 KEY PRESSED: keyval=%u, keycode=%u, state=%u", keyval, keycode, state);

  if (!context->im) {
    g_debug ("GTK4 KEY: context->im is NULL, returning FALSE");
    return FALSE;
  }

  GdkEvent *event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));
  if (!event) {
    g_debug ("GTK4 KEY: event is NULL, returning FALSE");
    return FALSE;
  }

  NimfEvent *nimf_event = translate_gdk_event_key (event);
  gboolean retval = nimf_im_filter_event (context->im, nimf_event);
  g_debug ("GTK4 KEY: nimf_im_filter_event returned %s", retval ? "TRUE" : "FALSE");
  if (!retval && context->simple) {
    retval = gtk_im_context_filter_keypress (context->simple, event);
    g_debug ("GTK4 KEY: fallback simple->filter_keypress returned %s", retval ? "TRUE" : "FALSE");
  }
  nimf_event_free (nimf_event);

  return retval;
}

static gboolean
on_key_released (GtkEventControllerKey *controller,
                 guint                  keyval,
                 guint                  keycode,
                 GdkModifierType        state,
                 NimfGtk4IMContext     *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!context->im)
    return FALSE;

  GdkEvent *event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));
  if (!event)
    return FALSE;

  NimfEvent *nimf_event = translate_gdk_event_key (event);
  gboolean retval = nimf_im_filter_event (context->im, nimf_event);
  nimf_event_free (nimf_event);

  return retval;
}

/* IM Context method implementations */
static void
nimf_gtk4_im_context_set_client_widget (GtkIMContext *context, GtkWidget *widget)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *nimf_context = NIMF_GTK4_IM_CONTEXT (context);

  /* Clean up previous resources */
  if (nimf_context->client_surface)
  {
    g_object_unref (nimf_context->client_surface);
    nimf_context->client_surface = NULL;
  }

  if (nimf_context->key_controller)
  {
    GtkWidget *old_widget = gtk_event_controller_get_widget (nimf_context->key_controller);
    if (old_widget)
      gtk_widget_remove_controller (old_widget, nimf_context->key_controller);
    nimf_context->key_controller = NULL;
  }

  if (widget)
  {
    GtkNative *native = gtk_widget_get_native (widget);
    if (native)
    {
      GdkSurface *surface = gtk_native_get_surface (native);
      if (surface)
      {
        nimf_context->client_surface = g_object_ref (surface);
        
        /* Create key event controller */
        nimf_context->key_controller = gtk_event_controller_key_new ();
        /* Ensure IM gets key events before app-level handlers that may capture Enter */
        gtk_event_controller_set_propagation_phase (nimf_context->key_controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller (widget, nimf_context->key_controller);
        
        g_signal_connect (nimf_context->key_controller, "key-pressed",
                         G_CALLBACK (on_key_pressed), nimf_context);
        g_signal_connect (nimf_context->key_controller, "key-released",
                         G_CALLBACK (on_key_released), nimf_context);

        /* Propagate client widget to simple context for fallback path */
        if (nimf_context->simple)
          gtk_im_context_set_client_widget (nimf_context->simple, widget);
      }
    }
  }

  /* Chain up to parent */
  if (GTK_IM_CONTEXT_CLASS (nimf_gtk4_im_context_parent_class)->set_client_widget)
    GTK_IM_CONTEXT_CLASS (nimf_gtk4_im_context_parent_class)->set_client_widget (context, widget);
}

static void
nimf_gtk4_im_context_get_preedit_string (GtkIMContext   *context,
                                         gchar         **str,
                                         PangoAttrList **attrs,
                                         gint           *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *nimf_context = NIMF_GTK4_IM_CONTEXT (context);

  if (!nimf_context->im)
  {
    if (str) *str = g_strdup ("");
    if (attrs) *attrs = pango_attr_list_new ();
    if (cursor_pos) *cursor_pos = 0;
    return;
  }

  NimfPreeditAttr **nimf_attrs = NULL;
  nimf_im_get_preedit_string (nimf_context->im, str, &nimf_attrs, cursor_pos);

  if (attrs)
  {
    *attrs = pango_attr_list_new ();
    if (nimf_attrs)
    {
      for (int i = 0; nimf_attrs[i]; i++)
      {
        PangoAttribute *attr = NULL;
        
        switch (nimf_attrs[i]->type)
        {
          case NIMF_PREEDIT_ATTR_UNDERLINE:
            attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
            break;
          case NIMF_PREEDIT_ATTR_HIGHLIGHT:
            attr = pango_attr_background_new (0, 0, 65535);
            break;
          default:
            break;
        }
        
        if (attr)
        {
          attr->start_index = nimf_attrs[i]->start_index;
          attr->end_index = nimf_attrs[i]->end_index;
          pango_attr_list_insert (*attrs, attr);
        }
      }
    }
  }

  nimf_preedit_attr_freev (nimf_attrs);
}

static gboolean
nimf_gtk4_im_context_filter_keypress (GtkIMContext *context, GdkEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *nimf_context = NIMF_GTK4_IM_CONTEXT (context);
  
  g_debug ("GTK4 IM: keyval=%u, state=%u", gdk_key_event_get_keyval (event), gdk_event_get_modifier_state (event));

  if (!nimf_context->im || !event) {
    g_debug ("GTK4 IM: nimf_context->im is NULL or event is NULL, returning FALSE");
    return FALSE;
  }

  NimfEvent *nimf_event = translate_gdk_event_key (event);
  if (!nimf_event) {
    g_debug ("GTK4 IM: translate_gdk_event_key returned NULL, returning FALSE");
    return FALSE;
  }

  g_debug ("GTK4 IM: calling nimf_im_filter_event");
  gboolean retval = nimf_im_filter_event (nimf_context->im, nimf_event);
  g_debug ("GTK4 IM: nimf_im_filter_event returned %s", retval ? "TRUE" : "FALSE");
  
  /* System Keyboard 엔진이 활성화된 상태에서 nimf가 FALSE를 반환하는 경우
   * 영어 문자를 직접 처리하여 입력되도록 함 */
  if (!retval && nimf_event->key.type == NIMF_EVENT_KEY_PRESS) {
    g_debug ("GTK4 IM: Checking for direct character input");
    guint keyval = gdk_key_event_get_keyval (event);
    GdkModifierType state = gdk_event_get_modifier_state (event);
    
    g_debug ("GTK4 IM: keyval=0x%x, state=0x%x", keyval, state);
    
    /* 일반적인 영어 문자 (a-z, A-Z, 0-9, 특수문자 등) 처리 */
    if (keyval >= 0x20 && keyval <= 0x7E && !(state & (GDK_CONTROL_MASK | GDK_ALT_MASK))) {
      g_debug ("GTK4 IM: Processing direct character input for keyval=0x%x", keyval);
      gchar text[7];
      gint len;
      gunichar uc = gdk_keyval_to_unicode (keyval);
      
      g_debug ("GTK4 IM: Unicode character: U+%04X", uc);
      
      if (uc != 0 && g_unichar_validate (uc)) {
        len = g_unichar_to_utf8 (uc, text);
        text[len] = ' ';
        
        g_debug ("GTK4 IM: Directly committing text: '%s'", text);
        g_signal_emit_by_name (nimf_context, "commit", text);
        retval = TRUE;
      } else {
        g_debug ("GTK4 IM: Invalid unicode character, skipping");
      }
    } else {
      g_debug ("GTK4 IM: Keyval 0x%x out of range or has modifiers, skipping", keyval);
    }
  }
  
  nimf_event_free (nimf_event);

  return retval;
}

static void
nimf_gtk4_im_context_focus_in (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *nimf_context = NIMF_GTK4_IM_CONTEXT (context);

  nimf_context->has_focus = TRUE;
  if (nimf_context->im)
    nimf_im_focus_in (nimf_context->im);
  if (nimf_context->simple)
    gtk_im_context_focus_in (nimf_context->simple);
}

static void
nimf_gtk4_im_context_focus_out (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *nimf_context = NIMF_GTK4_IM_CONTEXT (context);

  nimf_context->has_focus = FALSE;
  if (nimf_context->im)
    nimf_im_focus_out (nimf_context->im);
  if (nimf_context->simple)
    gtk_im_context_focus_out (nimf_context->simple);
}

static void
nimf_gtk4_im_context_reset (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *nimf_context = NIMF_GTK4_IM_CONTEXT (context);

  if (nimf_context->im)
    nimf_im_reset (nimf_context->im);
  if (nimf_context->simple)
    gtk_im_context_reset (nimf_context->simple);
}

static void
nimf_gtk4_im_context_set_cursor_location (GtkIMContext *context, GdkRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *nimf_context = NIMF_GTK4_IM_CONTEXT (context);

  if (nimf_context->im && area)
  {
    NimfRectangle nimf_area = {area->x, area->y, area->width, area->height};
    nimf_im_set_cursor_location (nimf_context->im, &nimf_area);
  }

  if (nimf_context->simple && area)
    gtk_im_context_set_cursor_location (nimf_context->simple, area);
}

static void
nimf_gtk4_im_context_set_use_preedit (GtkIMContext *context, gboolean use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *nimf_context = NIMF_GTK4_IM_CONTEXT (context);

  if (nimf_context->im)
    nimf_im_set_use_preedit (nimf_context->im, use_preedit);
  if (nimf_context->simple)
    gtk_im_context_set_use_preedit (nimf_context->simple, use_preedit);
}

static void
nimf_gtk4_im_context_set_surrounding (GtkIMContext *context,
                                      const gchar  *text,
                                      gint          len,
                                      gint          cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *nimf_context = NIMF_GTK4_IM_CONTEXT (context);

  if (nimf_context->im)
    nimf_im_set_surrounding (nimf_context->im, text, len, cursor_index);
  if (nimf_context->simple)
    gtk_im_context_set_surrounding_with_selection (nimf_context->simple, text, len, cursor_index, cursor_index);
}

static gboolean
nimf_gtk4_im_context_get_surrounding (GtkIMContext *context,
                                      gchar       **text,
                                      gint         *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *nimf_context = NIMF_GTK4_IM_CONTEXT (context);

  if (nimf_context->simple)
    return gtk_im_context_get_surrounding_with_selection (nimf_context->simple, text, cursor_index, NULL);

  return FALSE;
}

/* Settings callback */
static void
on_reset_on_button_press_event_changed (GSettings *settings,
                                        gchar     *key,
                                        NimfGtk4IMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  context->is_reset_on_button_press_event = g_settings_get_boolean (settings, key);
}

/* Object lifecycle */
static void
nimf_gtk4_im_context_init (NimfGtk4IMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  /* Initialize all pointers to NULL */
  context->im = NULL;
  context->simple = NULL;
  context->client_surface = NULL;
  context->key_controller = NULL;
  context->settings = NULL;
  context->is_reset_on_button_press_event = FALSE;
  context->has_focus = FALSE;

  /* Create Nimf IM instance */
  context->im = nimf_im_new ();
  if (!context->im)
  {
    g_warning ("Failed to create Nimf IM instance");
    return;
  }

  /* Create simple fallback context */
  context->simple = gtk_im_context_simple_new ();
  if (!context->simple)
  {
    g_warning ("Failed to create GTK simple IM context");
    g_clear_object (&context->im);
    return;
  }

  /* Connect Nimf IM signals */
  g_signal_connect (context->im, "commit", G_CALLBACK (on_commit), context);
  g_signal_connect (context->im, "preedit-start", G_CALLBACK (on_preedit_start), context);
  g_signal_connect (context->im, "preedit-end", G_CALLBACK (on_preedit_end), context);
  g_signal_connect (context->im, "preedit-changed", G_CALLBACK (on_preedit_changed), context);
  g_signal_connect (context->im, "retrieve-surrounding", G_CALLBACK (on_retrieve_surrounding), context);
  g_signal_connect (context->im, "delete-surrounding", G_CALLBACK (on_delete_surrounding), context);
  g_signal_connect (context->im, "beep", G_CALLBACK (on_beep), context);

  /* Connect GTK simple IM signals for fallback path */
  g_signal_connect (context->simple, "commit", G_CALLBACK (on_commit), context);
  g_signal_connect (context->simple, "preedit-start", G_CALLBACK (on_preedit_start), context);
  g_signal_connect (context->simple, "preedit-end", G_CALLBACK (on_preedit_end), context);
  g_signal_connect (context->simple, "preedit-changed", G_CALLBACK (on_preedit_changed), context);
  g_signal_connect (context->simple, "retrieve-surrounding", G_CALLBACK (on_retrieve_surrounding), context);
  g_signal_connect (context->simple, "delete-surrounding", G_CALLBACK (on_delete_surrounding), context);

  /* Setup settings */
  context->settings = g_settings_new ("org.nimf.clients.gtk");
  context->is_reset_on_button_press_event = 
    g_settings_get_boolean (context->settings, "reset-on-gdk-button-press-event");

  g_signal_connect (context->settings, "changed::reset-on-gdk-button-press-event",
                   G_CALLBACK (on_reset_on_button_press_event_changed), context);
}

static void
nimf_gtk4_im_context_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfGtk4IMContext *context = NIMF_GTK4_IM_CONTEXT (object);

  /* Clean up key controller */
  if (context->key_controller)
  {
    GtkWidget *widget = gtk_event_controller_get_widget (context->key_controller);
    if (widget)
      gtk_widget_remove_controller (widget, context->key_controller);
    context->key_controller = NULL;
  }

  /* Clean up resources */
  g_clear_object (&context->client_surface);
  g_clear_object (&context->im);
  g_clear_object (&context->simple);
  g_clear_object (&context->settings);

  G_OBJECT_CLASS (nimf_gtk4_im_context_parent_class)->finalize (object);
}

static void
nimf_gtk4_im_context_class_init (NimfGtk4IMContextClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (klass);

  /* Object methods */
  object_class->finalize = nimf_gtk4_im_context_finalize;

  /* IM Context methods */
  im_context_class->set_client_widget   = nimf_gtk4_im_context_set_client_widget;
  im_context_class->get_preedit_string  = nimf_gtk4_im_context_get_preedit_string;
  im_context_class->filter_keypress     = nimf_gtk4_im_context_filter_keypress;
  im_context_class->focus_in            = nimf_gtk4_im_context_focus_in;
  im_context_class->focus_out           = nimf_gtk4_im_context_focus_out;
  im_context_class->reset               = nimf_gtk4_im_context_reset;
  im_context_class->set_cursor_location = nimf_gtk4_im_context_set_cursor_location;
  im_context_class->set_use_preedit     = nimf_gtk4_im_context_set_use_preedit;
  im_context_class->set_surrounding     = nimf_gtk4_im_context_set_surrounding;
  im_context_class->get_surrounding     = nimf_gtk4_im_context_get_surrounding;
}

/* Module entry points */
G_MODULE_EXPORT void
g_io_module_load (GIOModule *module)
{
  g_debug (G_STRLOC ": %s - Loading nimf GTK4 IM module", G_STRFUNC);

  /* Register our type */
  g_type_module_use (G_TYPE_MODULE (module));

  /* Implement GTK IM extension point */
  g_io_extension_point_implement ("gtk-im-module",
                                   NIMF_GTK4_TYPE_IM_CONTEXT,
                                   "nimf",
                                   10);

  g_debug ("nimf GTK4 IM module loaded successfully");
}

G_MODULE_EXPORT void
g_io_module_unload (GIOModule *module)
{
  g_debug (G_STRLOC ": %s - Unloading nimf GTK4 IM module", G_STRFUNC);
  g_type_module_unuse (G_TYPE_MODULE (module));
}

G_MODULE_EXPORT char **
g_io_module_query (void)
{
  g_debug (G_STRLOC ": %s - Querying nimf GTK4 IM module", G_STRFUNC);
  
  char *eps[] = {
    "gtk-im-module",
    NULL
  };

  return g_strdupv (eps);
}