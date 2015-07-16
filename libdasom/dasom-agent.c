/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-agent.c
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

#include "config.h"
#include "dasom-agent.h"
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "dasom-private.h"
#include <gio/gunixsocketaddress.h>
#include "dasom-marshalers.h"

enum {
  ENGINE_CHANGED,
  LAST_SIGNAL
};

static guint agent_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (DasomAgent, dasom_agent, G_TYPE_OBJECT);

static gboolean
on_incoming_message (GSocket      *socket,
                     GIOCondition  condition,
                     gpointer      user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomAgent *agent = DASOM_AGENT (user_data);

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    g_socket_close (socket, NULL);
    return G_SOURCE_REMOVE;
  }

  DasomMessage *message;
  message = dasom_recv_message (socket);
  dasom_message_unref (agent->reply);
  agent->reply = message;

  switch (message->header->type)
  {
    /* reply */
    case DASOM_MESSAGE_ENGINE_CHANGED:
      dasom_message_ref (agent->reply);
      g_signal_emit_by_name (agent, "engine-changed", (gchar *) agent->reply->data);
      dasom_message_unref (agent->reply);
      break;
    default:
      g_warning (G_STRLOC ": %s: Unknown message type: %d", G_STRFUNC, message->header->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

static void
dasom_agent_init (DasomAgent *agent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSocketClient  *client;
  GSocketAddress *address;
  GSocket        *socket;
  DasomMessage   *message;

  address = g_unix_socket_address_new_with_type ("unix:abstract=dasom", -1,
                                                  G_UNIX_SOCKET_ADDRESS_ABSTRACT);

  client = g_socket_client_new ();
  agent->connection = g_socket_client_connect (client,
                                               G_SOCKET_CONNECTABLE (address),
                                               NULL, NULL);
  g_object_unref (address);

  if (agent->connection == NULL)
    return; /* 에러 메시지 있어야 한다 */

  socket = g_socket_connection_get_socket (agent->connection);

  if (!socket)
    return; /* 에러 메시지 있어야 한다 */

  DasomConnectionType type = DASOM_CONNECTION_DASOM_AGENT;

  dasom_send_message (socket, DASOM_MESSAGE_CONNECT, &type, sizeof (DasomConnectionType), NULL);
  g_socket_condition_wait (socket, G_IO_IN, NULL, NULL);
  message = dasom_recv_message (socket);

  if (message->header->type != DASOM_MESSAGE_CONNECT_REPLY)
    g_error ("FIXME: error handling");

  g_print ("\trecv: DASOM_MESSAGE_CONNECT_REPLY\n");

  dasom_message_unref (message);

  GSource *source = g_socket_create_source (socket, G_IO_IN | G_IO_HUP | G_IO_ERR, NULL);
  g_source_attach (source, NULL);
  g_source_set_callback (source,
                         (GSourceFunc) on_incoming_message,
                         agent,
                         NULL);
}

static void
dasom_agent_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomAgent *agent = DASOM_AGENT (object);
  dasom_message_unref (agent->reply);

  G_OBJECT_CLASS (dasom_agent_parent_class)->finalize (object);
}

static void
dasom_agent_class_init (DasomAgentClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->finalize = dasom_agent_finalize;

  agent_signals[ENGINE_CHANGED] =
    g_signal_new (g_intern_static_string ("engine-changed"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DasomAgentClass, engine_changed),
                  NULL, NULL,
                  dasom_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

DasomAgent *
dasom_agent_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (DASOM_TYPE_AGENT, NULL);
}

void
dasom_agent_set_engine (gchar *name)
{
}

DasomEngineInfo *
dasom_agent_get_engine_info (gchar *name)
{
  return NULL;
}

gchar **
dasom_agent_list_engines (DasomAgent *agent)
{
  return NULL;
}
