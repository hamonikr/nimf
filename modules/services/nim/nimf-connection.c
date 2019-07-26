/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-connection.c
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

#include "nimf-connection.h"
#include "nimf-events.h"
#include "nimf-service-ic.h"
#include <string.h>
#include "nimf-nim-ic.h"

G_DEFINE_TYPE (NimfConnection, nimf_connection, G_TYPE_OBJECT);

void
nimf_connection_change_engine_by_id (NimfConnection *connection,
                                     const gchar    *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GHashTableIter iter;
  gpointer       im;

  g_hash_table_iter_init (&iter, connection->ics);

  while (g_hash_table_iter_next (&iter, NULL, &im))
  {
    if (NIMF_NIM_IC (im)->icid == connection->nim->last_focused_icid)
      nimf_service_ic_change_engine_by_id (im, engine_id);
  }
}

void
nimf_connection_change_engine (NimfConnection *connection,
                               const gchar    *engine_id,
                               const gchar    *method_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GHashTableIter iter;
  gpointer       im;

  g_hash_table_iter_init (&iter, connection->ics);

  while (g_hash_table_iter_next (&iter, NULL, &im))
  {
    if (NIMF_NIM_IC (im)->icid == connection->nim->last_focused_icid)
      nimf_service_ic_change_engine (im, engine_id, method_id);
  }
}

static void
nimf_connection_init (NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  connection->result = nimf_result_new ();
  connection->ics = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
                                           (GDestroyNotify) g_object_unref);
}

static void
nimf_connection_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfConnection *connection = NIMF_CONNECTION (object);

  if (connection->source)
  {
    g_source_destroy (connection->source);
    g_source_unref   (connection->source);
  }

  if (connection->socket_connection)
    g_object_unref (connection->socket_connection);

  nimf_result_unref  (connection->result);
  g_hash_table_unref (connection->ics);

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
nimf_connection_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (NIMF_TYPE_CONNECTION, NULL);
}

guint16
nimf_connection_get_id (NimfConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_CONNECTION (connection), 0);

  return connection->id;
}
