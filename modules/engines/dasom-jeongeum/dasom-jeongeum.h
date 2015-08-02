/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-jeongeum.h
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

#ifndef __DASOM_JEONGEUM_H__
#define __DASOM_JEONGEUM_H__

#include <glib-object.h>
#include "dasom.h"
#include <hangul.h>

G_BEGIN_DECLS

#define DASOM_TYPE_JEONGEUM             (dasom_jeongeum_get_type ())
#define DASOM_JEONGEUM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_JEONGEUM, DasomJeongeum))
#define DASOM_JEONGEUM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DASOM_TYPE_JEONGEUM, DasomJeongeumClass))
#define DASOM_IS_JEONGEUM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_JEONGEUM))
#define DASOM_IS_JEONGEUM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DASOM_TYPE_JEONGEUM))
#define DASOM_JEONGEUM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_JEONGEUM, DasomJeongeumClass))

typedef struct _DasomJeongeum      DasomJeongeum;
typedef struct _DasomJeongeumClass DasomJeongeumClass;

struct _DasomJeongeum
{
  DasomEngine parent_instance;

  HangulInputContext *context;
  gchar              *preedit_string;
  gchar              *en_name;
  gchar              *ko_name;

  DasomCandidate     *candidate;
  gboolean            is_candidate_mode;
  gboolean            is_english_mode;
  HanjaTable         *hanja_table;
  DasomKey          **hangul_keys;
};

struct _DasomJeongeumClass
{
  /*< private >*/
  DasomEngineClass parent_class;
};

GType dasom_jeongeum_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __DASOM_JEONGEUM_H__ */
