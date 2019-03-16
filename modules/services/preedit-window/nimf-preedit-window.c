/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-preedit-window.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2017-2019 Hodong Kim <cogniti@gmail.com>
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
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "nimf.h"

#define NIMF_TYPE_PREEDIT_WINDOW             (nimf_preedit_window_get_type ())
#define NIMF_PREEDIT_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_PREEDIT_WINDOW, NimfPreeditWindow))
#define NIMF_PREEDIT_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_PREEDIT_WINDOW, NimfPreeditWindowClass))
#define NIMF_IS_PREEDIT_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_PREEDIT_WINDOW))
#define NIMF_IS_PREEDIT_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_PREEDIT_WINDOW))
#define NIMF_PREEDIT_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_PREEDIT_WINDOW, NimfPreeditWindowClass))

typedef struct _NimfPreeditWindow      NimfPreeditWindow;
typedef struct _NimfPreeditWindowClass NimfPreeditWindowClass;

struct _NimfPreeditWindowClass
{
  NimfServiceClass parent_class;
};

struct _NimfPreeditWindow
{
  NimfService parent_instance;

  gchar    *id;
  gboolean  active;

  GtkWidget     *window;
  GtkWidget     *entry;
  NimfRectangle  cursor_area;
};

GType nimf_preedit_window_get_type (void) G_GNUC_CONST;

static void nimf_preedit_window_show (NimfPreeditable *preeditable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditWindow *preedit_window = NIMF_PREEDIT_WINDOW (preeditable);

  GtkRequisition  natural_size;
  int             x, y, w, h;
  int             screen_width, screen_height;

  #if GTK_CHECK_VERSION (3, 22, 0)
    GdkRectangle  geometry;
    GdkDisplay   *display = gtk_widget_get_display (preedit_window->window);
    GdkWindow    *window  = gtk_widget_get_window  (preedit_window->window);
    GdkMonitor   *monitor = gdk_display_get_monitor_at_window (display, window);
    gdk_monitor_get_geometry (monitor, &geometry);
    screen_width  = geometry.width;
    screen_height = geometry.height;
  #else
    screen_width  = gdk_screen_width ();
    screen_height = gdk_screen_height ();
  #endif

  gtk_widget_show_all (preedit_window->window);
  gtk_widget_get_preferred_size (preedit_window->window, NULL, &natural_size);
  gtk_window_resize (GTK_WINDOW (preedit_window->window),
                     natural_size.width, natural_size.height);
  gtk_window_get_size (GTK_WINDOW (preedit_window->window), &w, &h);

  x = preedit_window->cursor_area.x - preedit_window->cursor_area.width;
  y = preedit_window->cursor_area.y + preedit_window->cursor_area.height;

  if (x + w > screen_width)
    x = screen_width - w;

  if (y + h > screen_height)
    y = preedit_window->cursor_area.y - h;

  gtk_window_move (GTK_WINDOW (preedit_window->window), x, y);
}

static void nimf_preedit_window_hide (NimfPreeditable *preeditable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gtk_widget_hide (NIMF_PREEDIT_WINDOW (preeditable)->window);
}

static gboolean nimf_preedit_window_is_visible (NimfPreeditable *preeditable)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return gtk_widget_is_visible (NIMF_PREEDIT_WINDOW (preeditable)->window);
}

static void nimf_preedit_window_set_text (NimfPreeditable *preeditable,
                                          const gchar     *text,
                                          gint             cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkEntry *entry = GTK_ENTRY (NIMF_PREEDIT_WINDOW (preeditable)->entry);
  gtk_entry_set_text        (entry, text);
  gtk_editable_set_position (GTK_EDITABLE (entry), cursor_pos);
  gtk_entry_set_width_chars (entry, g_utf8_strlen (text, -1) * 2);
}

static void
nimf_preedit_window_set_cursor_location (NimfPreeditable     *preeditable,
                                         const NimfRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditWindow *preedit_window = NIMF_PREEDIT_WINDOW (preeditable);
  preedit_window->cursor_area = *area;
}

