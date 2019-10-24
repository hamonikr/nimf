/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-system-keyboard.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2016-2018 Hodong Kim <cogniti@gmail.com>
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

#include <nimf.h>
#include <glib/gi18n.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <locale.h>

#define NIMF_TYPE_SYSTEM_KEYBOARD               (nimf_system_keyboard_get_type ())
#define NIMF_SYSTEM_KEYBOARD(object)            (G_TYPE_CHECK_INSTANCE_CAST ((object), NIMF_TYPE_SYSTEM_KEYBOARD, NimfSystemKeyboard))
#define NIMF_SYSTEM_KEYBOARD_CLASS(class)       (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_SYSTEM_KEYBOARD, NimfSystemKeyboardClass))
#define NIMF_IS_SYSTEM_KEYBOARD(object)         (G_TYPE_CHECK_INSTANCE_TYPE ((object), NIMF_TYPE_SYSTEM_KEYBOARD))
#define NIMF_IS_SYSTEM_KEYBOARD_CLASS(class)    (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_SYSTEM_KEYBOARD))
#define NIMF_SYSTEM_KEYBOARD_GET_CLASS(object)  (G_TYPE_INSTANCE_GET_CLASS ((object), NIMF_TYPE_SYSTEM_KEYBOARD, NimfSystemKeyboardClass))

typedef struct _NimfSystemKeyboard      NimfSystemKeyboard;
typedef struct _NimfSystemKeyboardClass NimfSystemKeyboardClass;

struct _NimfSystemKeyboard
{
  NimfEngine parent_instance;

  gchar *id;
  /* preedit */
  gchar              *preedit_string;
  NimfPreeditAttr   **preedit_attrs;
  NimfPreeditState    preedit_state;
  /* compose */
  struct xkb_context       *xkb_context;
  struct xkb_compose_table *xkb_compose_table;
  struct xkb_compose_state *xkb_compose_state;
  gchar                     buffer[8];
};

struct _NimfSystemKeyboardClass
{
  NimfEngineClass parent_class;
};

GType nimf_system_keyboard_get_type (void) G_GNUC_CONST;

G_DEFINE_DYNAMIC_TYPE (NimfSystemKeyboard, nimf_system_keyboard, NIMF_TYPE_ENGINE);

static const gchar *
nimf_system_keyboard_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_SYSTEM_KEYBOARD (engine)->id;
}

static const gchar *
nimf_system_keyboard_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_SYSTEM_KEYBOARD (engine)->id;
}

static void
nimf_system_keyboard_init (NimfSystemKeyboard *keyboard)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  keyboard->id = g_strdup ("nimf-system-keyboard");
  keyboard->preedit_string = g_strdup ("");
  keyboard->preedit_attrs  = g_malloc0_n (2, sizeof (NimfPreeditAttr *));
  keyboard->preedit_attrs[0] = nimf_preedit_attr_new (NIMF_PREEDIT_ATTR_UNDERLINE, 0, 0);
  keyboard->preedit_attrs[1] = NULL;
  keyboard->xkb_context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);

  keyboard->xkb_compose_table =
    xkb_compose_table_new_from_locale (keyboard->xkb_context,
                                       setlocale (LC_CTYPE, NULL),
                                       XKB_COMPOSE_COMPILE_NO_FLAGS);
  if (!keyboard->xkb_compose_table)
    keyboard->xkb_compose_table =
      xkb_compose_table_new_from_locale (keyboard->xkb_context, "C",
                                         XKB_COMPOSE_COMPILE_NO_FLAGS);

  if (!keyboard->xkb_compose_table)
  {
    g_warning ("xkb compose is disabled");
    return;
  }

  keyboard->xkb_compose_state =
    xkb_compose_state_new (keyboard->xkb_compose_table,
                           XKB_COMPOSE_STATE_NO_FLAGS);
}

static void
nimf_system_keyboard_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSystemKeyboard *keyboard = NIMF_SYSTEM_KEYBOARD (object);

  g_free (keyboard->preedit_string);
  nimf_preedit_attr_freev (keyboard->preedit_attrs);

  xkb_compose_state_unref (keyboard->xkb_compose_state);
  xkb_compose_table_unref (keyboard->xkb_compose_table);
  xkb_context_unref       (keyboard->xkb_context);

  g_free (NIMF_SYSTEM_KEYBOARD (object)->id);

  G_OBJECT_CLASS (nimf_system_keyboard_parent_class)->finalize (object);
}

