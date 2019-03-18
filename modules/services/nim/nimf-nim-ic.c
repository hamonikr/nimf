/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-nim-ic.c
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

#include "nimf-nim-ic.h"
#include <string.h>
#include "nimf-connection.h"
#include "nimf-message-private.h"

G_DEFINE_TYPE (NimfNimIC, nimf_nim_ic, NIMF_TYPE_SERVICE_IC);

void
nimf_nim_ic_emit_commit (NimfServiceIC *ic,
                         const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIC *nic = NIMF_NIM_IC (ic);

  nimf_send_message (nic->connection->socket, nic->icid, NIMF_MESSAGE_COMMIT,
                     (gchar *) text, strlen (text) + 1, NULL);
  nimf_result_iteration_until (nic->connection->result, NULL, nic->icid,
                               NIMF_MESSAGE_COMMIT_REPLY);
}

void nimf_nim_ic_emit_preedit_start (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIC *nic = NIMF_NIM_IC (ic);

  nimf_send_message (nic->connection->socket, nic->icid,
                     NIMF_MESSAGE_PREEDIT_START, NULL, 0, NULL);
  nimf_result_iteration_until (nic->connection->result, NULL, nic->icid,
                               NIMF_MESSAGE_PREEDIT_START_REPLY);
}

void
nimf_nim_ic_emit_preedit_changed (NimfServiceIC    *ic,
                                  const gchar      *preedit_string,
                                  NimfPreeditAttr **attrs,
                                  gint              cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIC *nic = NIMF_NIM_IC (ic);
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

  nimf_send_message (nic->connection->socket, nic->icid,
                     NIMF_MESSAGE_PREEDIT_CHANGED, data, data_len, g_free);
  nimf_result_iteration_until (nic->connection->result, NULL, nic->icid,
                               NIMF_MESSAGE_PREEDIT_CHANGED_REPLY);
}

void nimf_nim_ic_emit_preedit_end (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIC *nic = NIMF_NIM_IC (ic);

  nimf_send_message (nic->connection->socket, nic->icid,
                     NIMF_MESSAGE_PREEDIT_END, NULL, 0, NULL);
  nimf_result_iteration_until (nic->connection->result, NULL, nic->icid,
                               NIMF_MESSAGE_PREEDIT_END_REPLY);
}

gboolean
nimf_nim_ic_emit_retrieve_surrounding (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIC *nic = NIMF_NIM_IC (ic);

  nimf_send_message (nic->connection->socket, nic->icid,
                     NIMF_MESSAGE_RETRIEVE_SURROUNDING, NULL, 0, NULL);
  nimf_result_iteration_until (nic->connection->result, NULL, nic->icid,
                               NIMF_MESSAGE_RETRIEVE_SURROUNDING_REPLY);

  if (nic->connection->result->reply == NULL)
    return FALSE;

  return *(gboolean *) (nic->connection->result->reply->data);
}

gboolean
nimf_nim_ic_emit_delete_surrounding (NimfServiceIC *ic,
                                     gint           offset,
                                     gint           n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIC *nic = NIMF_NIM_IC (ic);

  gint *data = g_malloc (2 * sizeof (gint));
  data[0] = offset;
  data[1] = n_chars;

  nimf_send_message (nic->connection->socket, nic->icid,
                     NIMF_MESSAGE_DELETE_SURROUNDING,
                     data, 2 * sizeof (gint), g_free);
  nimf_result_iteration_until (nic->connection->result, NULL, nic->icid,
                               NIMF_MESSAGE_DELETE_SURROUNDING_REPLY);

  if (nic->connection->result->reply == NULL)
    return FALSE;

  return *(gboolean *) (nic->connection->result->reply->data);
}

void nimf_nim_ic_emit_beep (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIC *nic = NIMF_NIM_IC (ic);

  nimf_send_message (nic->connection->socket, nic->icid,
                     NIMF_MESSAGE_BEEP, NULL, 0, NULL);
  nimf_result_iteration_until (nic->connection->result, NULL, nic->icid,
                               NIMF_MESSAGE_BEEP_REPLY);
}

const gchar *
nimf_nim_ic_get_service_id (NimfServiceIC *ic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfService *service = NIMF_SERVICE (NIMF_NIM_IC (ic)->connection->nim);

  return nimf_service_get_id (service);
}

NimfNimIC *
nimf_nim_ic_new (guint16         icid,
                 NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfNimIC *nic = g_object_new (NIMF_TYPE_NIM_IC, NULL);

  nic->connection = connection;
  nic->icid = icid;

  return nic;
}

static void
nimf_nim_ic_init (NimfNimIC *nic)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_nim_ic_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  G_OBJECT_CLASS (nimf_nim_ic_parent_class)->finalize (object);
}

static void
nimf_nim_ic_class_init (NimfNimICClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass       *object_class     = G_OBJECT_CLASS (class);
  NimfServiceICClass *service_im_class = NIMF_SERVICE_IC_CLASS (class);

  object_class->finalize = nimf_nim_ic_finalize;

  service_im_class->emit_commit          = nimf_nim_ic_emit_commit;
  service_im_class->emit_preedit_start   = nimf_nim_ic_emit_preedit_start;
  service_im_class->emit_preedit_changed = nimf_nim_ic_emit_preedit_changed;
  service_im_class->emit_preedit_end     = nimf_nim_ic_emit_preedit_end;
  service_im_class->emit_retrieve_surrounding = nimf_nim_ic_emit_retrieve_surrounding;
  service_im_class->emit_delete_surrounding = nimf_nim_ic_emit_delete_surrounding;
  service_im_class->emit_beep            = nimf_nim_ic_emit_beep;
  service_im_class->get_service_id       = nimf_nim_ic_get_service_id;
}
