/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-xim-ic.c
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

#include "nimf-xim-ic.h"
#include <X11/Xutil.h>
#include "IMdkit/i18nMethod.h"

G_DEFINE_TYPE (NimfXimIC, nimf_xim_ic, NIMF_TYPE_SERVICE_IC);

void nimf_xim_ic_set_cursor_location (NimfXimIC *xic,
                                      gint       x,
                                      gint       y)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfRectangle area = {0};
  Window        window;

  if (xic->focus_window)
    window = xic->focus_window;
  else
    window = xic->client_window;

  if (window)
  {
    Window             child;
    XWindowAttributes  attr;
    const char        *xft_dpi;
    int dpi   = 0;
    int dpi_x = 0;
    int dpi_y = 0;

    XGetWindowAttributes (xic->xim->display, window, &attr);

    if (x < 0 && y < 0)
      XTranslateCoordinates (xic->xim->display, window, attr.root,
                             0, attr.height, &area.x, &area.y, &child);
    else
      XTranslateCoordinates (xic->xim->display, window, attr.root,
                             x, y, &area.x, &area.y, &child);

    xft_dpi = XGetDefault (xic->xim->display, "Xft", "dpi");

    if (xft_dpi)
      dpi = atoi (xft_dpi);

    if (dpi)
    {
      area.x = (double) area.x * (96.0 / dpi);
      area.y = (double) area.y * (96.0 / dpi);
    }
    else
    {
      dpi_x = ((double) WidthOfScreen    (attr.screen)) * 25.4 /
              ((double) WidthMMOfScreen  (attr.screen));
      dpi_y = ((double) HeightOfScreen   (attr.screen)) * 25.4 /
              ((double) HeightMMOfScreen (attr.screen));
      area.x = (double) area.x * (96.0 / dpi_x);
      area.y = (double) area.y * (96.0 / dpi_y);
    }
  }

  nimf_service_ic_set_cursor_location (NIMF_SERVICE_IC (xic), &area);
}

static void
nimf_xim_ic_emit_commit (NimfServiceIC *ic,
                         const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXimIC *xic = NIMF_XIM_IC (ic);
  XTextProperty property;
  Xutf8TextListToTextProperty (xic->xim->display,
                               (char **)&text, 1, XCompoundTextStyle,
                               &property);

  IMCommitStruct commit_data = {0};
  commit_data.major_code    = XIM_COMMIT;
  commit_data.connect_id    = xic->connect_id;
  commit_data.icid          = xic->icid;
  commit_data.flag          = XimLookupChars;
  commit_data.commit_string = (gchar *) property.value;
  xi18n_commit (xic->xim, (XPointer) &commit_data);

  XFree (property.value);
}

static void nimf_xim_ic_emit_preedit_end (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXimIC *xic = NIMF_XIM_IC (ic);

  IMPreeditCBStruct preedit_cb_data = {0};
  preedit_cb_data.major_code = XIM_PREEDIT_DONE;
  preedit_cb_data.connect_id = xic->connect_id;
  preedit_cb_data.icid       = xic->icid;
  nimf_xim_call_callback (xic->xim, (XPointer) &preedit_cb_data);
}

static void nimf_xim_ic_emit_preedit_start (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXimIC *xic = NIMF_XIM_IC (ic);

  IMPreeditCBStruct preedit_cb_data = {0};
  preedit_cb_data.major_code = XIM_PREEDIT_START;
  preedit_cb_data.connect_id = xic->connect_id;
  preedit_cb_data.icid       = xic->icid;
  nimf_xim_call_callback (xic->xim, (XPointer) &preedit_cb_data);
}

static void
nimf_xim_ic_emit_preedit_changed (NimfServiceIC    *ic,
                                  const gchar      *preedit_string,
                                  NimfPreeditAttr **attrs,
                                  gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXimIC *xic = NIMF_XIM_IC (ic);

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

  preedit_cb_data.major_code           = XIM_PREEDIT_DRAW;
  preedit_cb_data.connect_id           = xic->connect_id;
  preedit_cb_data.icid                 = xic->icid;
  preedit_cb_data.todo.draw.caret      = cursor_pos;
  preedit_cb_data.todo.draw.chg_first  = 0;
  preedit_cb_data.todo.draw.chg_length = xic->prev_preedit_length;
  preedit_cb_data.todo.draw.text       = &text;

  text.feedback = feedback;

  if (len > 0)
  {
    Xutf8TextListToTextProperty (xic->xim->display,
                                 (char **) &preedit_string, 1,
                                 XCompoundTextStyle, &text_property);
    text.encoding_is_wchar = 0;
    text.length = strlen ((char *) text_property.value);
    text.string.multi_byte = (char *) text_property.value;
    nimf_xim_call_callback (xic->xim, (XPointer) &preedit_cb_data);
    XFree (text_property.value);
  }
  else
  {
    text.encoding_is_wchar = 0;
    text.length = 0;
    text.string.multi_byte = "";
    nimf_xim_call_callback (xic->xim, (XPointer) &preedit_cb_data);
    len = 0;
  }

  xic->prev_preedit_length = len;

  g_free (feedback);
}

NimfXimIC *
nimf_xim_ic_new (NimfXim *xim,
                 guint16  connect_id,
                 guint16  icid)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXimIC *xic = g_object_new (NIMF_TYPE_XIM_IC, NULL);

  xic->xim        = xim;
  xic->connect_id = connect_id;
  xic->icid       = icid;

  return xic;
}

const gchar *
nimf_xim_ic_get_service_id (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_service_get_id (NIMF_SERVICE (NIMF_XIM_IC (ic)->xim));
}

static void
nimf_xim_ic_init (NimfXimIC *xic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_xim_ic_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  G_OBJECT_CLASS (nimf_xim_ic_parent_class)->finalize (object);
}

static void
nimf_xim_ic_class_init (NimfXimICClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass       *object_class     = G_OBJECT_CLASS (class);
  NimfServiceICClass *service_im_class = NIMF_SERVICE_IC_CLASS (class);

  service_im_class->emit_commit          = nimf_xim_ic_emit_commit;
  service_im_class->emit_preedit_start   = nimf_xim_ic_emit_preedit_start;
  service_im_class->emit_preedit_changed = nimf_xim_ic_emit_preedit_changed;
  service_im_class->emit_preedit_end     = nimf_xim_ic_emit_preedit_end;
  service_im_class->get_service_id       = nimf_xim_ic_get_service_id;

  object_class->finalize = nimf_xim_ic_finalize;
}
