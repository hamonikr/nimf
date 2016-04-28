/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-system-keyboard.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2016 Hodong Kim <hodong@gmail.com>
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

  gchar *icon_name;
};

struct _NimfSystemKeyboardClass
{
  NimfEngineClass parent_class;
};

GType nimf_system_keyboard_get_type (void) G_GNUC_CONST;

G_DEFINE_DYNAMIC_TYPE (NimfSystemKeyboard, nimf_system_keyboard, NIMF_TYPE_ENGINE);

const gchar *
nimf_system_keyboard_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_SYSTEM_KEYBOARD_ID;
}

const gchar *
nimf_system_keyboard_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_SYSTEM_KEYBOARD (engine)->icon_name;
}

static void
nimf_system_keyboard_init (NimfSystemKeyboard *keyboard)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  keyboard->icon_name = g_strdup ("keyboard");
}

static void
nimf_system_keyboard_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfSystemKeyboard *keyboard = NIMF_SYSTEM_KEYBOARD (object);

  g_free (keyboard->icon_name);

  G_OBJECT_CLASS (nimf_system_keyboard_parent_class)->finalize (object);
}

static void
nimf_system_keyboard_class_init (NimfSystemKeyboardClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass    *object_class = G_OBJECT_CLASS (class);
  NimfEngineClass *engine_class = NIMF_ENGINE_CLASS (class);

  engine_class->get_id        = nimf_system_keyboard_get_id;
  engine_class->get_icon_name = nimf_system_keyboard_get_icon_name;

  object_class->finalize = nimf_system_keyboard_finalize;
}

static void
nimf_system_keyboard_class_finalize (NimfSystemKeyboardClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static const NimfEngineInfo engine_info = {
  "nimf-system-keyboard",   /* ID */
  N_("System Keyboard"),    /* Human readable name */
  "nimf-indicator-keyboard" /* icon name */
};

const NimfEngineInfo *module_get_info ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return &engine_info;
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
