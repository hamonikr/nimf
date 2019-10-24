/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-module-private.h
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

#ifndef __NIMF_MODULE_H__
#define __NIMF_MODULE_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include <gmodule.h>
#include "nimf-engine.h"

G_BEGIN_DECLS

#define NIMF_TYPE_MODULE             (nimf_module_get_type ())
#define NIMF_MODULE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_MODULE, NimfModule))
#define NIMF_MODULE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_MODULE, NimfModuleClass))
#define NIMF_IS_MODULE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_MODULE))
#define NIMF_IS_MODULE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_MODULE))
#define NIMF_MODULE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_MODULE, NimfModuleClass))

typedef struct _NimfModule      NimfModule;
typedef struct _NimfModuleClass NimfModuleClass;

struct _NimfModule
{
  GTypeModule parent_instance;

  gchar   *path;
  GModule *library;
  GType    type;
};

struct _NimfModuleClass
{
  GTypeModuleClass parent_class;
};

GType       nimf_module_get_type (void) G_GNUC_CONST;
NimfModule *nimf_module_new      (const gchar *path);

G_END_DECLS

#endif /* __NIMF_MODULE_H__ */
