/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-events.h
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

#ifndef __NIMF_EVENTS_H__
#define __NIMF_EVENTS_H__

#include <glib-object.h>
#include "nimf-types.h"

G_BEGIN_DECLS

typedef struct _NimfEventKey NimfEventKey;
typedef union  _NimfEvent    NimfEvent;

/**
 * NimfEventType:
 * @NIMF_EVENT_NOTHING: a special code to indicate a null event.
 * @NIMF_EVENT_KEY_PRESS: a key has been pressed.
 * @NIMF_EVENT_KEY_RELEASE: a key has been released.
 */
typedef enum
{
  NIMF_EVENT_NOTHING     = -1,
  NIMF_EVENT_KEY_PRESS   =  0,
  NIMF_EVENT_KEY_RELEASE =  1,
} NimfEventType;

/**
 * NimfEventKey:
 * @type: the type of the event (%NIMF_EVENT_KEY_PRESS or
 *   %NIMF_EVENT_KEY_RELEASE).
 * @state: (type NimfModifierType): a bit-mask representing the state of
 *   the modifier keys (e.g. Control, Shift and Alt) and the pointer
 *   buttons. See #NimfModifierType.
 * @keyval: the key that was pressed or released. See the
 *   `nimf-key-syms.h` header file for a complete list of Nimf key codes.
 * @hardware_keycode: the raw code of the key that was pressed or released.
 *
 * Describes a key press or key release event.
 */
struct _NimfEventKey
{
  NimfEventType type;
  guint32       state;
  guint32       keyval;
  guint32       hardware_keycode;
};

/**
 * NimfEvent:
 * @type: a #NimfEventType
 * @key: a #NimfEventKey
 *
 * A #NimfEvent contains a union.
 */
union _NimfEvent
{
  NimfEventType type;
  NimfEventKey  key;
};

NimfEvent *nimf_event_new                      (NimfEventType     type);
void       nimf_event_free                     (NimfEvent        *event);
gboolean   nimf_event_matches                  (NimfEvent        *event,
                                                const NimfKey   **keys);
guint      nimf_event_keycode_to_qwerty_keyval (const NimfEvent  *event);

G_END_DECLS

#endif /* __NIMF_EVENTS_H__ */
