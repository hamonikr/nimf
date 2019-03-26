/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-server.c
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

#include <glib.h>
#include <glib-object.h>
#include "nimf-server.h"
#include "nimf-server-private.h"
#include "nimf-service.h"
#include <glib/gstdio.h>
#include "nimf-marshalers-private.h"

enum {
  ENGINE_CHANGED,
  ENGINE_STATUS_CHANGED,
  LAST_SIGNAL
};

static guint nimf_server_signals[LAST_SIGNAL] = { 0 };
static NimfServer *nimf_server = NULL;

G_DEFINE_TYPE_WITH_PRIVATE (NimfServer, nimf_server, G_TYPE_OBJECT);

static gint
on_comparing_engine_with_id (NimfEngine *engine, const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_strcmp0 (nimf_engine_get_id (engine), engine_id);
}

NimfEngine *
nimf_server_get_engine_by_id (NimfServer  *server,
                              const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  list = g_list_find_custom (g_list_first (server->priv->instances), engine_id,
                             (GCompareFunc) on_comparing_engine_with_id);
  if (list)
    return list->data;

  return NULL;
}

NimfEngine *
nimf_server_get_next_instance (NimfServer *server, NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  server->priv->instances = g_list_first (server->priv->instances);
  server->priv->instances = g_list_find  (server->priv->instances, engine);

  list = g_list_next (server->priv->instances);

  if (list == NULL)
    list = g_list_first (server->priv->instances);

  if (list)
  {
    server->priv->instances = list;
    return list->data;
  }

  g_assert (list != NULL);

  return engine;
}

NimfEngine *
nimf_server_get_default_engine (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings  *settings;
  gchar      *engine_id;
  NimfEngine *engine;

  settings  = g_settings_new ("org.nimf.engines");
  engine_id = g_settings_get_string (settings, "default-engine");
  engine    = nimf_server_get_engine_by_id (server, engine_id);

  if (G_UNLIKELY (engine == NULL))
  {
    g_settings_reset (settings, "default-engine");
    g_free (engine_id);
    engine_id = g_settings_get_string (settings, "default-engine");
    engine = nimf_server_get_engine_by_id (server, engine_id);
  }

  g_free (engine_id);
  g_object_unref (settings);

  return engine;
}

/**
 * nimf_server_get_default:
 *
 * Returns the default #NimfServer instance.
 *
 * If there is no default server then %NULL is returned.
 *
 * Returns: (transfer none): the default server, or %NULL if server is not
 * running
 */
NimfServer *
nimf_server_get_default ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_server;
}

/**
 * nimf_server_get_preeditable:
 * @server: a #NimfServer
 *
 * Returns the #NimfPreeditable instance.
 *
 * If there is no default preeditable then %NULL is returned.
 *
 * Returns: (transfer none): a #NimfPreeditable, or %NULL
 */
NimfPreeditable *
nimf_server_get_preeditable (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return server->priv->preeditable;
}

static void
on_changed_hotkeys (GSettings  *settings,
                    gchar      *key,
                    NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **keys = g_settings_get_strv (settings, key);

  nimf_key_freev (server->priv->hotkeys);
  server->priv->hotkeys = nimf_key_newv ((const gchar **) keys);

  g_strfreev (keys);
}

static void
on_use_singleton (GSettings  *settings,
                  gchar      *key,
                  NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  server->priv->use_singleton = g_settings_get_boolean (server->priv->settings,
                                                        "use-singleton");
}

static void
nimf_server_init (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_server = server;
  server->priv = nimf_server_get_instance_private (server);

  server->priv->settings = g_settings_new ("org.nimf");
  server->priv->use_singleton = g_settings_get_boolean (server->priv->settings,
                                                        "use-singleton");
  server->priv->trigger_gsettings = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                           g_free, g_object_unref);
  server->priv->trigger_keys = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                                      (GDestroyNotify) nimf_key_freev,
                                                      g_free);
  gchar **hotkeys = g_settings_get_strv (server->priv->settings, "hotkeys");
  server->priv->hotkeys = nimf_key_newv ((const gchar **) hotkeys);
  g_strfreev (hotkeys);

  g_signal_connect (server->priv->settings, "changed::hotkeys",
                    G_CALLBACK (on_changed_hotkeys), server);
  g_signal_connect (server->priv->settings, "changed::use-singleton",
                    G_CALLBACK (on_use_singleton), server);

  server->priv->modules  = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free, NULL);
  server->priv->services = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free, g_object_unref);
}

