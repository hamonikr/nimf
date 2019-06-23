/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-m17n.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2019 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_M17N_H__
#define __NIMF_M17N_H__

#include <glib-object.h>
#include "nimf-engine.h"
#include <gio/gio.h>
#include <m17n.h>

G_BEGIN_DECLS

#define NIMF_TYPE_M17N              (nimf_m17n_get_type ())
#define NIMF_M17N(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), NIMF_TYPE_M17N, NimfM17n))
#define NIMF_M17N_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_M17N, NimfM17nClass))
#define NIMF_IS_M17N(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), NIMF_TYPE_M17N))
#define NIMF_IS_M17N_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE ((class), NIMF_TYPE_M17N))
#define NIMF_M17N_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), NIMF_TYPE_M17N, NimfM17nClass))

#define M17N_CHECK_VERSION(major,minor,micro) \
    (M17NLIB_MAJOR_VERSION > (major) || \
     (M17NLIB_MAJOR_VERSION == (major) && M17NLIB_MINOR_VERSION > (minor)) || \
     (M17NLIB_MAJOR_VERSION == (major) && M17NLIB_MINOR_VERSION == (minor) && \
      M17NLIB_PATCH_LEVEL >= (micro)))

typedef struct _NimfM17n      NimfM17n;
typedef struct _NimfM17nClass NimfM17nClass;

struct _NimfM17n
{
  NimfEngine parent_instance;

  NimfCandidatable  *candidatable;
  gchar             *id;
  GSettings         *settings;
  gchar             *method;
  MInputMethod      *im;
  MInputContext     *ic;
  MConverter        *converter;
  gchar             *preedit;
  NimfPreeditState   preedit_state;
  NimfPreeditAttr  **preedit_attrs;
  gint               current_page;
  gint               n_pages;
};

struct _NimfM17nClass
{
  /*< private >*/
  NimfEngineClass parent_class;
};

GType            nimf_m17n_get_type         (void) G_GNUC_CONST;
void             nimf_m17n_open_im          (NimfM17n      *m17n);
void             on_changed_method          (GSettings     *settings,
                                             gchar         *key,
                                             NimfM17n      *m17n);

G_END_DECLS

#endif /* __NIMF_M17N_H__ */
