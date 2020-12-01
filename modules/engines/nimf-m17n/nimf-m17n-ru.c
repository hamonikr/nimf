/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-m17n-ru.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2019,2020 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-m17n.h"

#define NIMF_TYPE_M17N_RU              (nimf_m17n_ru_get_type ())
#define NIMF_M17N_RU(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), NIMF_TYPE_M17N_RU, NimfM17nRu))
#define NIMF_M17N_RU_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_M17N_RU, NimfM17nRuClass))
#define NIMF_IS_M17N_RU(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), NIMF_TYPE_M17N_RU))
#define NIMF_IS_M17N_RU_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_M17N_RU))
#define NIMF_M17N_RU_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), NIMF_TYPE_M17N_RU, NimfM17nRuClass))

typedef struct _NimfM17nRu      NimfM17nRu;
typedef struct _NimfM17nRuClass NimfM17nRuClass;

struct _NimfM17nRu
{
  NimfM17n parent_instance;
};

struct _NimfM17nRuClass
{
  /*< private >*/
  NimfM17nClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE (NimfM17nRu, nimf_m17n_ru, NIMF_TYPE_M17N);

static void
nimf_m17n_ru_init (NimfM17nRu *ru)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfM17n *m17n = NIMF_M17N (ru);

  m17n->id       = g_strdup ("nimf-m17n-ru");
  m17n->settings = g_settings_new ("org.nimf.engines.nimf-m17n-ru");
  m17n->method   = g_settings_get_string (m17n->settings, "get-method-infos");
  m17n->preedit_attrs = g_malloc_n (2, sizeof (NimfPreeditAttr *));

  nimf_m17n_open_im (m17n);

  g_signal_connect_data (m17n->settings, "changed::get-method-infos",
                  G_CALLBACK (on_changed_method), m17n, NULL, G_CONNECT_AFTER);
}

static void
nimf_m17n_ru_class_init (NimfM17nRuClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_m17n_ru_class_finalize (NimfM17nRuClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

NimfMethodInfo **
nimf_m17n_ru_get_method_infos ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMethodInfo *info;
  GPtrArray      *array;
  gchar          *methods[] = {"phonetic", "kbd", "yawerty", "translit", NULL};
  gint            i;

  array = g_ptr_array_new ();

  for (i = 0; methods[i]; i++)
  {
    info = nimf_method_info_new ();
    info->method_id = g_strdup_printf ("ru:%s", methods[i]);
    info->label     = g_strdup (methods[i]);
    g_ptr_array_add (array, info);
  }

  g_ptr_array_add (array, NULL);

  return (NimfMethodInfo **) g_ptr_array_free (array, FALSE);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_m17n_ru_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_m17n_ru_get_type ();
}
