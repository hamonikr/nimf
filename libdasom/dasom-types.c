/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-types.c
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

#include "dasom-types.h"
#include "dasom-enum-types.h"

G_DEFINE_QUARK (dasom-error-quark, dasom_error)

DasomKey *
dasom_key_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_slice_new0 (DasomKey);
}

DasomKey *
dasom_key_new_from_nicks (const gchar **nicks)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomKey    *key = g_slice_new0 (DasomKey);
  GEnumValue  *enum_value;  /* Do not free */
  GFlagsValue *flags_value; /* Do not free */
  GFlagsClass *flags_class = g_type_class_ref (DASOM_TYPE_MODIFIER_TYPE);
  GEnumClass  *enum_class  = g_type_class_ref (DASOM_TYPE_KEY_SYM);

  gint i;
  for (i = 0; nicks[i] != NULL; i++)
  {
    if (g_str_has_suffix (nicks[i], "mask"))
    {
      flags_value = g_flags_get_value_by_nick (flags_class, nicks[i]);

      if (flags_value)
        key->mods |= flags_value->value;
      else
        g_warning ("DasomModifierType doesn't have a member with that nickname: %s", nicks[i]);
    }
    else
    {
      enum_value = g_enum_get_value_by_nick (enum_class, nicks[i]);

      if (enum_value)
        key->keyval = enum_value->value;
      else
        g_warning ("DasomKeySym doesn't have a member with that nickname: %s", nicks[i]);
    }
  }

  g_type_class_unref (flags_class);
  g_type_class_unref (enum_class);

  return key;
}

DasomKey **dasom_key_newv (const gchar **keys)
{
  DasomKey **dasom_keys = g_malloc0_n (sizeof (DasomKey *), 1);

  gint i;
  for (i = 0; keys[i] != NULL; i++)
  {
    gchar **nicks = g_strsplit (keys[i], " ", -1);
    DasomKey *key = dasom_key_new_from_nicks ((const gchar **) nicks);
    g_strfreev (nicks);

    dasom_keys = g_realloc_n (dasom_keys, sizeof (DasomKey *), i + 2);
    dasom_keys[i] = key;
    dasom_keys[i + 1] = NULL;
  }

  return dasom_keys;
}

void
dasom_key_freev (DasomKey **keys)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (keys)
  {
    int i;
    for (i = 0; keys[i] != NULL; i++)
      dasom_key_free (keys[i]);

    g_free (keys);
  }
}

void
dasom_key_free (DasomKey *key)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (key != NULL);

  g_slice_free (DasomKey, key);
}