static void
nimf_server_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServer     *server = NIMF_SERVER (object);
  GHashTableIter  iter;
  gpointer        service;

  g_hash_table_iter_init (&iter, server->priv->services);
  while (g_hash_table_iter_next (&iter, NULL, &service))
    nimf_service_stop (NIMF_SERVICE (service));

  g_hash_table_unref (server->priv->modules);
  g_hash_table_unref (server->priv->services);

  if (server->priv->instances)
  {
    g_list_free_full (server->priv->instances, g_object_unref);
    server->priv->instances = NULL;
  }

  g_object_unref     (server->priv->settings);
  g_hash_table_unref (server->priv->trigger_gsettings);
  g_hash_table_unref (server->priv->trigger_keys);
  nimf_key_freev     (server->priv->hotkeys);

  G_OBJECT_CLASS (nimf_server_parent_class)->finalize (object);
}

static void
nimf_server_class_init (NimfServerClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = nimf_server_finalize;

  /**
   * NimfServer::engine-changed:
   * @server: a #NimfServer
   * @engine_id: engine id
   * @icon_name: icon name
   *
   * The #NimfServer::engine-changed signal is emitted when the engine is
   * changed.
   */
  nimf_server_signals[ENGINE_CHANGED] =
    g_signal_new (g_intern_static_string ("engine-changed"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfServerClass, engine_changed),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__STRING_STRING,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  G_TYPE_STRING);
  /**
   * NimfServer::engine-status-changed:
   * @server: a #NimfServer
   * @engine_id: engine id
   * @icon_name: icon name
   *
   * The #NimfServer::engine-status-changed signal is emitted when the engine
   * status is changed.
   */
  nimf_server_signals[ENGINE_STATUS_CHANGED] =
    g_signal_new (g_intern_static_string ("engine-status-changed"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfServerClass, engine_status_changed),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__STRING_STRING,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  G_TYPE_STRING);
}

/**
 * nimf_server_change_engine_by_id:
 * @server: a #NimfServer
 * @engine_id: engine id
 */
void
nimf_server_change_engine_by_id (NimfServer  *server,
                                 const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GHashTableIter  iter;
  gpointer        service;

  g_hash_table_iter_init (&iter, server->priv->services);

  while (g_hash_table_iter_next (&iter, NULL, &service))
  {
    if (!g_strcmp0 (server->priv->last_focused_service,
                    nimf_service_get_id (service)))
      nimf_service_change_engine_by_id (service, engine_id);
  }
}

/**
 * nimf_server_change_engine:
 * @server: a #NimfServer
 * @engine_id: engine id
 * @method_id: method id
 */
void
nimf_server_change_engine (NimfServer  *server,
                           const gchar *engine_id,
                           const gchar *method_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GHashTableIter  iter;
  gpointer        service;

  g_hash_table_iter_init (&iter, server->priv->services);

  while (g_hash_table_iter_next (&iter, NULL, &service))
  {
    if (!g_strcmp0 (server->priv->last_focused_service,
                    nimf_service_get_id (service)))
      nimf_service_change_engine (service, engine_id, method_id);
  }
}

/**
 * nimf_server_get_loaded_engine_ids:
 * @server: a #NimfServer
 *
 * The array is %NULL-terminated.
 *
 * Returns: (transfer full): a new %NULL-terminated engine id array
 */
gchar **nimf_server_get_loaded_engine_ids (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **engine_ids;
  gint    i;
  GList  *list;
  const gchar *id;

  engine_ids = g_malloc0_n (1, sizeof (gchar *));

  for (list = g_list_first (server->priv->instances), i = 0;
       list != NULL;
       list = list->next, i++)
  {
    id = nimf_engine_get_id (list->data);
    engine_ids[i] = g_strdup (id);
    engine_ids = g_realloc_n (engine_ids, sizeof (gchar *), i + 2);
    engine_ids[i + 1] = NULL;
  }

  return engine_ids;
}

NimfServiceIC *
nimf_server_get_last_focused_im (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return server->priv->last_focused_im;
}

void
nimf_server_set_last_focused_im (NimfServer    *server,
                                 NimfServiceIC *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  server->priv->last_focused_im = im;
}
