/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-message.c
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

#include "dasom-message.h"
#include "dasom-types.h"
#include "dasom-enum-types.h"
#include <string.h>

DasomMessage *
dasom_message_new ()
{
  DasomMessage *message;
  message = g_slice_new0 (DasomMessage);

  return message;
}

DasomMessage *
dasom_message_new_full (DasomMessageType type,
                        gpointer         data,
                        GDestroyNotify   data_destroy_func)
{
  DasomMessage *message;

  message = g_slice_new0 (DasomMessage);
  message->type = type;
  message->body.data = data;
  message->body.data_destroy_func = data_destroy_func;

  switch (type)
  {
    case DASOM_MESSAGE_CONNECT:
      message->body.data_len = sizeof (DasomConnectionType);
      break;
    case DASOM_MESSAGE_FILTER_EVENT:
      message->body.data_len = sizeof (DasomEvent);
      break;
    case DASOM_MESSAGE_FILTER_EVENT_REPLY:
      message->body.data_len = sizeof (gboolean);
      break;
    case DASOM_MESSAGE_GET_PREEDIT_STRING_REPLY:
      message->body.data_len = strlen (data) + 1 + sizeof (gint);
      break;
    case DASOM_MESSAGE_COMMIT:
    case DASOM_MESSAGE_ENGINE_CHANGED:
      message->body.data_len = strlen (message->body.data) + 1;
      break;
    default:
      message->body.data_len = 0;
      break;
  }

  return message;
}

void dasom_message_free (DasomMessage *message)
{
  g_return_if_fail (message != NULL);

  if (message->body.data_destroy_func)
    message->body.data_destroy_func (message->body.data);

  g_slice_free (DasomMessage, message);
  message = NULL;
}

const gchar *dasom_message_get_name (DasomMessage *message)
{
  GEnumClass *enum_class = (GEnumClass *) g_type_class_ref (DASOM_TYPE_MESSAGE_TYPE);
  GEnumValue *enum_value = g_enum_get_value (enum_class, message->type);
  g_type_class_unref (enum_class);

  return enum_value ? enum_value->value_name : NULL;
}
