
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-key-syms.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2016,2017 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-enum-types.h"

const gchar *
nimf_keyval_to_keysym_name (guint keyval)
{
  GEnumClass *enum_class = (GEnumClass *) g_type_class_ref (NIMF_TYPE_KEY_SYM);
  GEnumValue *enum_value = g_enum_get_value (enum_class, keyval);
  g_type_class_unref (enum_class);

  return enum_value ? enum_value->value_nick : NULL;
}
