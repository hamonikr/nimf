/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-module-manager.c
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

#include "config.h"
#include "dasom-module-manager.h"
#include "dasom-module.h"
#include <gio/gio.h>

G_DEFINE_TYPE (DasomModuleManager, dasom_module_manager, G_TYPE_OBJECT);

GList *dasom_module_manager_create_instances (DasomModuleManager *manager)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *instances = NULL;

  GHashTableIter iter;
  gpointer value;

  g_hash_table_iter_init (&iter, manager->modules);
  while (g_hash_table_iter_next (&iter, NULL, &value))
  {
    DasomModule *module = value;
    instances = g_list_prepend (instances, g_object_new (module->type,
                                                         "path", module->path,
                                                         NULL));
  }

  return instances;
}

DasomModuleManager *
dasom_module_manager_get_default (void)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (DASOM_TYPE_MODULE_MANAGER, NULL);
}

static void
dasom_module_manager_load_module (DasomModuleManager *manager,
                                  const gchar        *path)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomModule *module;

  module = dasom_module_new (path);

  if (!g_type_module_use (G_TYPE_MODULE (module)))
  {
    g_warning (G_STRLOC ":" "Failed to load module: %s", path);
    g_object_unref (module);
    return;
  }

  g_hash_table_insert (manager->modules, g_strdup (path), module);

  g_type_module_unuse (G_TYPE_MODULE (module));
}

static void
dasom_module_manager_load_modules (DasomModuleManager *manager)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GDir *dir;
  GError *error = NULL;
  const gchar *filename;
  gchar *path;

  dir = g_dir_open (DASOM_MODULE_DIR, 0, &error);

  if (error)
  {
    g_warning (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
    g_clear_error (&error);
    return;
  }

  while ((filename = g_dir_read_name (dir)))
  {
    path = g_build_path (G_DIR_SEPARATOR_S, DASOM_MODULE_DIR, filename, NULL);
    dasom_module_manager_load_module (manager, path);
    g_free (path);
  }

  g_dir_close (dir);
}

static GObject *
dasom_module_manager_constructor (GType                  type,
                                  guint                  n_construct_params,
                                  GObjectConstructParam *construct_params)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  static GObject *object = NULL;
  GMutex singleton_mutex;

  g_mutex_init (&singleton_mutex);
  g_mutex_lock (&singleton_mutex);

  if (G_UNLIKELY (object == NULL))
  {
    GObjectClass *class = G_OBJECT_CLASS (dasom_module_manager_parent_class);
    object = class->constructor (type, n_construct_params, construct_params);
    g_object_add_weak_pointer (object, (gpointer) &object);
  }
  else
  {
    g_object_ref (object);
  }

  g_mutex_unlock (&singleton_mutex);

  return object;
}

static void
dasom_module_manager_init (DasomModuleManager *manager)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  manager->modules = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            NULL);
  dasom_module_manager_load_modules (manager);
}

static void
dasom_module_manager_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomModuleManager *manager = DASOM_MODULE_MANAGER (object);
  g_hash_table_unref (manager->modules);

  G_OBJECT_CLASS (dasom_module_manager_parent_class)->finalize (object);
}

static void
dasom_module_manager_class_init (DasomModuleManagerClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->constructor = dasom_module_manager_constructor;
  object_class->finalize    = dasom_module_manager_finalize;
}
