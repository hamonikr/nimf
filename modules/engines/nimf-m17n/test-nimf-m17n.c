/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * test-nimf-m17n.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2019 Hodong Kim <cogniti@gmail.com>
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

#include <glib.h>
#include <m17n.h>
#include "nimf-m17n.h"

static gint
on_sort_by_group (gconstpointer a,
                  gconstpointer b)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  const NimfMethodInfo *info1 = *(NimfMethodInfo **) a;
  const NimfMethodInfo *info2 = *(NimfMethodInfo **) b;

  return g_strcmp0 (info1->group, info2->group);
}

static gint
on_sort_by_method_id (gconstpointer a,
                      gconstpointer b)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  const NimfMethodInfo *info1 = *(NimfMethodInfo **) a;
  const NimfMethodInfo *info2 = *(NimfMethodInfo **) b;

  return g_strcmp0 (info1->method_id, info2->method_id);
}

/* This method is very slow */
static NimfMethodInfo **
nimf_m17n_get_method_infos (const gchar *lang_code)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMethodInfo *info;
  MPlist *imlist, *pl;
  M17N_INIT();

  GPtrArray *array = g_ptr_array_new ();
  imlist = minput_list (Mnil);

  for (pl = imlist; mplist_key (pl) != Mnil; pl = mplist_next (pl))
  {
    MPlist *p = mplist_value (pl);
    MSymbol lang, name, sane;

    lang = mplist_value (p);
    p = mplist_next (p);
    name = mplist_value (p);
    p = mplist_next (p);
    sane = mplist_value (p);

    if (sane == Mt)
    {
      const gchar *code = msymbol_name (lang);

      if (lang_code && g_strcmp0 (code, lang_code))
        continue;

      info = nimf_method_info_new ();
      info->method_id = g_strdup_printf ("%s:%s", code, msymbol_name (name));
      info->label     = g_strdup_printf ("%s", msymbol_name (name));

      g_ptr_array_add (array, info);
    }
  }

  g_ptr_array_sort (array, on_sort_by_group);
  g_ptr_array_add (array, NULL);

  m17n_object_unref (imlist);
  M17N_FINI();

  return (NimfMethodInfo **) g_ptr_array_free (array, FALSE);
}

static void
test_nimf_m17n_available_languages ()
{
  NimfMethodInfo **infos;
  GHashTable *code_table;
  GHashTable *c_table;
  GHashTable *icon_table;
  gint i;
  GDir *dir;

  code_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  c_table    = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  icon_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  infos = nimf_m17n_get_method_infos (NULL);

  for (i = 0; infos[i]; i++)
  {
    gchar **strv;

    strv = g_strsplit (infos[i]->method_id, ":", 2);

    if (!M17N_CHECK_VERSION (1, 8, 0) && !g_strcmp0 (strv[0], "ua"))
      strv[0][1] = 'k';

    g_hash_table_add (code_table, g_strdup (strv[0]));

    g_strfreev (strv);
  }

  dir = g_dir_open (".", 0, NULL);
  const gchar *filename;

  while ((filename = g_dir_read_name (dir)))
  {
    if (g_str_has_prefix (filename, "nimf-m17n-") &&
        g_str_has_suffix (filename, ".c"))
    {
      gchar *code;

      if (!M17N_CHECK_VERSION (1, 8, 0) &&
          !g_strcmp0 (filename, "nimf-m17n-hu.c"))
        continue;

      g_print ("Check if %s available.\n", filename);
      code = g_strndup (strlen ("nimf-m17n-") + filename,
                        strlen (filename) - strlen (".c") - strlen ("nimf-m17n-"));
      g_hash_table_add (c_table, code);
      g_assert_true (g_hash_table_contains (code_table, code));
    }
  }

  g_dir_close (dir);

  dir = g_dir_open ("./icons/scalable", 0, NULL);
  while ((filename = g_dir_read_name (dir)))
  {
    if (g_str_has_prefix (filename, "nimf-m17n-") &&
        g_str_has_suffix (filename, ".svg"))
    {
      gchar *code;

      if (!M17N_CHECK_VERSION (1, 8, 0) &&
          !g_strcmp0 (filename, "nimf-m17n-hu.svg"))
        continue;

      g_print ("Check if %s is available.\n", filename);
      code = g_strndup (strlen ("nimf-m17n-") + filename,
                        strlen (filename) - strlen (".svg") - strlen ("nimf-m17n-"));
      g_hash_table_add (icon_table, code);
      g_assert_true (g_hash_table_contains (code_table, code));
    }
  }

  g_dir_close (dir);

  GHashTableIter iter;
  gpointer key;

  g_hash_table_iter_init (&iter, code_table);
  while (g_hash_table_iter_next (&iter, &key, NULL))
  {
    g_print ("Check if %s is exists.\n", (const gchar *) key);

    if (!g_strcmp0 (key, "en") ||
        !g_strcmp0 (key, "ja") ||
        !g_strcmp0 (key, "ko") ||
        !g_strcmp0 (key, "zh"))
      continue;

    g_assert_nonnull (g_hash_table_lookup (c_table, key));
    g_assert_nonnull (g_hash_table_lookup (icon_table, key));
  }

  g_hash_table_unref (code_table);
  g_hash_table_unref (c_table);
  g_hash_table_unref (icon_table);
  nimf_method_info_freev (infos);
}

