/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-agent.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015,2016 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-agent.h"
#include "nimf-private.h"
#include <gio/gunixsocketaddress.h>
#include "nimf-marshalers.h"
#include <string.h>

extern GMainContext      *nimf_client_socket_context;
extern NimfResult        *nimf_client_result;
extern GSocketConnection *nimf_client_connection;

G_DEFINE_TYPE (NimfAgent, nimf_agent, NIMF_TYPE_CLIENT);

static void
nimf_agent_init (NimfAgent *agent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_agent_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  G_OBJECT_CLASS (nimf_agent_parent_class)->finalize (object);
}

static void
nimf_agent_class_init (NimfAgentClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->finalize = nimf_agent_finalize;
}

NimfAgent *
nimf_agent_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (NIMF_TYPE_AGENT,
                       "connection-type", NIMF_CONNECTION_NIMF_AGENT, NULL);
}

void
nimf_agent_set_engine_by_id (NimfAgent   *agent,
                             const gchar *id,
                             gboolean     is_english_mode)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfClient *client = NIMF_CLIENT (agent);

  /* TODO: 요 코드 정상 작동 여부 반드시 확인할 것 */
  if (nimf_client_connection == NULL ||
      !g_socket_connection_is_connected (nimf_client_connection))
    return;

  GSocket *socket = g_socket_connection_get_socket (nimf_client_connection);

  gchar *data     = NULL;
  gint   str_len  = strlen (id);
  gint   data_len = str_len + 1 + sizeof (gboolean);

  data = g_strndup (id, data_len - 1);
  *(gboolean *) (data + str_len + 1) = is_english_mode;

  nimf_send_message (socket, client->id, NIMF_MESSAGE_SET_ENGINE_BY_ID,
                     data, data_len, g_free);
  nimf_result_iteration_until (nimf_client_result, nimf_client_socket_context,
                               client->id,
                               NIMF_MESSAGE_SET_ENGINE_BY_ID_REPLY);
}

NimfEngineInfo *
nimf_agent_get_engine_info (gchar *name)
{
  /* TODO */
  return NULL;
}

/**
 * nimf_agent_get_loaded_engine_ids:
 * @agent: a #NimfAgent.
 *
 * Returns: (transfer full): gchar **
 */
gchar **
nimf_agent_get_loaded_engine_ids (NimfAgent *agent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfClient *client = NIMF_CLIENT (agent);

  /* TODO: 요 코드 정상 작동 여부 반드시 확인할 것 */
  if (nimf_client_connection == NULL ||
      !g_socket_connection_is_connected (nimf_client_connection))
    return NULL;

  GSocket *socket = g_socket_connection_get_socket (nimf_client_connection);

  nimf_send_message (socket, client->id, NIMF_MESSAGE_GET_LOADED_ENGINE_IDS,
                     NULL, 0, NULL);
  nimf_result_iteration_until (nimf_client_result, nimf_client_socket_context,
                               client->id,
                               NIMF_MESSAGE_GET_LOADED_ENGINE_IDS_REPLY);
  if (nimf_client_result->reply == NULL)
    return NULL;
  /* 0x1e is RS (record separator) */
  return g_strsplit (nimf_client_result->reply->data, "\x1e", -1);
}
