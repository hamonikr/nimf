/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-english.h
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

#ifndef __DASOM_ENGLISH_H__
#define __DASOM_ENGLISH_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>
#include "dasom-engine.h"
#include "dasom-key-syms.h"

G_BEGIN_DECLS

#define DASOM_TYPE_ENGLISH             (dasom_english_get_type ())
#define DASOM_ENGLISH(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_ENGLISH, DasomEnglish))
#define DASOM_ENGLISH_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DASOM_TYPE_ENGLISH, DasomEnglishClass))
#define DASOM_IS_ENGLISH(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_ENGLISH))
#define DASOM_IS_ENGLISH_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DASOM_TYPE_ENGLISH))
#define DASOM_ENGLISH_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_ENGLISH, DasomEnglishClass))

typedef struct _DasomEnglish      DasomEnglish;
typedef struct _DasomEnglishClass DasomEnglishClass;

struct _DasomEnglish
{
  DasomEngine parent_instance;
  gchar *id;
  gchar *name;
};

struct _DasomEnglishClass
{
  /*< private >*/
  DasomEngineClass parent_class;
};

GType    dasom_english_get_type     (void) G_GNUC_CONST;
gboolean dasom_english_filter_event (DasomEngine     *engine,
                                     DasomConnection *connection,
                                     DasomEvent      *event);

G_END_DECLS

#endif /* __DASOM_ENGLISH_H__ */
