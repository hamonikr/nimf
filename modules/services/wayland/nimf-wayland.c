/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-wayland.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2012 Intel Corporation
 * Copyright (C) 2017-2019 Hodong Kim <cogniti@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"
#include "nimf-wayland-ic.h"
#include "nimf-wayland.h"
#include "nimf-service.h"

G_DEFINE_DYNAMIC_TYPE (NimfWayland, nimf_wayland, NIMF_TYPE_SERVICE);

typedef struct
{
  GSource      source;
  NimfWayland *wayland;
  GPollFD      poll_fd;
  gboolean     reading;
} NimfWaylandEventSource;

static gboolean nimf_wayland_source_prepare (GSource *base,
                                             gint    *timeout)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWaylandEventSource *source = (NimfWaylandEventSource *) base;

  *timeout = -1;

  if (source->reading)
    return FALSE;

  if (wl_display_prepare_read (source->wayland->display) != 0)
    return TRUE;

  source->reading = TRUE;

  if (wl_display_flush (source->wayland->display) < 0)
    g_critical (G_STRLOC ": %s: wl_display_flush() failed: %s", G_STRFUNC,
                g_strerror (errno));

  return FALSE;
}

static void nimf_wayland_stop (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWayland *wayland = NIMF_WAYLAND (service);

  if (!wayland->active)
    return;

  if (wayland->event_source)
  {
    g_source_destroy (wayland->event_source);
    g_source_unref   (wayland->event_source);
  }

  wayland->active = FALSE;
}

static gboolean nimf_wayland_source_check (GSource *base)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWaylandEventSource *source = (NimfWaylandEventSource *) base;

  if (source->reading)
  {
    if (source->poll_fd.revents & G_IO_IN)
    {
      if (wl_display_read_events (source->wayland->display) < 0)
      {
        g_critical (G_STRLOC ": %s: wl_display_read_events() failed: %s",
                    G_STRFUNC, g_strerror (errno));
        nimf_wayland_stop (NIMF_SERVICE (source->wayland));
      }
    }
    else
    {
      wl_display_cancel_read (source->wayland->display);
    }

    source->reading = FALSE;
  }

  return source->poll_fd.revents;
}

static gboolean nimf_wayland_source_dispatch (GSource     *base,
                                              GSourceFunc  callback,
                                              gpointer     user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWaylandEventSource *source = (NimfWaylandEventSource *) base;

  if (wl_display_dispatch_pending (source->wayland->display) < 0)
  {
    g_critical (G_STRLOC ": %s: wl_display_dispatch_pending() failed: %s: %m",
                G_STRFUNC, g_strerror (errno));
    nimf_wayland_stop (NIMF_SERVICE (source->wayland));
  }

  source->poll_fd.revents = 0;

  return TRUE;
}

static void nimf_wayland_source_finalize (GSource *base)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWaylandEventSource *source = (NimfWaylandEventSource *) base;

  if (source->reading)
    wl_display_cancel_read (source->wayland->display);

  source->reading = FALSE;
}

static GSourceFuncs event_funcs = {
  nimf_wayland_source_prepare,
  nimf_wayland_source_check,
  nimf_wayland_source_dispatch,
  nimf_wayland_source_finalize
};

GSource *
nimf_wayland_source_new (NimfWayland *wayland)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSource *source;
  NimfWaylandEventSource *wl_source;

  source = g_source_new (&event_funcs, sizeof (NimfWaylandEventSource));
  wl_source = (NimfWaylandEventSource *) source;

  wl_source->wayland = wayland;
  wl_source->poll_fd.fd = wl_display_get_fd (wl_source->wayland->display);
  wl_source->poll_fd.events = G_IO_IN | G_IO_ERR | G_IO_HUP;
  g_source_add_poll (source, &wl_source->poll_fd);

  g_source_set_priority (source, G_PRIORITY_DEFAULT);
  g_source_set_can_recurse (source, TRUE);

  return source;
}

static void
handle_surrounding_text (void *data,
                         struct zwp_input_method_context_v1 *context,
                         const char *text,
                         uint32_t cursor,
                         uint32_t anchor)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
