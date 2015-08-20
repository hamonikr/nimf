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
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomMessage *message;

  message            = g_slice_new0 (DasomMessage);
  message->header    = g_slice_new0 (DasomMessageHeader);
  message->ref_count = 1;

  return message;
}

DasomMessage *
dasom_message_new_full (DasomMessageType type,
                        gpointer         data,
                        guint16          data_len,
                        GDestroyNotify   data_destroy_func)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomMessage *message;

  message                    = g_slice_new0 (DasomMessage);
  message->header            = g_slice_new0 (DasomMessageHeader);
  message->header->type      = type;
  message->header->data_len  = data_len;
  message->data              = data;
  message->data_destroy_func = data_destroy_func;
  message->ref_count = 1;

  return message;
}

DasomMessage *
dasom_message_ref (DasomMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (message != NULL, NULL);

  g_atomic_int_inc (&message->ref_count);

  return message;
}

void
dasom_message_unref (DasomMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (message == NULL))
    return;

  if (g_atomic_int_dec_and_test (&message->ref_count))
  {
    g_slice_free (DasomMessageHeader, message->header);

    if (message->data_destroy_func)
      message->data_destroy_func (message->data);

    g_slice_free (DasomMessage, message);
  }
}

const DasomMessageHeader *
dasom_message_get_header (DasomMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->header;
}

guint16
dasom_message_get_header_size ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return sizeof (DasomMessageHeader);
}

void
dasom_message_set_body (DasomMessage   *message,
                        gchar          *data,
                        guint16         data_len,
                        GDestroyNotify  data_destroy_func)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  message->data              = data;
  message->header->data_len  = data_len;
  message->data_destroy_func = data_destroy_func;
}

const gchar *
dasom_message_get_body (DasomMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->data;
}

guint16
dasom_message_get_body_size (DasomMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->header->data_len;
}

const gchar *dasom_message_get_name (DasomMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GEnumClass *enum_class = (GEnumClass *) g_type_class_ref (DASOM_TYPE_MESSAGE_TYPE);
  GEnumValue *enum_value = g_enum_get_value (enum_class, message->header->type);
  g_type_class_unref (enum_class);

  return enum_value ? enum_value->value_name : NULL;
}

const gchar *dasom_message_get_name_by_type (DasomMessageType type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GEnumClass *enum_class = (GEnumClass *) g_type_class_ref (DASOM_TYPE_MESSAGE_TYPE);
  GEnumValue *enum_value = g_enum_get_value (enum_class, type);
  g_type_class_unref (enum_class);

  return enum_value ? enum_value->value_name : NULL;
}
