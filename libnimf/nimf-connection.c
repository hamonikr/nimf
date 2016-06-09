/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-connection.c
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

#include "nimf-connection.h"
#include "nimf-events.h"
#include "nimf-marshalers.h"
#include "nimf-private.h"
#include "nimf-context.h"
#include <string.h>
#include <X11/Xutil.h>
#include "IMdkit/Xi18n.h"

G_DEFINE_TYPE (NimfConnection, nimf_connection, G_TYPE_OBJECT);

void
nimf_connection_set_engine_by_id (NimfConnection *connection,
                                  const gchar    *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GHashTableIter iter;
  gpointer       context;

  g_hash_table_iter_init (&iter, connection->contexts);

  while (g_hash_table_iter_next (&iter, NULL, &context))
    if (((NimfContext *) context)->connection->type != NIMF_CONTEXT_NIMF_AGENT)
      nimf_context_set_engine_by_id (context, engine_id);
}

static void
nimf_connection_init (NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  connection->result = g_slice_new0 (NimfResult);
  connection->contexts = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                                NULL,
                                                (GDestroyNotify) nimf_context_free);
}

static void
nimf_connection_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfConnection *connection = NIMF_CONNECTION (object);
  nimf_message_unref (connection->result->reply);

  if (connection->source)
  {
    g_source_destroy (connection->source);
    g_source_unref   (connection->source);
  }

  if (connection->socket_connection)
    g_object_unref (connection->socket_connection);

  g_slice_free (NimfResult, connection->result);
  g_hash_table_unref (connection->contexts);

  G_OBJECT_CLASS (nimf_connection_parent_class)->finalize (object);
}

static void
nimf_connection_class_init (NimfConnectionClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->finalize = nimf_connection_finalize;
}

NimfConnection *
nimf_connection_new (NimfContextType type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfConnection *connection = g_object_new (NIMF_TYPE_CONNECTION, NULL);

  connection->type = type;

  return connection;
}

guint16
nimf_connection_get_id (NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_CONNECTION (connection), 0);

  return connection->id;
}