handle_reset (void *data,
              struct zwp_input_method_context_v1 *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
handle_content_type (void *data,
                     struct zwp_input_method_context_v1 *context,
                     uint32_t hint,
                     uint32_t purpose)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
handle_invoke_action (void *data,
                      struct zwp_input_method_context_v1 *context,
                      uint32_t button,
                      uint32_t index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
handle_commit_state (void *data,
                     struct zwp_input_method_context_v1 *context,
                     uint32_t serial)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWayland *wayland = data;

  wayland->serial = serial;
}

static void
handle_preferred_language (void *data,
                           struct zwp_input_method_context_v1 *context,
                           const char *language)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static const struct zwp_input_method_context_v1_listener input_method_context_listener = {
  handle_surrounding_text,
  handle_reset,
  handle_content_type,
  handle_invoke_action,
  handle_commit_state,
  handle_preferred_language
};

static void
input_method_keyboard_keymap (void               *data,
                              struct wl_keyboard *wl_keyboard,
                              uint32_t            format,
                              int32_t             fd,
                              uint32_t            size)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWayland *wayland = data;

  char *map_str;

  if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
  {
    close (fd);
    return;
  }

  map_str = mmap (NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  if (map_str == MAP_FAILED)
  {
    close (fd);
    return;
  }

  wayland->keymap = xkb_keymap_new_from_string (wayland->xkb_context,
                                                map_str,
                                                XKB_KEYMAP_FORMAT_TEXT_V1,
                                                XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap (map_str, size);
  close (fd);

  if (!wayland->keymap)
  {
    g_critical (G_STRLOC ": %s: xkb_keymap_new_from_string() failed",
                G_STRFUNC);
    return;
  }

  wayland->state = xkb_state_new (wayland->keymap);
  if (!wayland->state)
  {
    g_critical (G_STRLOC ": %s: xkb_state_new() failed", G_STRFUNC);
    xkb_keymap_unref(wayland->keymap);
    return;
  }

  wayland->shift_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Shift");
  wayland->lock_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Lock");
  wayland->control_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Control");
  wayland->mod1_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Mod1");
  wayland->mod2_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Mod2");
  wayland->mod3_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Mod3");
  wayland->mod4_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Mod4");
  wayland->mod5_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Mod5");
  wayland->super_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Super");
  wayland->hyper_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Hyper");
  wayland->meta_mask =
    1 << xkb_keymap_mod_get_index (wayland->keymap, "Meta");
}

static void
input_method_keyboard_key (void *data,
                           struct wl_keyboard *wl_keyboard,
                           uint32_t serial,
                           uint32_t time,
                           uint32_t key,
                           uint32_t state_w)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWayland *wayland = data;
  uint32_t code;
  uint32_t num_syms;
  const xkb_keysym_t *syms;
  xkb_keysym_t sym;
  enum wl_keyboard_key_state state = state_w;
  guint32 modifiers;
  NimfEvent *event;

  if (!wayland->state)
    return;

  code = key + 8;
  num_syms = xkb_state_key_get_syms (wayland->state, code, &syms);

  sym = XKB_KEY_NoSymbol;
  if (num_syms == 1)
    sym = syms[0];

  event = nimf_event_new (NIMF_EVENT_NOTHING);

  modifiers = wayland->modifiers;
  if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
  {
      modifiers |= NIMF_RELEASE_MASK;
      event->key.type = NIMF_EVENT_KEY_RELEASE;
  }
  else
  {
    event->key.type = NIMF_EVENT_KEY_PRESS;
  }

  event->key.state = modifiers;
  event->key.keyval = sym;
  event->key.hardware_keycode = code;

  if (!nimf_service_ic_filter_event (NIMF_SERVICE_IC (wayland->wic), event))
    zwp_input_method_context_v1_key (wayland->context, serial, time, key, state);

  nimf_event_free (event);
}

static void
input_method_keyboard_modifiers (void *data,
                                 struct wl_keyboard *wl_keyboard,
                                 uint32_t serial,
                                 uint32_t mods_depressed,
                                 uint32_t mods_latched,
                                 uint32_t mods_locked,
                                 uint32_t group)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWayland *wayland = data;
  struct zwp_input_method_context_v1 *context = wayland->context;
  xkb_mod_mask_t mask;

  xkb_state_update_mask (wayland->state, mods_depressed,
                         mods_latched, mods_locked, 0, 0, group);
  mask = xkb_state_serialize_mods (wayland->state,
                                   XKB_STATE_MODS_DEPRESSED |
                                   XKB_STATE_MODS_LATCHED);
  wayland->modifiers = 0;
  if (mask & wayland->shift_mask)
    wayland->modifiers |= NIMF_SHIFT_MASK;
  if (mask & wayland->lock_mask)
    wayland->modifiers |= NIMF_LOCK_MASK;
  if (mask & wayland->control_mask)
    wayland->modifiers |= NIMF_CONTROL_MASK;
  if (mask & wayland->mod1_mask)
    wayland->modifiers |= NIMF_MOD1_MASK;
  if (mask & wayland->mod2_mask)
    wayland->modifiers |= NIMF_MOD2_MASK;
  if (mask & wayland->mod3_mask)
    wayland->modifiers |= NIMF_MOD3_MASK;
  if (mask & wayland->mod4_mask)
    wayland->modifiers |= NIMF_MOD4_MASK;
  if (mask & wayland->mod5_mask)
    wayland->modifiers |= NIMF_MOD5_MASK;
  if (mask & wayland->super_mask)
    wayland->modifiers |= NIMF_SUPER_MASK;
  if (mask & wayland->hyper_mask)
    wayland->modifiers |= NIMF_HYPER_MASK;
  if (mask & wayland->meta_mask)
    wayland->modifiers |= NIMF_META_MASK;

  zwp_input_method_context_v1_modifiers (context, serial,
                                         mods_depressed, mods_depressed,
                                         mods_latched, group);
}

