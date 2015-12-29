/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-module-manager.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2015 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_MODULE_MANAGER_H__
#define __NIMF_MODULE_MANAGER_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define NIMF_TYPE_MODULE_MANAGER             (nimf_module_manager_get_type ())
#define NIMF_MODULE_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_MODULE_MANAGER, NimfModuleManager))
#define NIMF_MODULE_MANAGER_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_MODULE_MANAGER, NimfModuleManagerClass))
#define NIMF_IS_MODULE_MANAGER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_MODULE_MANAGER))
#define NIMF_IS_MODULE_MANAGER_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_MODULE_MANAGER))
#define NIMF_MODULE_MANAGER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_MODULE_MANAGER, NimfModuleManagerClass))

typedef struct _NimfModuleManager      NimfModuleManager;
typedef struct _NimfModuleManagerClass NimfModuleManagerClass;

struct _NimfModuleManager
{
  GObject parent_instance;

  GHashTable *modules;
};

struct _NimfModuleManagerClass
{
  GObjectClass parent_class;
};

GType              nimf_module_manager_get_type    (void) G_GNUC_CONST;
NimfModuleManager *nimf_module_manager_get_default (void);

G_END_DECLS

#endif /* __NIMF_MODULE_MANAGER_H__ */
