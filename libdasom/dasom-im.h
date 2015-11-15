/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-im.h
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

#ifndef __DASOM_IM_H__
#define __DASOM_IM_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>
#include <gio/gio.h>
#include "dasom-events.h"
#include "dasom-engine.h"
#include "dasom-message.h"
#include "dasom-private.h"

G_BEGIN_DECLS

#define DASOM_TYPE_IM             (dasom_im_get_type ())
#define DASOM_IM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_IM, DasomIM))
#define DASOM_IM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DASOM_TYPE_IM, DasomIMClass))
#define DASOM_IS_IM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_IM))
#define DASOM_IS_IM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DASOM_TYPE_IM))
#define DASOM_IM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_IM, DasomIMClass))

typedef struct _DasomIM      DasomIM;
typedef struct _DasomIMClass DasomIMClass;

struct _DasomIM
{
  GObject parent_instance;

  DasomEngine       *engine;
  GSocketConnection *connection;
  DasomResult       *result;
  GSource           *sockets_context_source;
  GSource           *default_context_source;
};

struct _DasomIMClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Signals */
  void     (*preedit_start)        (DasomIM *im);
  void     (*preedit_end)          (DasomIM *im);
  void     (*preedit_changed)      (DasomIM *im);
  void     (*commit)               (DasomIM *im, const gchar *str);
  gboolean (*retrieve_surrounding) (DasomIM *im);
  gboolean (*delete_surrounding)   (DasomIM *im,
                                    gint     offset,
                                    gint     n_chars);
};

GType     dasom_im_get_type            (void) G_GNUC_CONST;
DasomIM  *dasom_im_new                 (void);
void      dasom_im_focus_in            (DasomIM           *im);
void      dasom_im_focus_out           (DasomIM           *im);
void      dasom_im_reset               (DasomIM           *im);
gboolean  dasom_im_filter_event        (DasomIM           *im,
                                        DasomEvent        *event);
void      dasom_im_get_preedit_string  (DasomIM           *im,
                                        gchar            **str,
                                        gint              *cursor_pos);
void      dasom_im_set_cursor_location (DasomIM              *im,
                                        const DasomRectangle *area);
void      dasom_im_set_use_preedit     (DasomIM           *im,
                                        gboolean           use_preedit);
gboolean  dasom_im_get_surrounding     (DasomIM           *im,
                                        gchar            **text,
                                        gint              *cursor_index);
void      dasom_im_set_surrounding     (DasomIM           *im,
                                        const char        *text,
                                        gint               len,
                                        gint               cursor_index);

G_END_DECLS

#endif /* __DASOM_IM_H__ */
