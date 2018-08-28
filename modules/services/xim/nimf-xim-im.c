/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-xim-im.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015-2018 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-xim-im.h"
#include <X11/Xutil.h>

G_DEFINE_TYPE (NimfXimIM, nimf_xim_im, NIMF_TYPE_SERVICE_IM);

static void
nimf_xim_im_emit_commit (NimfServiceIM *im,
                         const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXimIM *xim_im = NIMF_XIM_IM (im);
  XTextProperty property;
  Xutf8TextListToTextProperty (xim_im->xim->xims->core.display,
                               (char **)&text, 1, XCompoundTextStyle,
                               &property);

  IMCommitStruct commit_data = {0};
  commit_data.major_code = XIM_COMMIT;
  commit_data.connect_id = xim_im->connect_id;
  commit_data.icid       = im->icid;
  commit_data.flag       = XimLookupChars;
  commit_data.commit_string = (gchar *) property.value;
  IMCommitString (xim_im->xim->xims, (XPointer) &commit_data);

  XFree (property.value);
}

static void nimf_xim_im_emit_preedit_end (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXimIM *xim_im = NIMF_XIM_IM (im);

  IMPreeditStateStruct preedit_state_data = {0};

  preedit_state_data.connect_id = xim_im->connect_id;
  preedit_state_data.icid       = im->icid;
  IMPreeditEnd (xim_im->xim->xims, (XPointer) &preedit_state_data);

  IMPreeditCBStruct preedit_cb_data = {0};
  preedit_cb_data.major_code = XIM_PREEDIT_DONE;
  preedit_cb_data.connect_id = xim_im->connect_id;
  preedit_cb_data.icid       = im->icid;
  IMCallCallback (xim_im->xim->xims, (XPointer) &preedit_cb_data);
}

static void nimf_xim_im_emit_preedit_start (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXimIM *xim_im = NIMF_XIM_IM (im);
  IMPreeditStateStruct preedit_state_data = {0};

  preedit_state_data.connect_id = xim_im->connect_id;
  preedit_state_data.icid       = im->icid;
  IMPreeditStart (xim_im->xim->xims, (XPointer) &preedit_state_data);
  IMPreeditCBStruct preedit_cb_data = {0};
  preedit_cb_data.major_code = XIM_PREEDIT_START;
  preedit_cb_data.connect_id = xim_im->connect_id;
  preedit_cb_data.icid       = im->icid;
  IMCallCallback (xim_im->xim->xims, (XPointer) &preedit_cb_data);
}

static void
nimf_xim_im_emit_preedit_changed (NimfServiceIM    *im,
                                  const gchar      *preedit_string,
                                  NimfPreeditAttr **attrs,
                                  gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXimIM *xim_im = NIMF_XIM_IM (im);

  IMPreeditCBStruct preedit_cb_data = {0};
  XIMText           text;
  XTextProperty     text_property;

  static XIMFeedback *feedback;
  gint i, j, len;

  len = g_utf8_strlen (preedit_string, -1);
  feedback = g_malloc0 (sizeof (XIMFeedback) * (len + 1));

  for (i = 0; attrs[i]; i++)
  {
    switch (attrs[i]->type)
    {
      case NIMF_PREEDIT_ATTR_HIGHLIGHT:
        for (j = attrs[i]->start_index; j < attrs[i]->end_index; j++)
          feedback[j] |= XIMHighlight;
        break;
      case NIMF_PREEDIT_ATTR_UNDERLINE:
        for (j = attrs[i]->start_index; j < attrs[i]->end_index; j++)
          feedback[j] |= XIMUnderline;
        break;
      default:
        break;
    }
  }

  feedback[len] = 0;

  preedit_cb_data.major_code = XIM_PREEDIT_DRAW;
  preedit_cb_data.connect_id = xim_im->connect_id;
  preedit_cb_data.icid = im->icid;
  preedit_cb_data.todo.draw.caret = len;
  preedit_cb_data.todo.draw.chg_first = 0;
  preedit_cb_data.todo.draw.chg_length = xim_im->preedit_length;
  preedit_cb_data.todo.draw.text = &text;

  text.feedback = feedback;

  if (len > 0)
  {
    Xutf8TextListToTextProperty (xim_im->xim->xims->core.display,
                                 (char **) &preedit_string, 1,
                                 XCompoundTextStyle, &text_property);
    text.encoding_is_wchar = 0;
    text.length = strlen ((char *) text_property.value);
    text.string.multi_byte = (char *) text_property.value;
    IMCallCallback (xim_im->xim->xims, (XPointer) &preedit_cb_data);
    XFree (text_property.value);
  }
  else
  {
    text.encoding_is_wchar = 0;
    text.length = 0;
    text.string.multi_byte = "";
    IMCallCallback (xim_im->xim->xims, (XPointer) &preedit_cb_data);
    len = 0;
  }

  xim_im->preedit_length = len;

  g_free (feedback);
}

static void
on_changed_draw_preedit_on_the_server_side (GSettings *settings,
                                            gchar     *key,
                                            NimfXimIM *xim_im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  xim_im->draw_preedit_on_the_server_side =
    g_settings_get_boolean (xim_im->xim->settings, key);

  if (xim_im->draw_preedit_on_the_server_side || !xim_im->input_style)
    nimf_service_im_set_use_preedit (NIMF_SERVICE_IM (xim_im), FALSE);
}

NimfXimIM *nimf_xim_im_new (NimfServer *server,
                            NimfXim    *xim)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXimIM *xim_im;

  xim_im = g_object_new (NIMF_TYPE_XIM_IM, "server", server, NULL);
  xim_im->xim = xim;

  xim_im->draw_preedit_on_the_server_side =
    g_settings_get_boolean (xim->settings, "draw-preedit-on-the-server-side");

  g_signal_connect (xim->settings, "changed::draw-preedit-on-the-server-side",
                    G_CALLBACK (on_changed_draw_preedit_on_the_server_side),
                    xim_im);
  return xim_im;
}

static void
nimf_xim_im_init (NimfXimIM *nimf_xim_im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_xim_im_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  G_OBJECT_CLASS (nimf_xim_im_parent_class)->finalize (object);
}

static void
nimf_xim_im_class_init (NimfXimIMClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass       *object_class     = G_OBJECT_CLASS (class);
  NimfServiceIMClass *service_im_class = NIMF_SERVICE_IM_CLASS (class);

  service_im_class->emit_commit          = nimf_xim_im_emit_commit;
  service_im_class->emit_preedit_start   = nimf_xim_im_emit_preedit_start;
  service_im_class->emit_preedit_changed = nimf_xim_im_emit_preedit_changed;
  service_im_class->emit_preedit_end     = nimf_xim_im_emit_preedit_end;

  object_class->finalize = nimf_xim_im_finalize;
}