static void
test_nimf_m17n_available_method_infos ()
{
  GDir *dir;
  const gchar *filename;
  NimfMethodInfo ** (* get_method_infos) ();

  dir = g_dir_open (".libs", 0, NULL);

  while ((filename = g_dir_read_name (dir)))
  {
    if (g_str_has_suffix (filename, ".so") && g_strcmp0 (filename, "libnimf-m17n.so"))
    {
      gchar           *code;
      NimfMethodInfo **infos1;
      NimfMethodInfo **infos2;
      GModule         *module;
      gchar           *path;
      gchar           *symname;
      gint             i;
      GPtrArray       *array1;
      GPtrArray       *array2;

      code = g_strndup (strlen ("libnimf-m17n-") + filename,
                        strlen (filename) - 3 - strlen ("libnimf-m17n-"));

      path   = g_build_path ("/", ".libs", filename, NULL);
      g_print ("Check method infos of %s\n", path);
      module = g_module_open (path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);

      symname = g_strdup_printf ("nimf_m17n_%s_get_method_infos", code);
      g_module_symbol (module, symname, (gpointer *) &get_method_infos);

      if (!M17N_CHECK_VERSION (1, 8, 0) && !g_strcmp0 (code, "uk"))
        code[1] = 'a';

      infos1 = nimf_m17n_get_method_infos (code);

      infos2 = get_method_infos ();
      array1 = g_ptr_array_new ();
      array2 = g_ptr_array_new ();

      for (i = 0; ; i++)
      {
        if (infos1[i] == NULL)
        {
          g_assert_null (infos2[i]);
          break;
        }

        g_assert_nonnull (infos2[i]);

        g_ptr_array_add (array1, infos1[i]);
        g_ptr_array_add (array2, infos2[i]);
      }

      g_ptr_array_sort (array1, on_sort_by_method_id);
      g_ptr_array_sort (array2, on_sort_by_method_id);
      g_ptr_array_add  (array1, NULL);
      g_ptr_array_add  (array2, NULL);

      g_free (infos1);
      g_free (infos2);

      infos1 = (NimfMethodInfo **) g_ptr_array_free (array1, FALSE);
      infos2 = (NimfMethodInfo **) g_ptr_array_free (array2, FALSE);

      for (i = 0; infos1[i]; i++)
      {
        g_print ("Check %s\n", infos1[i]->method_id);

        if (!M17N_CHECK_VERSION (1, 8, 0) && !g_strcmp0 (code, "ua"))
          infos1[i]->method_id[1] = 'k';

        g_assert_cmpstr (infos1[i]->method_id, ==, infos2[i]->method_id);
        g_assert_cmpstr (infos1[i]->label,     ==, infos2[i]->label);
        g_assert_cmpstr (infos1[i]->group,     ==, infos2[i]->group);
      }

      nimf_method_info_freev (infos1);
      nimf_method_info_freev (infos2);
      g_module_close (module);
      g_free (symname);
      g_free (path);
      g_free (code);
    }
  }

  g_dir_close (dir);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/nimf_m17n_available_languages/test",    test_nimf_m17n_available_languages);
  g_test_add_func ("/nimf_m17n_available_method_infos/test", test_nimf_m17n_available_method_infos);

  return g_test_run ();
}
