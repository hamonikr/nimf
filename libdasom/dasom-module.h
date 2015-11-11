/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-module.h
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

#ifndef __DASOM_MODULE_H__
#define __DASOM_MODULE_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>
#include <gmodule.h>
#include "dasom-engine.h"

G_BEGIN_DECLS

#define DASOM_TYPE_MODULE             (dasom_module_get_type ())
#define DASOM_MODULE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_MODULE, DasomModule))
#define DASOM_MODULE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DASOM_TYPE_MODULE, DasomModuleClass))
#define DASOM_IS_MODULE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_MODULE))
#define DASOM_IS_MODULE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DASOM_TYPE_MODULE))
#define DASOM_MODULE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_MODULE, DasomModuleClass))

typedef struct _DasomModule      DasomModule;
typedef struct _DasomModuleClass DasomModuleClass;

struct _DasomModule
{
  GTypeModule parent_instance;

  char     *path;
  GModule  *library;
  GType     type;

  void  (* load)     (GTypeModule *module);
  GType (* get_type) (void);
  void  (* unload)   (void);
};

struct _DasomModuleClass
{
  GTypeModuleClass parent_class;
};

GType        dasom_module_get_type (void) G_GNUC_CONST;
DasomModule *dasom_module_new      (const gchar *path);

G_END_DECLS

#endif /* __DASOM_MODULE_H__ */
