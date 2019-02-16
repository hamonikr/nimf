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

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-types.h"

G_BEGIN_DECLS

#define NIMF_TYPE_EVENT (nimf_event_get_type ())

typedef struct _NimfEventKey NimfEventKey;
typedef union  _NimfEvent    NimfEvent;

typedef enum
{
  NIMF_EVENT_NOTHING     = -1,
  NIMF_EVENT_KEY_PRESS   =  0,
  NIMF_EVENT_KEY_RELEASE =  1,
} NimfEventType;

struct _NimfEventKey
{
  NimfEventType type;
  guint32       state;
  guint32       keyval;
  guint32       hardware_keycode;
};

union _NimfEvent
{
  NimfEventType type;
  NimfEventKey  key;
};

GType      nimf_event_get_type (void) G_GNUC_CONST;
NimfEvent *nimf_event_new                      (NimfEventType     type);
NimfEvent *nimf_event_copy                     (NimfEvent        *event);
void       nimf_event_free                     (NimfEvent        *event);
gboolean   nimf_event_matches                  (NimfEvent        *event,
                                                const NimfKey   **keys);
guint      nimf_event_keycode_to_qwerty_keyval (const NimfEvent  *event);

G_END_DECLS

#endif /* __NIMF_EVENTS_H__ */
