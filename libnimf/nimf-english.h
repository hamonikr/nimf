/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-english.h
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

#ifndef __NIMF_ENGLISH_H__
#define __NIMF_ENGLISH_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-engine.h"

G_BEGIN_DECLS

#define NIMF_TYPE_ENGLISH             (nimf_english_get_type ())
#define NIMF_ENGLISH(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_ENGLISH, NimfEnglish))
#define NIMF_ENGLISH_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_ENGLISH, NimfEnglishClass))
#define NIMF_IS_ENGLISH(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_ENGLISH))
#define NIMF_IS_ENGLISH_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_ENGLISH))
#define NIMF_ENGLISH_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_ENGLISH, NimfEnglishClass))

typedef struct _NimfEnglish      NimfEnglish;
typedef struct _NimfEnglishClass NimfEnglishClass;

struct _NimfEnglish
{
  NimfEngine parent_instance;

  gchar *id;
  gchar *name;
};

struct _NimfEnglishClass
{
  /*< private >*/
  NimfEngineClass parent_class;
};

GType    nimf_english_get_type     (void) G_GNUC_CONST;
gboolean nimf_english_filter_event (NimfEngine     *engine,
                                    NimfConnection *connection,
                                    NimfEvent      *event);
G_END_DECLS

#endif /* __NIMF_ENGLISH_H__ */
