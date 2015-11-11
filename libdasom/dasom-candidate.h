/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-candidate.h
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

#ifndef __DASOM_CANDIDATE_H__
#define __DASOM_CANDIDATE_H__

#if !defined (__DASOM_H_INSIDE__) && !defined (DASOM_COMPILATION)
#error "Only <dasom.h> can be included directly."
#endif

#include <glib-object.h>
#include <gtk/gtk.h>
#include "dasom-connection.h"

G_BEGIN_DECLS

#define DASOM_TYPE_CANDIDATE             (dasom_candidate_get_type ())
#define DASOM_CANDIDATE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASOM_TYPE_CANDIDATE, DasomCandidate))
#define DASOM_CANDIDATE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DASOM_TYPE_CANDIDATE, DasomCandidateClass))
#define DASOM_IS_CANDIDATE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASOM_TYPE_CANDIDATE))
#define DASOM_IS_CANDIDATE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DASOM_TYPE_CANDIDATE))
#define DASOM_CANDIDATE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASOM_TYPE_CANDIDATE, DasomCandidateClass))

typedef struct _DasomConnection      DasomConnection;

typedef struct _DasomCandidate       DasomCandidate;
typedef struct _DasomCandidateClass  DasomCandidateClass;

struct _DasomCandidate
{
  GObject parent_instance;

  GtkWidget       *window;
  GtkWidget       *treeview;
  GtkTreeIter      iter;
  DasomConnection *target;
};

struct _DasomCandidateClass
{
  GObjectClass parent_class;
};

GType dasom_candidate_get_type (void) G_GNUC_CONST;

DasomCandidate *dasom_candidate_new                   (void);
void            dasom_candidate_update_window         (DasomCandidate  *candidate,
                                                       const gchar    **strv);
void            dasom_candidate_show_window           (DasomCandidate  *candidate,
                                                       DasomConnection *target);
void            dasom_candidate_hide_window           (DasomCandidate  *candidate);
void            dasom_candidate_select_previous_item  (DasomCandidate  *candidate);
void            dasom_candidate_select_next_item      (DasomCandidate  *candidate);
void            dasom_candidate_select_page_up_item   (DasomCandidate  *candidate);
void            dasom_candidate_select_page_down_item (DasomCandidate  *candidate);
gchar          *dasom_candidate_get_selected_text     (DasomCandidate  *candidate);

G_END_DECLS

#endif /* __DASOM_CANDIDATE_H__ */

