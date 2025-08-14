/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-im.h
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

#ifndef __NIMF_IM_H__
#define __NIMF_IM_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-events.h"
#include "nimf-types.h"

G_BEGIN_DECLS

#define NIMF_TYPE_IM             (nimf_im_get_type ())
#define NIMF_IM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_IM, NimfIM))
#define NIMF_IM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_IM, NimfIMClass))
#define NIMF_IS_IM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_IM))
#define NIMF_IM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_IM, NimfIMClass))

typedef struct _NimfIM        NimfIM;
typedef struct _NimfIMClass   NimfIMClass;
typedef struct _NimfIMPrivate NimfIMPrivate;

struct _NimfIM
{
  GObject parent_instance;
  NimfIMPrivate *priv;
};

struct _NimfIMClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Signals */
  void     (*preedit_start)        (NimfIM *im);
  void     (*preedit_end)          (NimfIM *im);
  void     (*preedit_changed)      (NimfIM *im);
  void     (*commit)               (NimfIM *im, const gchar *text);
  gboolean (*retrieve_surrounding) (NimfIM *im);
  gboolean (*delete_surrounding)   (NimfIM *im,
                                    gint    offset,
                                    gint    n_chars);
  void     (*beep)                 (NimfIM *im);
};

GType     nimf_im_get_type                (void) G_GNUC_CONST;
NimfIM   *nimf_im_new                     (void);
void      nimf_im_focus_in                (NimfIM              *im);
void      nimf_im_focus_out               (NimfIM              *im);
void      nimf_im_reset                   (NimfIM              *im);
gboolean  nimf_im_filter_event            (NimfIM              *im,
                                           NimfEvent           *event);
void      nimf_im_get_preedit_string      (NimfIM              *im,
                                           gchar              **str,
                                           NimfPreeditAttr   ***attrs,
                                           gint                *cursor_pos);
void      nimf_im_set_cursor_location     (NimfIM              *im,
                                           const NimfRectangle *area);
void      nimf_im_set_use_preedit         (NimfIM              *im,
                                           gboolean             use_preedit);
void      nimf_im_set_surrounding         (NimfIM              *im,
                                           const char          *text,
                                           gint                 len,
                                           gint                 cursor_index);

G_END_DECLS

#endif /* __NIMF_IM_H__ */