static void
nimf_preedit_window_iface_init (NimfPreeditableInterface *iface)
{
  iface->show                = nimf_preedit_window_show;
  iface->hide                = nimf_preedit_window_hide;
  iface->is_visible          = nimf_preedit_window_is_visible;
  iface->set_text            = nimf_preedit_window_set_text;
  iface->set_cursor_location = nimf_preedit_window_set_cursor_location;
}

G_DEFINE_DYNAMIC_TYPE_EXTENDED (NimfPreeditWindow,
                                nimf_preedit_window,
                                NIMF_TYPE_SERVICE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (NIMF_TYPE_PREEDITABLE,
                                                               nimf_preedit_window_iface_init));

const gchar *
nimf_preedit_window_get_id (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_SERVICE (service), NULL);

  return NIMF_PREEDIT_WINDOW (service)->id;
}

static gboolean nimf_preedit_window_is_active (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_PREEDIT_WINDOW (service)->active;
}

static gboolean
on_entry_draw (GtkWidget *widget,
               cairo_t   *cr,
               gpointer   user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GtkStyleContext *style_context;
  PangoContext    *pango_context;
  PangoLayout     *layout;
  const char      *text;
  gint             cursor_index;
  gint             x, y;

  style_context = gtk_widget_get_style_context (widget);
  pango_context = gtk_widget_get_pango_context (widget);
  layout = gtk_entry_get_layout (GTK_ENTRY (widget));
  text = pango_layout_get_text (layout);
  gtk_entry_get_layout_offsets (GTK_ENTRY (widget), &x, &y);
  cursor_index = g_utf8_offset_to_pointer (text, gtk_editable_get_position (GTK_EDITABLE (widget))) - text;
  gtk_render_insertion_cursor (style_context, cr, x, y, layout, cursor_index,
                               pango_context_get_base_dir (pango_context));
  return FALSE;
}

static gboolean nimf_preedit_window_start (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditWindow *preedit_window = NIMF_PREEDIT_WINDOW (service);

  if (preedit_window->active)
    return TRUE;

  if (!gtk_init_check (NULL, NULL))
    return FALSE;

  preedit_window->entry = gtk_entry_new ();
  gtk_editable_set_editable (GTK_EDITABLE (preedit_window->entry), FALSE);
  g_signal_connect_after (preedit_window->entry, "draw",
                          G_CALLBACK (on_entry_draw), NULL);
  /* gtk window */
  preedit_window->window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_type_hint (GTK_WINDOW (preedit_window->window),
                            GDK_WINDOW_TYPE_HINT_POPUP_MENU);
  gtk_container_set_border_width (GTK_CONTAINER (preedit_window->window), 1);
  gtk_container_add (GTK_CONTAINER (preedit_window->window),
                     preedit_window->entry);
  gtk_widget_realize (preedit_window->window);
  gtk_window_move (GTK_WINDOW (preedit_window->window), 0, 0);

  preedit_window->active = TRUE;

  return TRUE;
}

static void nimf_preedit_window_stop (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditWindow *preedit_window = NIMF_PREEDIT_WINDOW (service);

  if (!preedit_window->active)
    return;

  preedit_window->active = FALSE;
}

static void
nimf_preedit_window_init (NimfPreeditWindow *preedit_window)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  preedit_window->id = g_strdup ("nimf-preedit-window");
}

static void
nimf_preedit_window_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditWindow *preedit_window = NIMF_PREEDIT_WINDOW (object);

  if (preedit_window->active)
    nimf_preedit_window_stop (NIMF_SERVICE (preedit_window));

  gtk_widget_destroy (preedit_window->window);
  g_free (preedit_window->id);

  G_OBJECT_CLASS (nimf_preedit_window_parent_class)->finalize (object);
}

static void
nimf_preedit_window_class_init (NimfPreeditWindowClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass     *object_class  = G_OBJECT_CLASS (class);
  NimfServiceClass *service_class = NIMF_SERVICE_CLASS (class);

  service_class->get_id    = nimf_preedit_window_get_id;
  service_class->start     = nimf_preedit_window_start;
  service_class->stop      = nimf_preedit_window_stop;
  service_class->is_active = nimf_preedit_window_is_active;

  object_class->finalize = nimf_preedit_window_finalize;
}

static void
nimf_preedit_window_class_finalize (NimfPreeditWindowClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_preedit_window_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_preedit_window_get_type ();
}
