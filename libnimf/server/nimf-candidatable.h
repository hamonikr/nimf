/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-candidatable.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2018 Hodong Kim <cogniti@gmail.com>
 *
 * # 법적 고지
 *
 * Nimf 소프트웨어는 대한민국 저작권법과 국제 조약의 보호를 받습니다.
 * Nimf 개발자는 대한민국 법률의 보호를 받습니다.
 * 커뮤니티의 위력을 이용하여 개발자의 시간과 노동력을 약탈하려는 행위를 금하시기 바랍니다.
 *
 * * 커뮤니티 게시판에 개발자를 욕(비난)하거나
 * * 욕보이는(음해하는) 글을 작성하거나
 * * 허위 사실을 공표하거나
 * * 명예를 훼손하는
 *
 * 등의 행위는 정보통신망 이용촉진 및 정보보호 등에 관한 법률의 제재를 받습니다.
 *
 * # 면책 조항
 *
 * Nimf 는 무료로 배포되는 오픈소스 소프트웨어입니다.
 * Nimf 개발자는 개발 및 유지보수에 대해 어떠한 의무도 없고 어떠한 책임도 없습니다.
 * 어떠한 경우에도 보증하지 않습니다. 도덕적 보증 책임도 없고, 도의적 보증 책임도 없습니다.
 * Nimf 개발자는 리브레오피스, 이클립스 등 귀하가 사용하시는 소프트웨어의 버그를 해결해야 할 의무가 없습니다.
 * Nimf 개발자는 귀하가 사용하시는 배포판에 대해 기술 지원을 해드려야 할 의무가 없습니다.
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

#ifndef __NIMF_CANDIDATABLE_H__
#define __NIMF_CANDIDATABLE_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>
#include "nimf-service-im.h"

G_BEGIN_DECLS

#define NIMF_TYPE_CANDIDATABLE nimf_candidatable_get_type ()

G_DECLARE_INTERFACE (NimfCandidatable, nimf_candidatable, NIMF, CANDIDATABLE, GObject)

typedef struct _NimfServiceIM NimfServiceIM;

struct _NimfCandidatableInterface
{
  GTypeInterface parent;

  void     (* show)                         (NimfCandidatable *candidatable,
                                             NimfServiceIM    *target,
                                             gboolean          show_entry);
  void     (* hide)                         (NimfCandidatable *candidatable);
  gboolean (* is_visible)                   (NimfCandidatable *candidatable);
  void     (* clear)                        (NimfCandidatable *candidatable,
                                             NimfServiceIM    *target);
  void     (* set_page_values)              (NimfCandidatable *candidatable,
                                             NimfServiceIM    *target,
                                             gint              page_index,
                                             gint              n_pages,
                                             gint              page_size);
  void     (* append)                       (NimfCandidatable *candidatable,
                                             const gchar      *item1,
                                             const gchar      *item2);
  gint     (* get_selected_index)           (NimfCandidatable *candidatable);
  gchar *  (* get_selected_text)            (NimfCandidatable *candidatable);
  void     (* select_first_item_in_page)    (NimfCandidatable *candidatable);
  void     (* select_last_item_in_page)     (NimfCandidatable *candidatable);
  void     (* select_item_by_index_in_page) (NimfCandidatable *candidatable,
                                             gint              index);
  void     (* select_previous_item)         (NimfCandidatable *candidatable);
  void     (* select_next_item)             (NimfCandidatable *candidatable);
  void     (* set_auxiliary_text)           (NimfCandidatable *candidatable,
                                             const gchar      *text,
                                             gint              cursor_pos);
};

void     nimf_candidatable_show            (NimfCandidatable *candidatable,
                                            NimfServiceIM    *target,
                                            gboolean          show_entry);
void     nimf_candidatable_hide            (NimfCandidatable *candidatable);
gboolean nimf_candidatable_is_visible      (NimfCandidatable *candidatable);
void     nimf_candidatable_clear           (NimfCandidatable *candidatable,
                                            NimfServiceIM    *target);
void     nimf_candidatable_set_page_values (NimfCandidatable *candidatable,
                                            NimfServiceIM    *target,
                                            gint              page_index,
                                            gint              n_pages,
                                            gint              page_size);
void     nimf_candidatable_append          (NimfCandidatable *candidatable,
                                            const gchar      *item1,
                                            const gchar      *item2);
gint     nimf_candidatable_get_selected_index
                                           (NimfCandidatable *candidatable);
gchar   *nimf_candidatable_get_selected_text
                                           (NimfCandidatable *candidatable);
void     nimf_candidatable_select_first_item_in_page
                                           (NimfCandidatable *candidatable);
void     nimf_candidatable_select_last_item_in_page
                                           (NimfCandidatable *candidatable);
void     nimf_candidatable_select_item_by_index_in_page
                                           (NimfCandidatable *candidatable,
                                            gint              index);
void     nimf_candidatable_select_previous_item
                                           (NimfCandidatable *candidatable);
void     nimf_candidatable_select_next_item
                                           (NimfCandidatable *candidatable);
void     nimf_candidatable_set_auxiliary_text
                                           (NimfCandidatable *candidatable,
                                            const gchar      *text,
                                            gint              cursor_pos);
G_END_DECLS

#endif /* __NIMF_CANDIDATABLE_H__ */
