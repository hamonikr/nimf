/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-events.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015,2016 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-events.h"
#include "nimf-types.h"
#include "nimf-key-syms.h"
#include <string.h>

gboolean
nimf_event_matches (NimfEvent *event, const NimfKey **keys)
{
  g_debug (G_STRLOC ": %s: event->key.state: %d", G_STRFUNC, event->key.state);

  gint i;

  /* Ignore NIMF_MOD2_MASK (Number),
   *        NIMF_LOCK_MASK (CapsLock),
   *        virtual modifiers
   */
  guint mods_mask = NIMF_SHIFT_MASK   |
                    NIMF_CONTROL_MASK |
                    NIMF_MOD1_MASK    |
                    NIMF_MOD3_MASK    |
                    NIMF_MOD4_MASK    |
                    NIMF_MOD5_MASK;

  for (i = 0; keys[i] != 0; i++)
  {
    if ((event->key.state & mods_mask) == (keys[i]->mods & mods_mask) &&
        event->key.keyval == keys[i]->keyval)
      return TRUE;
  }

  return FALSE;
}

NimfEvent *
nimf_event_new (NimfEventType type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEvent *new_event = g_slice_new0 (NimfEvent);
  new_event->type = type;

  return new_event;
}

void
nimf_event_free (NimfEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (event != NULL);

  g_slice_free (NimfEvent, event);
}

NimfEvent *
nimf_event_copy (NimfEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (event != NULL, NULL);

  NimfEvent *new_event;
  new_event = nimf_event_new (NIMF_EVENT_NOTHING);
  *new_event = *event;

  return new_event;
}

G_DEFINE_BOXED_TYPE (NimfEvent, nimf_event, nimf_event_copy, nimf_event_free)