static const struct wl_keyboard_listener input_method_keyboard_listener = {
  input_method_keyboard_keymap,
  NULL, /* enter */
  NULL, /* leave */
  input_method_keyboard_key,
  input_method_keyboard_modifiers
};

static void
input_method_activate (void *data,
                       struct zwp_input_method_v1 *input_method,
                       struct zwp_input_method_context_v1 *context)
{
  g_debug (G_STRLOC ": %s: %p, %p", G_STRFUNC, input_method, context);

  NimfWayland *wayland = data;

  if (wayland->context)
    zwp_input_method_context_v1_destroy (wayland->context);

  wayland->serial = 0;
  wayland->context = context;
  zwp_input_method_context_v1_add_listener (context,
                                            &input_method_context_listener,
                                            wayland);
  wayland->keyboard = zwp_input_method_context_v1_grab_keyboard (context);
  wl_keyboard_add_listener (wayland->keyboard,
                            &input_method_keyboard_listener,
                            wayland);
}

static void
input_method_deactivate (void *data,
                         struct zwp_input_method_v1 *input_method,
                         struct zwp_input_method_context_v1 *context)
{
  g_debug (G_STRLOC ": %s: %p, %p", G_STRFUNC, input_method, context);

  NimfWayland *wayland = data;

  if (!wayland->context)
    return;

  nimf_service_ic_reset (NIMF_SERVICE_IC (wayland->wic));
  zwp_input_method_context_v1_destroy (wayland->context);
  wayland->context = NULL;
}

static const struct zwp_input_method_v1_listener input_method_listener = {
  input_method_activate,
  input_method_deactivate
};

static void
registry_handle_global (void               *data,
                        struct wl_registry *registry,
                        uint32_t            name,
                        const char         *interface,
                        uint32_t            version)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWayland *wayland = data;

  if (!g_strcmp0 (interface, "zwp_input_method_v1"))
  {
    wayland->input_method =
      wl_registry_bind (registry, name, &zwp_input_method_v1_interface, 1);
    zwp_input_method_v1_add_listener (wayland->input_method,
                                      &input_method_listener, wayland);
  }
}

static void
registry_handle_global_remove (void *data, struct wl_registry *registry,
                               uint32_t name)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static const struct wl_registry_listener registry_listener = {
  registry_handle_global,
  registry_handle_global_remove
};

static gboolean nimf_wayland_is_active (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_WAYLAND (service)->active;
}

static gboolean nimf_wayland_start (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWayland *wayland = NIMF_WAYLAND (service);
  const gchar *type;

  if (wayland->active)
    return TRUE;

  type = g_getenv ("XDG_SESSION_TYPE");

  if (type && g_strcmp0 (type, "wayland"))
    return FALSE;

  wayland->display = wl_display_connect (NULL);
  if (wayland->display == NULL)
  {
    g_warning (G_STRLOC ": %s: wl_display_connect() failed", G_STRFUNC);
    return FALSE;
  }

  wayland->registry = wl_display_get_registry (wayland->display);
  wl_registry_add_listener (wayland->registry, &registry_listener, wayland);
  wl_display_roundtrip (wayland->display);
  if (wayland->input_method == NULL)
  {
    g_critical (G_STRLOC ": %s: No input_method global", G_STRFUNC);
    return FALSE;
  }

  wayland->xkb_context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
  if (wayland->xkb_context == NULL) {
    g_critical (G_STRLOC ": %s: xkb_context_new() failed", G_STRFUNC);
    return FALSE;
  }

  wayland->wic = nimf_wayland_ic_new (wayland);
  wayland->context = NULL;
  wayland->event_source = nimf_wayland_source_new (wayland);
  g_source_attach (wayland->event_source, NULL);

  wayland->active = TRUE;

  return TRUE;
}

static const gchar *
nimf_wayland_get_id (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_SERVICE (service), NULL);

  return NIMF_WAYLAND (service)->id;
}

static void
nimf_wayland_init (NimfWayland *wayland)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  wayland->id = g_strdup ("nimf-wayland");
}

static void
nimf_wayland_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfWayland *wayland = NIMF_WAYLAND (object);

  if (wayland->active)
    nimf_wayland_stop (NIMF_SERVICE (wayland));

  g_free (wayland->id);

  if (wayland->wic)
    g_object_unref (wayland->wic);

  G_OBJECT_CLASS (nimf_wayland_parent_class)->finalize (object);
}

static void
nimf_wayland_class_init (NimfWaylandClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass     *object_class  = G_OBJECT_CLASS (class);
  NimfServiceClass *service_class = NIMF_SERVICE_CLASS (class);

  service_class->get_id    = nimf_wayland_get_id;
  service_class->start     = nimf_wayland_start;
  service_class->stop      = nimf_wayland_stop;
  service_class->is_active = nimf_wayland_is_active;

  object_class->finalize = nimf_wayland_finalize;
}

static void
nimf_wayland_class_finalize (NimfWaylandClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_wayland_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_wayland_get_type ();
}
