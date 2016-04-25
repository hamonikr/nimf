/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-message.c
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

#include "nimf-message.h"
#include "nimf-types.h"
#include "nimf-enum-types.h"
#include <string.h>

NimfMessage *
nimf_message_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_message_new_full (NIMF_MESSAGE_NONE, 0, NULL, 0, NULL);
}

NimfMessage *
nimf_message_new_full (NimfMessageType type,
                       guint16         client_id,
                       gpointer        data,
                       guint16         data_len,
                       GDestroyNotify  data_destroy_func)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMessage *message;

  message                    = g_slice_new0 (NimfMessage);
  message->header            = g_slice_new0 (NimfMessageHeader);
  message->header->client_id = client_id;
  message->header->type      = type;
  message->header->data_len  = data_len;
  message->data              = data;
  message->data_destroy_func = data_destroy_func;
  message->ref_count = 1;

  return message;
}

NimfMessage *
nimf_message_ref (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (message != NULL, NULL);

  g_atomic_int_inc (&message->ref_count);

  return message;
}

void
nimf_message_unref (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (message == NULL))
    return;

  if (g_atomic_int_dec_and_test (&message->ref_count))
  {
    g_slice_free (NimfMessageHeader, message->header);

    if (message->data_destroy_func)
      message->data_destroy_func (message->data);

    g_slice_free (NimfMessage, message);
  }
}

const NimfMessageHeader *
nimf_message_get_header (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->header;
}

guint16
nimf_message_get_header_size ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return sizeof (NimfMessageHeader);
}

void
nimf_message_set_body (NimfMessage    *message,
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
nimf_message_get_body (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->data;
}

guint16
nimf_message_get_body_size (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->header->data_len;
}

const gchar *nimf_message_get_name (NimfMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GEnumClass *enum_class = (GEnumClass *) g_type_class_ref (NIMF_TYPE_MESSAGE_TYPE);
  GEnumValue *enum_value = g_enum_get_value (enum_class, message->header->type);
  g_type_class_unref (enum_class);

  return enum_value ? enum_value->value_name : NULL;
}

const gchar *nimf_message_get_name_by_type (NimfMessageType type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GEnumClass *enum_class = (GEnumClass *) g_type_class_ref (NIMF_TYPE_MESSAGE_TYPE);
  GEnumValue *enum_value = g_enum_get_value (enum_class, type);
  g_type_class_unref (enum_class);

  return enum_value ? enum_value->value_name : NULL;
}
