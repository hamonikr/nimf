/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-events.c
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

#include "dasom-events.h"
#include "dasom-types.h"
#include "dasom-key-syms.h"
#include <string.h>

typedef struct _dasom_mod_info DasomModifierInfo;

struct _dasom_mod_info {
  gchar *name;
  DasomModifierType mod;
};

static const DasomModifierInfo mod_info_list[] = {
  {"Shift",    DASOM_SHIFT_MASK},
  {"Lock",     DASOM_LOCK_MASK},
  {"Control",  DASOM_CONTROL_MASK},
  {"Mod1",     DASOM_MOD1_MASK},
  {"Mod2",     DASOM_MOD2_MASK}, /* Num Lock */
  {"Mod3",     DASOM_MOD3_MASK},
  {"Mod4",     DASOM_MOD4_MASK},
  {"Mod5",     DASOM_MOD5_MASK},
  {"Button1",  DASOM_BUTTON1_MASK},
  {"Button2",  DASOM_BUTTON2_MASK},
  {"Button3",  DASOM_BUTTON3_MASK},
  {"Button4",  DASOM_BUTTON4_MASK},
  {"Button5",  DASOM_BUTTON5_MASK},
  {"Super",    DASOM_SUPER_MASK},
  {"Hyper",    DASOM_HYPER_MASK},
  {"Meta",     DASOM_META_MASK},
  {"Release",  DASOM_RELEASE_MASK}
};

guint dasom_keyval_from_name (const gchar *keyval_name)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  guint keyval = 0;

  if (g_strcmp0 (keyval_name, "space")  == 0) keyval = DASOM_KEY_space;
  if (g_strcmp0 (keyval_name, "Hangul") == 0) keyval = DASOM_KEY_Hangul;

  return keyval;
}

gchar *dasom_keyval_name (guint keyval)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar *name;

  switch (keyval)
  {
    case DASOM_KEY_space:  name = g_strdup ("space"); break;
    case DASOM_KEY_Hangul: name = g_strdup ("Hangul"); break;
    default:
      name = g_strdup ("");
      break;
  }
  return name;
}

gboolean
dasom_event_matches (DasomEvent *event, const DasomKey **keys)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;
  gint i;

  guint mods = event->key.state & (DASOM_MOD2_MASK | DASOM_RELEASE_MASK);

  for (i = 0; keys[i] != 0; i++)
  {
    if ((event->key.state & DASOM_MODIFIER_MASK) == (keys[i]->mods | mods) &&
        event->key.keyval == keys[i]->keyval)
    {
      retval = TRUE;
      break;
    }
  }

  return retval;
}

gboolean dasom_event_is_hotkey (DasomEvent          *event,
                                const gchar * const *key_names)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomModifierType mods;
  guint keyval = 0;
  guint i, j, k;

  /*
   * example
   *
   * hotkey_names[0] = "Control space";
   * hotkey_names[1] = "Hangul"
   */

  gchar **keys;

  for (i = 0; key_names[i] != NULL; i++)
  {
    keys = g_strsplit (key_names[i], " ", -1);
    mods = event->key.state & (DASOM_MOD2_MASK | DASOM_RELEASE_MASK);

    for (j = 0; keys[j] != NULL; j++)
    {
      for (k = 0; k < G_N_ELEMENTS (mod_info_list); k++)
        if (g_strcmp0 (keys[j], mod_info_list[k].name) == 0)
          mods = mods | mod_info_list[k].mod;

      keyval = dasom_keyval_from_name (keys[j]);

      if ((event->key.state & DASOM_MODIFIER_MASK) == mods && event->key.keyval == keyval)
        return TRUE;
    }

    g_strfreev (keys);
  }

  return FALSE;
}

DasomEvent *
dasom_event_new (DasomEventType type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomEvent *new_event = g_slice_new0 (DasomEvent);
  new_event->type = type;

  return new_event;
}

void
dasom_event_free (DasomEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (event != NULL);

  g_slice_free (DasomEvent, event);
}

DasomEvent *
dasom_event_copy (DasomEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (event != NULL, NULL);

  DasomEvent *new_event;
  new_event = dasom_event_new (DASOM_EVENT_NOTHING);
  *new_event = *event;

  return new_event;
}

G_DEFINE_BOXED_TYPE (DasomEvent, dasom_event, dasom_event_copy, dasom_event_free)
