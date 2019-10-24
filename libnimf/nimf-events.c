/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-events.c
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

#include "nimf-events.h"

/**
 * SECTION:nimf-events
 * @title: Events
 * @section_id: nimf-events
 */

/**
 * nimf_event_keycode_to_qwerty_keyval:
 * @event: a #NimfEvent
 *
 * Converts @event to qwerty keyval. Use only for PC keyboards.
 *
 * Returns: the #guint value
 */
guint
nimf_event_keycode_to_qwerty_keyval (const NimfEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  guint keyval = 0;
  gboolean is_shift = event->key.state & NIMF_SHIFT_MASK;

  switch (event->key.hardware_keycode)
  {
    /* 20(-) ~ 21(=) */
    case 20: keyval = is_shift ? '_' : '-';  break;
    case 21: keyval = is_shift ? '+' : '=';  break;
    /* 24(q) ~ 35(]) */
    case 24: keyval = is_shift ? 'Q' : 'q';  break;
    case 25: keyval = is_shift ? 'W' : 'w';  break;
    case 26: keyval = is_shift ? 'E' : 'e';  break;
    case 27: keyval = is_shift ? 'R' : 'r';  break;
    case 28: keyval = is_shift ? 'T' : 't';  break;
    case 29: keyval = is_shift ? 'Y' : 'y';  break;
    case 30: keyval = is_shift ? 'U' : 'u';  break;
    case 31: keyval = is_shift ? 'I' : 'i';  break;
    case 32: keyval = is_shift ? 'O' : 'o';  break;
    case 33: keyval = is_shift ? 'P' : 'p';  break;
    case 34: keyval = is_shift ? '{' : '[';  break;
    case 35: keyval = is_shift ? '}' : ']';  break;
    /* 38(a) ~ 48(') */
    case 38: keyval = is_shift ? 'A' : 'a';  break;
    case 39: keyval = is_shift ? 'S' : 's';  break;
    case 40: keyval = is_shift ? 'D' : 'd';  break;
    case 41: keyval = is_shift ? 'F' : 'f';  break;
    case 42: keyval = is_shift ? 'G' : 'g';  break;
    case 43: keyval = is_shift ? 'H' : 'h';  break;
    case 44: keyval = is_shift ? 'J' : 'j';  break;
    case 45: keyval = is_shift ? 'K' : 'k';  break;
    case 46: keyval = is_shift ? 'L' : 'l';  break;
    case 47: keyval = is_shift ? ':' : ';';  break;
    case 48: keyval = is_shift ? '"' : '\''; break;
    /* 52(z) ~ 61(?) */
    case 52: keyval = is_shift ? 'Z' : 'z';  break;
    case 53: keyval = is_shift ? 'X' : 'x';  break;
    case 54: keyval = is_shift ? 'C' : 'c';  break;
    case 55: keyval = is_shift ? 'V' : 'v';  break;
    case 56: keyval = is_shift ? 'B' : 'b';  break;
    case 57: keyval = is_shift ? 'N' : 'n';  break;
    case 58: keyval = is_shift ? 'M' : 'm';  break;
    case 59: keyval = is_shift ? '<' : ',';  break;
    case 60: keyval = is_shift ? '>' : '.';  break;
    case 61: keyval = is_shift ? '?' : '/';  break;
    default: keyval = event->key.keyval;     break;
  }

  return keyval;
}

/**
 * nimf_event_matches:
 * @event: a #NimfEvent
 * @keys: a %NULL-terminated array of #NimfKey
 *
 * Checks if @event matches one of the @keys.
 *
 * Returns: #TRUE if a match was found.
 */
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
    if ((event->key.state & mods_mask) == (keys[i]->state & mods_mask) &&
        event->key.keyval == keys[i]->keyval)
      return TRUE;
  }

  return FALSE;
}

/**
 * nimf_event_new:
 * @type: a #NimfEventType
 *
 * Creates a new event of the given type. All fields are set to 0.
 *
 * Returns: a new #NimfEvent, which should be freed with nimf_event_free().
 */
NimfEvent *
nimf_event_new (NimfEventType type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfEvent *new_event = g_slice_new0 (NimfEvent);
  new_event->type = type;

  return new_event;
}

/**
 * nimf_event_free:
 * @event: a #NimfEvent
 *
 * Frees @event.
 */
void
nimf_event_free (NimfEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (event != NULL);

  g_slice_free (NimfEvent, event);
}
