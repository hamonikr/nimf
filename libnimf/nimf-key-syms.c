/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-key-syms.c
 * This file is part of NIMF.
 *
 * Copyright (C) 2016 Hodong Kim <cogniti@gmail.com>
 *
 * NIMF is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NIMF is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program;  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nimf-key-syms.h"

/* Thanks to Markus G. Kuhn <mkuhn@acm.org> for the ksysym to Unicode
 * mapping function, from the keysym2ucs.c which is in the public domain.
 * nimf_keyval_to_unicode() is the same as gdk_keyval_to_unicode()
 * for compatibility.
 */
guint32
nimf_keyval_to_unicode (guint keyval)
{
  int min = 0;
  int max = G_N_ELEMENTS (nimf_keysym_to_unicode_table) - 1;
  int mid;

  /* First check for Latin-1 characters (1:1 mapping) */
  if ((keyval >= 0x0020 && keyval <= 0x007e) ||
      (keyval >= 0x00a0 && keyval <= 0x00ff))
    return keyval;

  /* Also check for directly encoded 24-bit UCS characters:
   */
  if ((keyval & 0xff000000) == 0x01000000)
    return keyval & 0x00ffffff;

  /* binary search in table */
  while (max >= min) {
    mid = (min + max) / 2;
    if (nimf_keysym_to_unicode_table[mid].keysym < keyval)
      min = mid + 1;
    else if (nimf_keysym_to_unicode_table[mid].keysym > keyval)
      max = mid - 1;
    else {
      /* found it */
      return nimf_keysym_to_unicode_table[mid].ucs;
    }
  }

  /* No matching Unicode value found */
  return 0;
}
