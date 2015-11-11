/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-module-manager.h
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

#ifndef __DASOM_MODULE_MANAGER_H__
#define __DASOM_MODULE_MANAGER_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define DASOM_TYPE_MODULE_MANAGER             (dasom_module_manager_get_type ())
#define DASOM_MODULE_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_MODULE_MANAGER, DasomModuleManager))
#define DASOM_MODULE_MANAGER_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), DASOM_TYPE_MODULE_MANAGER, DasomModuleManagerClass))
#define DASOM_IS_MODULE_MANAGER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_MODULE_MANAGER))
#define DASOM_IS_MODULE_MANAGER_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), DASOM_TYPE_MODULE_MANAGER))
#define DASOM_MODULE_MANAGER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_MODULE_MANAGER, DasomModuleManagerClass))

typedef struct _DasomModuleManager      DasomModuleManager;
typedef struct _DasomModuleManagerClass DasomModuleManagerClass;

struct _DasomModuleManager
{
  GObject parent_instance;

  GHashTable *modules;
};

struct _DasomModuleManagerClass
{
  GObjectClass parent_class;
};

GType               dasom_module_manager_get_type         (void) G_GNUC_CONST;
DasomModuleManager *dasom_module_manager_get_default      (void);

G_END_DECLS

#endif /* __DASOM_MODULE_MANAGER_H__ */
