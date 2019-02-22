/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-nim-im.c
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

#include "nimf-nim-im.h"
#include <string.h>

G_DEFINE_TYPE (NimfNimIM, nimf_nim_im, NIMF_TYPE_SERVICE_IM);

void
nimf_nim_im_emit_commit (NimfServiceIM *im,
                         const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIM *nim_im = NIMF_NIM_IM (im);

  nimf_send_message (nim_im->connection->socket, im->icid, NIMF_MESSAGE_COMMIT,
                     (gchar *) text, strlen (text) + 1, NULL);
  nimf_result_iteration_until (nim_im->connection->result, NULL, im->icid,
                               NIMF_MESSAGE_COMMIT_REPLY);
}

void nimf_nim_im_emit_preedit_start (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (im->use_preedit == FALSE &&
                  im->preedit_state == NIMF_PREEDIT_STATE_END))
    return;

  NimfNimIM *nim_im = NIMF_NIM_IM (im);

  nimf_send_message (nim_im->connection->socket, im->icid,
                     NIMF_MESSAGE_PREEDIT_START, NULL, 0, NULL);
  nimf_result_iteration_until (nim_im->connection->result, NULL, im->icid,
                               NIMF_MESSAGE_PREEDIT_START_REPLY);
  im->preedit_state = NIMF_PREEDIT_STATE_START;
}

void
nimf_nim_im_emit_preedit_changed (NimfServiceIM    *im,
                                  const gchar      *preedit_string,
                                  NimfPreeditAttr **attrs,
                                  gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (im->use_preedit == FALSE &&
                  im->preedit_state == NIMF_PREEDIT_STATE_END))
    return;

  NimfNimIM *nim_im = NIMF_NIM_IM (im);
  gchar *data;
  gsize  data_len;
  gint   str_len = strlen (preedit_string);
  gint   n_attr = 0;
  gint   i;

  while (attrs[n_attr] != NULL)
    n_attr++;

  data_len = str_len + 1 + n_attr * sizeof (NimfPreeditAttr) + sizeof (gint);
  data = g_strndup (preedit_string, data_len - 1);

  for (i = 0; attrs[i] != NULL; i++)
    *(NimfPreeditAttr *)
      (data + str_len + 1 + i * sizeof (NimfPreeditAttr)) = *attrs[i];

  *(gint *) (data + data_len - sizeof (gint)) = cursor_pos;

  nimf_send_message (nim_im->connection->socket, im->icid,
                     NIMF_MESSAGE_PREEDIT_CHANGED,
                     data, data_len, g_free);
  nimf_result_iteration_until (nim_im->connection->result, NULL, im->icid,
                               NIMF_MESSAGE_PREEDIT_CHANGED_REPLY);
}

void nimf_nim_im_emit_preedit_end (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (im->use_preedit == FALSE &&
                  im->preedit_state == NIMF_PREEDIT_STATE_END))
    return;

  NimfNimIM *nim_im = NIMF_NIM_IM (im);

  nimf_send_message (nim_im->connection->socket, im->icid,
                     NIMF_MESSAGE_PREEDIT_END, NULL, 0, NULL);
  nimf_result_iteration_until (nim_im->connection->result, NULL, im->icid,
                               NIMF_MESSAGE_PREEDIT_END_REPLY);
  im->preedit_state = NIMF_PREEDIT_STATE_END;
}

gboolean
nimf_nim_im_emit_retrieve_surrounding (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIM *nim_im = NIMF_NIM_IM (im);

  nimf_send_message (nim_im->connection->socket, im->icid,
                     NIMF_MESSAGE_RETRIEVE_SURROUNDING, NULL, 0, NULL);
  nimf_result_iteration_until (nim_im->connection->result, NULL, im->icid,
                               NIMF_MESSAGE_RETRIEVE_SURROUNDING_REPLY);

  if (nim_im->connection->result->reply == NULL)
    return FALSE;

  return *(gboolean *) (nim_im->connection->result->reply->data);
}

gboolean
nimf_nim_im_emit_delete_surrounding (NimfServiceIM *im,
                                     gint           offset,
                                     gint           n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIM *nim_im = NIMF_NIM_IM (im);

  gint *data = g_malloc (2 * sizeof (gint));
  data[0] = offset;
  data[1] = n_chars;

  nimf_send_message (nim_im->connection->socket, im->icid,
                     NIMF_MESSAGE_DELETE_SURROUNDING,
                     data, 2 * sizeof (gint), g_free);
  nimf_result_iteration_until (nim_im->connection->result, NULL, im->icid,
                               NIMF_MESSAGE_DELETE_SURROUNDING_REPLY);

  if (nim_im->connection->result->reply == NULL)
    return FALSE;

  return *(gboolean *) (nim_im->connection->result->reply->data);
}

void nimf_nim_im_emit_beep (NimfServiceIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIM *nim_im = NIMF_NIM_IM (im);

  nimf_send_message (nim_im->connection->socket, im->icid,
                     NIMF_MESSAGE_BEEP, NULL, 0, NULL);
  nimf_result_iteration_until (nim_im->connection->result, NULL, im->icid,
                               NIMF_MESSAGE_BEEP_REPLY);
}

NimfNimIM *nimf_nim_im_new (NimfConnection *connection)
{
  NimfNimIM *nim_im;

  nim_im = g_object_new (NIMF_TYPE_NIM_IM, NULL);
  nim_im->connection = connection;

  return nim_im;
}

static void
nimf_nim_im_init (NimfNimIM *nim_im)
{
}

static void
nimf_nim_im_finalize (GObject *object)
{
  G_OBJECT_CLASS (nimf_nim_im_parent_class)->finalize (object);
}

static void
nimf_nim_im_class_init (NimfNimIMClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  NimfServiceIMClass *service_im_class = NIMF_SERVICE_IM_CLASS (class);

  object_class->finalize = nimf_nim_im_finalize;

  service_im_class->emit_commit          = nimf_nim_im_emit_commit;
  service_im_class->emit_preedit_start   = nimf_nim_im_emit_preedit_start;
  service_im_class->emit_preedit_changed = nimf_nim_im_emit_preedit_changed;
  service_im_class->emit_preedit_end     = nimf_nim_im_emit_preedit_end;
  service_im_class->emit_retrieve_surrounding = nimf_nim_im_emit_retrieve_surrounding;
  service_im_class->emit_delete_surrounding = nimf_nim_im_emit_delete_surrounding;
  service_im_class->emit_beep            = nimf_nim_im_emit_beep;
}