static void
nimf_system_keyboard_update_preedit (NimfEngine    *engine,
                                     NimfServiceIC *target,
                                     gchar         *new_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSystemKeyboard *keyboard = NIMF_SYSTEM_KEYBOARD (engine);

  /* preedit-start */
  if (keyboard->preedit_state == NIMF_PREEDIT_STATE_END && new_preedit[0] != 0)
  {
    keyboard->preedit_state = NIMF_PREEDIT_STATE_START;
    nimf_engine_emit_preedit_start (engine, target);
  }
  /* preedit-changed */
  if (keyboard->preedit_string[0] != 0 || new_preedit[0] != 0)
  {
    g_free (keyboard->preedit_string);
    keyboard->preedit_string = new_preedit;
    keyboard->preedit_attrs[0]->end_index = g_utf8_strlen (keyboard->preedit_string, -1);
    nimf_engine_emit_preedit_changed (engine, target, keyboard->preedit_string,
                                      keyboard->preedit_attrs,
                                      g_utf8_strlen (keyboard->preedit_string,
                                                     -1));
  }
  else
  {
    g_free (new_preedit);
  }
  /* preedit-end */
  if (keyboard->preedit_state == NIMF_PREEDIT_STATE_START &&
      keyboard->preedit_string[0] == 0)
  {
    keyboard->preedit_state = NIMF_PREEDIT_STATE_END;
    nimf_engine_emit_preedit_end (engine, target);
  }
}

static gboolean
nimf_system_keyboard_filter_event (NimfEngine    *engine,
                                   NimfServiceIC *target,
                                   NimfEvent     *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSystemKeyboard *keyboard = NIMF_SYSTEM_KEYBOARD (engine);
  enum xkb_compose_feed_result result;

  if (G_UNLIKELY (!keyboard->xkb_compose_table))
    return FALSE;

  if (event->type == NIMF_EVENT_KEY_RELEASE)
    return FALSE;

  result = xkb_compose_state_feed (keyboard->xkb_compose_state,
                                   event->key.keyval);

  if (result == XKB_COMPOSE_FEED_IGNORED)
    return FALSE;

  switch (xkb_compose_state_get_status (keyboard->xkb_compose_state))
  {
    case XKB_COMPOSE_NOTHING:
      return FALSE;
    case XKB_COMPOSE_COMPOSING:
      if (xkb_keysym_to_utf8 (event->key.keyval, keyboard->buffer,
                              sizeof (keyboard->buffer)) > 0)
        nimf_system_keyboard_update_preedit (engine, target,
          g_strconcat (keyboard->preedit_string, keyboard->buffer, NULL));

      break;
    case XKB_COMPOSE_COMPOSED:
      nimf_system_keyboard_update_preedit (engine, target, g_strdup (""));

      if (xkb_compose_state_get_utf8 (keyboard->xkb_compose_state,
                                      keyboard->buffer,
                                      sizeof (keyboard->buffer)) > 0)
        nimf_engine_emit_commit (engine, target, keyboard->buffer);

      break;
    case XKB_COMPOSE_CANCELLED:
      nimf_system_keyboard_update_preedit (engine, target, g_strdup (""));
      nimf_engine_emit_beep (engine, target);
      break;
    default:
      break;
  }

  return TRUE;
}

static void
nimf_system_keyboard_reset (NimfEngine    *engine,
                            NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSystemKeyboard *keyboard = NIMF_SYSTEM_KEYBOARD (engine);

  if (G_UNLIKELY (!keyboard->xkb_compose_table))
    return;

  xkb_compose_state_reset (keyboard->xkb_compose_state);
  nimf_system_keyboard_update_preedit (engine, target, g_strdup (""));
}

static void
nimf_system_keyboard_class_init (NimfSystemKeyboardClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass    *object_class = G_OBJECT_CLASS (class);
  NimfEngineClass *engine_class = NIMF_ENGINE_CLASS (class);

  engine_class->filter_event  = nimf_system_keyboard_filter_event;
  engine_class->reset         = nimf_system_keyboard_reset;
  engine_class->get_id        = nimf_system_keyboard_get_id;
  engine_class->get_icon_name = nimf_system_keyboard_get_icon_name;

  object_class->finalize = nimf_system_keyboard_finalize;
}

static void
nimf_system_keyboard_class_finalize (NimfSystemKeyboardClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_system_keyboard_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_system_keyboard_get_type ();
}
