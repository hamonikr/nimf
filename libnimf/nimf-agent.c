/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-agent.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2015 Hodong Kim <cogniti@gmail.com>
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

enum {
  ENGINE_CHANGED,
  DISCONNECTED,
  LAST_SIGNAL
};

static guint agent_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (NimfAgent, nimf_agent, G_TYPE_OBJECT);

static gboolean
on_incoming_message (GSocket      *socket,
                     GIOCondition  condition,
                     NimfAgent    *agent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    g_socket_close (socket, NULL);
    g_signal_emit_by_name (agent, "disconnected", NULL);
    g_warning (G_STRLOC ": %s: G_IO_HUP | G_IO_ERR", G_STRFUNC);
    return G_SOURCE_REMOVE;
  }

  NimfMessage *message;
  message = nimf_recv_message (socket);
  nimf_message_unref (agent->reply);
  agent->reply = message;

  if (G_UNLIKELY (message == NULL))
  {
    g_critical (G_STRLOC ": NULL message");
    return G_SOURCE_CONTINUE;
  }

  switch (message->header->type)
  {
    /* reply */
    case NIMF_MESSAGE_ENGINE_CHANGED:
      nimf_message_ref (agent->reply);
      g_signal_emit_by_name (agent, "engine-changed", (gchar *) agent->reply->data);
      nimf_message_unref (agent->reply);
      break;
    default:
      g_warning (G_STRLOC ": %s: Unknown message type: %d", G_STRFUNC, message->header->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

gboolean
nimf_agent_connect_to_server (NimfAgent *agent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSocketAddress *address;
  GSocketClient  *client;
  GSocket        *socket;
  NimfMessage    *message;
  GError         *error = NULL;
  gint            retry_limit = 5;
  gint            retry_count = 0;

  address = g_unix_socket_address_new_with_type (NIMF_ADDRESS, -1,
                                                 G_UNIX_SOCKET_ADDRESS_ABSTRACT);
  client = g_socket_client_new ();

  for (retry_count = 0; retry_count < retry_limit; retry_count++)
  {
    g_clear_error (&error);
    agent->connection = g_socket_client_connect (client,
                                                 G_SOCKET_CONNECTABLE (address),
                                                 NULL, &error);
    if (agent->connection)
      break;
    else
      g_usleep (G_USEC_PER_SEC);;
  }

  g_object_unref (address);
  g_object_unref (client);

  if (agent->connection == NULL)
  {
    g_critical (G_STRLOC ": %s", error->message);
    g_clear_error (&error);
    g_signal_emit_by_name (agent, "disconnected", NULL);
    return FALSE;
  }

  socket = g_socket_connection_get_socket (agent->connection);

  if (!socket)
  {
    g_critical (G_STRLOC ": Can't get socket");
    g_signal_emit_by_name (agent, "disconnected", NULL);
    return FALSE;
  }

  NimfConnectionType type = NIMF_CONNECTION_NIMF_AGENT;

  nimf_send_message (socket, NIMF_MESSAGE_CONNECT, &type,
                     sizeof (NimfConnectionType), NULL);
  g_socket_condition_wait (socket, G_IO_IN, NULL, NULL);
  message = nimf_recv_message (socket);

  if (G_UNLIKELY (message == NULL ||
                  message->header->type != NIMF_MESSAGE_CONNECT_REPLY))
  {
    nimf_message_unref (message);
    g_critical ("Couldn't connect to nimf daemon");
    g_signal_emit_by_name (agent, "disconnected", NULL);
    return FALSE;
  }

  nimf_message_unref (message);

  agent->source = g_socket_create_source (socket, G_IO_IN, NULL);
  g_source_attach (agent->source, NULL);
  g_source_set_callback (agent->source, (GSourceFunc) on_incoming_message,
                         agent, NULL);
  return TRUE;
}

static void
nimf_agent_init (NimfAgent *agent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_agent_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfAgent *agent = NIMF_AGENT (object);

  if (agent->source)
  {
    g_source_destroy (agent->source);
    g_source_unref   (agent->source);
  }

  if (agent->connection)
    g_object_unref (agent->connection);

  nimf_message_unref (agent->reply);

  G_OBJECT_CLASS (nimf_agent_parent_class)->finalize (object);
}

static void
nimf_agent_class_init (NimfAgentClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->finalize = nimf_agent_finalize;

  agent_signals[ENGINE_CHANGED] =
    g_signal_new (g_intern_static_string ("engine-changed"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfAgentClass, engine_changed),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
  agent_signals[DISCONNECTED] =
    g_signal_new (g_intern_static_string ("disconnected"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfAgentClass, disconnected),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

NimfAgent *
nimf_agent_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (NIMF_TYPE_AGENT, NULL);
}

void
nimf_agent_set_engine (gchar *name)
{
  /* TODO */
}

NimfEngineInfo *
nimf_agent_get_engine_info (gchar *name)
{
  /* TODO */
  return NULL;
}

gchar **
nimf_agent_list_engines (NimfAgent *agent)
{
  /* TODO */
  return NULL;
}
