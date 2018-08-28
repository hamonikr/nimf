/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-types.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2018 Hodong Kim <cogniti@gmail.com>
 *
 * # 법적 고지
 *
 * Nimf 소프트웨어는 대한민국 저작권법과 국제 조약의 보호를 받습니다.
 * Nimf 개발자는 대한민국 법률의 보호를 받습니다.
 * 커뮤니티의 위력을 이용하여 개발자의 시간과 노동력을 약탈하려는 행위를 금하시기 바랍니다.
 *
 * * 커뮤니티 게시판에 개발자를 욕(비난)하거나
 * * 욕보이는(음해하는) 글을 작성하거나
 * * 허위 사실을 공표하거나
 * * 명예를 훼손하는
 *
 * 등의 행위는 정보통신망 이용촉진 및 정보보호 등에 관한 법률의 제재를 받습니다.
 *
 * # 면책 조항
 *
 * Nimf 는 무료로 배포되는 오픈소스 소프트웨어입니다.
 * Nimf 개발자는 개발 및 유지보수에 대해 어떠한 의무도 없고 어떠한 책임도 없습니다.
 * 어떠한 경우에도 보증하지 않습니다. 도덕적 보증 책임도 없고, 도의적 보증 책임도 없습니다.
 * Nimf 개발자는 리브레오피스, 이클립스 등 귀하가 사용하시는 소프트웨어의 버그를 해결해야 할 의무가 없습니다.
 * Nimf 개발자는 귀하가 사용하시는 배포판에 대해 기술 지원을 해드려야 할 의무가 없습니다.
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

#include "nimf-types.h"
#include "nimf-enum-types.h"

G_DEFINE_QUARK (nimf-error-quark, nimf_error)

NimfKey *
nimf_key_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_slice_new0 (NimfKey);
}

NimfKey *
nimf_key_new_from_nicks (const gchar **nicks)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfKey     *key = g_slice_new0 (NimfKey);
  GEnumValue  *enum_value;  /* Do not free */
  GFlagsValue *flags_value; /* Do not free */
  GFlagsClass *flags_class = g_type_class_ref (NIMF_TYPE_MODIFIER_TYPE);
  GEnumClass  *enum_class  = g_type_class_ref (NIMF_TYPE_KEY_SYM);

  gint i;
  for (i = 0; nicks[i] != NULL; i++)
  {
    if (g_str_has_prefix (nicks[i], "<"))
    {
      flags_value = g_flags_get_value_by_nick (flags_class, nicks[i]);

      if (flags_value)
        key->mods |= flags_value->value;
      else
        g_warning ("NimfModifierType doesn't have a member with that nickname: %s", nicks[i]);
    }
    else
    {
      enum_value = g_enum_get_value_by_nick (enum_class, nicks[i]);

      if (enum_value)
        key->keyval = enum_value->value;
      else
        g_warning ("NimfKeySym doesn't have a member with that nickname: %s", nicks[i]);
    }
  }

  g_type_class_unref (flags_class);
  g_type_class_unref (enum_class);

  return key;
}

NimfKey **nimf_key_newv (const gchar **keys)
{
  NimfKey **nimf_keys = g_malloc0_n (1, sizeof (NimfKey *));

  gint i;
  for (i = 0; keys[i] != NULL; i++)
  {
    gchar **nicks = g_strsplit (keys[i], " ", -1);
    NimfKey *key = nimf_key_new_from_nicks ((const gchar **) nicks);
    g_strfreev (nicks);

    nimf_keys = g_realloc_n (nimf_keys, sizeof (NimfKey *), i + 2);
    nimf_keys[i] = key;
    nimf_keys[i + 1] = NULL;
  }

  return nimf_keys;
}

void
nimf_key_freev (NimfKey **keys)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (keys)
  {
    int i;
    for (i = 0; keys[i] != NULL; i++)
      nimf_key_free (keys[i]);

    g_free (keys);
  }
}

void
nimf_key_free (NimfKey *key)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (key != NULL);

  g_slice_free (NimfKey, key);
}

NimfPreeditAttr *nimf_preedit_attr_new (NimfPreeditAttrType type,
                                        guint               start_index,
                                        guint               end_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditAttr *attr;

  attr = g_new (NimfPreeditAttr, 1);
  attr->type        = type;
  attr->start_index = start_index;
  attr->end_index   = end_index;

  return attr;
}

NimfPreeditAttr **nimf_preedit_attrs_copy (NimfPreeditAttr **attrs)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfPreeditAttr **preedit_attrs;
  gint              i;

  g_return_val_if_fail (attrs, NULL);

  preedit_attrs = g_malloc0_n (1, sizeof (NimfPreeditAttr *));

  for (i = 0; attrs[i] != NULL; i++)
  {
    preedit_attrs = g_realloc_n (preedit_attrs, 1 + i + 1, sizeof (NimfPreeditAttr *));
    preedit_attrs[i] = g_memdup (attrs[i], sizeof (NimfPreeditAttr));
    preedit_attrs[i + 1] = NULL;
  }

  return preedit_attrs;
}

void nimf_preedit_attr_free (NimfPreeditAttr *attr)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_free (attr);
}

void nimf_preedit_attr_freev (NimfPreeditAttr **attrs)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (attrs)
  {
    int i;
    for (i = 0; attrs[i] != NULL; i++)
      nimf_preedit_attr_free (attrs[i]);

    g_free (attrs);
  }
}
