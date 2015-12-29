/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-events.h
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

#ifndef __DASOM_EVENTS_H__
#define __DASOM_EVENTS_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>
#include "dasom-types.h"

G_BEGIN_DECLS

#define DASOM_TYPE_EVENT (dasom_event_get_type ())

typedef struct _DasomEventKey DasomEventKey;
typedef union  _DasomEvent    DasomEvent;

typedef enum
{
  DASOM_EVENT_NOTHING     = -1,
  DASOM_EVENT_KEY_PRESS   =  0,
  DASOM_EVENT_KEY_RELEASE =  1,
} DasomEventType;

struct _DasomEventKey
{
  DasomEventType type;
  guint          state;
  guint          keyval;
  guint16        hardware_keycode;
};

union _DasomEvent
{
  DasomEventType type;
  DasomEventKey  key;
};

GType       dasom_event_get_type   (void) G_GNUC_CONST;
DasomEvent *dasom_event_new        (DasomEventType       type);
DasomEvent *dasom_event_copy       (DasomEvent          *event);
void        dasom_event_free       (DasomEvent          *event);
gboolean    dasom_event_matches    (DasomEvent          *event,
                                    const DasomKey     **keys);

G_END_DECLS

#endif /* __DASOM_EVENTS_H__ */
