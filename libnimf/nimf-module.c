/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-module.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2019 Hodong Kim <cogniti@gmail.com>
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

#include "config.h"
#include "nimf-module-private.h"
#include <gio/gio.h>

G_DEFINE_TYPE (NimfModule, nimf_module, G_TYPE_TYPE_MODULE);

NimfModule *
nimf_module_new (const gchar *path)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (path != NULL, NULL);

  NimfModule *module = g_object_new (NIMF_TYPE_MODULE, NULL);

  module->path = g_strdup (path);

  return module;
}

static gboolean
nimf_module_load (GTypeModule *gmodule)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfModule *module = NIMF_MODULE (gmodule);
  void  (* module_register_type) (GTypeModule *module);
  GType (* module_get_type)      (void);

  module->library = g_module_open (module->path, G_MODULE_BIND_LAZY |
                                                 G_MODULE_BIND_LOCAL);

  if (!module->library)
  {
    g_warning (G_STRLOC ": %s", g_module_error ());
    return FALSE;
  }

  if (!g_module_symbol (module->library, "module_register_type",
                        (gpointer *) &module_register_type) ||
      !g_module_symbol (module->library, "module_get_type",
                        (gpointer *) &module_get_type))
  {
    g_warning (G_STRLOC ": %s", g_module_error ());
    g_module_close (module->library);

    return FALSE;
  }

  module_register_type (gmodule);
  module->type = module_get_type ();

  return TRUE;
}

static void
nimf_module_unload (GTypeModule *gmodule)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_module_close (NIMF_MODULE (gmodule)->library);
}

static void
nimf_module_init (NimfModule *module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_module_class_init (NimfModuleClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS (class);

  module_class->load   = nimf_module_load;
  module_class->unload = nimf_module_unload;
}
