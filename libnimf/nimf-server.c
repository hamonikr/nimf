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
#include "nimf-module-private.h"
#include <string.h>
#include "nimf-service-ic-private.h"

enum {
  ENGINE_CHANGED,
  ENGINE_STATUS_CHANGED,
  ENGINE_LOADED,
  ENGINE_UNLOADED,
  LAST_SIGNAL
};

static guint nimf_server_signals[LAST_SIGNAL] = { 0 };
static NimfServer *_nimf_server_ = NULL;

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

  list = g_list_find_custom (server->priv->engines, engine_id,
                             (GCompareFunc) on_comparing_engine_with_id);
  if (list)
    return list->data;

  return NULL;
}

NimfEngine *
nimf_server_get_next_engine (NimfServer *server, NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  list = g_list_find (server->priv->engines, engine);

  if (list == NULL)
    list = g_list_find_custom (server->priv->engines, nimf_engine_get_id (engine),
                               (GCompareFunc) on_comparing_engine_with_id);

  list = g_list_next (list);

  if (list == NULL)
    list = server->priv->engines;

  return list->data;
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

  return _nimf_server_;
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
on_changed_shortcuts (GSettings  *settings,
                      gchar      *key,
                      NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfShortcut *shortcut;
  const gchar  *engine_id;
  gchar        *schema_id;

  g_object_get (settings, "schema-id", &schema_id, NULL);
  engine_id = schema_id + strlen ("org.nimf.engines.");
  shortcut  = g_hash_table_lookup (server->priv->shortcuts, engine_id);

  if (g_strcmp0 (key, "shortcuts-to-lang") == 0)
  {
    gchar **keys;

    keys = g_settings_get_strv (settings, key);
    shortcut->to_lang = nimf_key_newv ((const gchar **) keys);

    g_strfreev (keys);
  }
  else if (g_strcmp0 (key, "shortcuts-to-sys") == 0)
  {
    gchar **keys;

    keys = g_settings_get_strv (settings, key);
    shortcut->to_sys = nimf_key_newv ((const gchar **) keys);

    g_strfreev (keys);
  }

  g_free (schema_id);
}

static NimfShortcut *
nimf_server_shortcut_new (NimfServer *server,
                          GSettings  *settings)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettingsSchemaSource *source;
  GSettingsSchema       *schema;
  NimfShortcut          *shortcut;
  gchar                 *schema_id;

  g_object_get (settings, "schema-id", &schema_id, NULL);
  source   = g_settings_schema_source_get_default ();
  schema   = g_settings_schema_source_lookup (source, schema_id, TRUE);
  shortcut = g_slice_new0 (NimfShortcut);

  if (g_settings_schema_has_key (schema, "shortcuts-to-lang"))
  {
    gchar **to_lang;

    to_lang = g_settings_get_strv (settings, "shortcuts-to-lang");
    shortcut->to_lang = nimf_key_newv ((const gchar **) to_lang);
    g_signal_connect (settings, "changed::shortcuts-to-lang",
                      G_CALLBACK (on_changed_shortcuts), server);

    g_strfreev (to_lang);
  }

  if (g_settings_schema_has_key (schema, "shortcuts-to-sys"))
  {
    gchar **to_sys;

    to_sys = g_settings_get_strv (settings, "shortcuts-to-sys");
    shortcut->to_sys  = nimf_key_newv ((const gchar **) to_sys);
    g_signal_connect (settings, "changed::shortcuts-to-sys",
                      G_CALLBACK (on_changed_shortcuts), server);

    g_strfreev (to_sys);
  }

  shortcut->settings = settings;

  g_free (schema_id);
  g_settings_schema_unref (schema);

  return shortcut;
}

static void
nimf_server_shortcut_free (NimfShortcut *shortcut)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (shortcut->settings)
    g_object_unref (shortcut->settings);

  if (shortcut->to_lang)
    nimf_key_freev (shortcut->to_lang);

  if (shortcut->to_sys)
    nimf_key_freev (shortcut->to_sys);

  g_slice_free (NimfShortcut, shortcut);
}

static void
nimf_server_init (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  _nimf_server_ = server;
  server->priv = nimf_server_get_instance_private (server);
  server->priv->ics = g_ptr_array_new ();
  server->priv->settings = g_settings_new ("org.nimf");
  server->priv->use_singleton = g_settings_get_boolean (server->priv->settings,
                                                        "use-singleton");
  server->priv->shortcuts =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                           (GDestroyNotify) nimf_server_shortcut_free);
  gchar **hotkeys = g_settings_get_strv (server->priv->settings, "hotkeys");
  server->priv->hotkeys = nimf_key_newv ((const gchar **) hotkeys);
  g_strfreev (hotkeys);

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

  g_ptr_array_free (server->priv->ics, FALSE);

  g_hash_table_iter_init (&iter, server->priv->services);
  while (g_hash_table_iter_next (&iter, NULL, &service))
    nimf_service_stop (NIMF_SERVICE (service));

  g_hash_table_unref (server->priv->modules);
  g_hash_table_unref (server->priv->services);

  if (server->priv->engines)
  {
    g_list_free_full (server->priv->engines, g_object_unref);
    server->priv->engines = NULL;
  }

  g_object_unref     (server->priv->settings);
  g_hash_table_unref (server->priv->shortcuts);
  nimf_key_freev     (server->priv->hotkeys);

  G_OBJECT_CLASS (nimf_server_parent_class)->finalize (object);
}

static void
nimf_server_class_init (NimfServerClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  G_OBJECT_CLASS (class)->finalize = nimf_server_finalize;

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
  /**
   * NimfServer::engine-loaded:
   * @server: a #NimfServer
   * @engine_id: engine id
   *
   * The #NimfServer::engine-loaded signal is emitted when the engine is loaded.
   */
  nimf_server_signals[ENGINE_LOADED] =
    g_signal_new (g_intern_static_string ("engine-loaded"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfServerClass, engine_loaded),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
  /**
   * NimfServer::engine-unloaded:
   * @server: a #NimfServer
   * @engine_id: engine id
   *
   * The #NimfServer::engine-unloaded signal is emitted when the engine is
   * unloaded.
   */
  nimf_server_signals[ENGINE_UNLOADED] =
    g_signal_new (g_intern_static_string ("engine-unloaded"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfServerClass, engine_unloaded),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

/**
 * nimf_server_change_engine_by_id:
 * @server: a #NimfServer
 * @engine_id: engine id
 *
 * Changes the last focused engine to the engine with the given ID.
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

static gint
on_comparison (gconstpointer engine_id_a,
               gconstpointer engine_id_b)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings *settings_a;
  GSettings *settings_b;
  gchar     *schema_id_a;
  gchar     *schema_id_b;
  gchar     *schema_name_a;
  gchar     *schema_name_b;
  gint       retval;

  schema_id_a = g_strdup_printf ("org.nimf.engines.%s", *(gchar **) engine_id_a);
  schema_id_b = g_strdup_printf ("org.nimf.engines.%s", *(gchar **) engine_id_b);

  settings_a = g_settings_new (schema_id_a);
  settings_b = g_settings_new (schema_id_b);

  schema_name_a = g_settings_get_string (settings_a, "hidden-schema-name");
  schema_name_b = g_settings_get_string (settings_b, "hidden-schema-name");

  retval = g_utf8_collate (schema_name_a, schema_name_b);

  g_free (schema_name_a);
  g_free (schema_name_b);
  g_free (schema_id_a);
  g_free (schema_id_b);
  g_object_unref (settings_a);
  g_object_unref (settings_b);

  return retval;
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

  GList       *list;
  const gchar *engine_id;

  GPtrArray *array = g_ptr_array_new ();

  for (list = server->priv->engines; list != NULL; list = list->next)
  {
    engine_id = nimf_engine_get_id (list->data);
    g_ptr_array_add (array, g_strdup (engine_id));
  }

  g_ptr_array_sort (array, on_comparison);
  g_ptr_array_add (array, NULL);

  return (gchar **) g_ptr_array_free (array, FALSE);
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

static void
nimf_server_load_service (NimfServer  *server,
                          const gchar *path)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfModule  *module;
  NimfService *service;

  module = nimf_module_new (path);

  if (!g_type_module_use (G_TYPE_MODULE (module)))
  {
    g_warning (G_STRLOC ":" "Failed to load module: %s", path);
    g_object_unref (module);
    return;
  }

  service = g_object_new (module->type, NULL);
  g_hash_table_insert (server->priv->services,
                       g_strdup (nimf_service_get_id (service)), service);

  g_type_module_unuse (G_TYPE_MODULE (module));
}

static void
nimf_server_load_services (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GDir        *dir;
  GError      *error = NULL;
  const gchar *filename;
  gchar       *path;

  dir = g_dir_open (NIMF_SERVICE_MODULE_DIR, 0, &error);

  if (error)
  {
    g_warning (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
    g_clear_error (&error);
    return;
  }

  while ((filename = g_dir_read_name (dir)))
  {
    path = g_build_path (G_DIR_SEPARATOR_S, NIMF_SERVICE_MODULE_DIR, filename, NULL);
    nimf_server_load_service (server, path);
    g_free (path);
  }

  g_dir_close (dir);
}

static void
nimf_server_load_engines (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettingsSchemaSource  *source; /* do not free */
  gchar                 **schema_ids;
  GPtrArray              *engine_ids;
  gchar                 **active_engines;
  gint                    i;

  engine_ids = g_ptr_array_new ();
  source = g_settings_schema_source_get_default ();
  g_settings_schema_source_list_schemas (source, TRUE, &schema_ids, NULL);

  for (i = 0; schema_ids[i] != NULL; i++)
  {
    if (g_str_has_prefix (schema_ids[i], "org.nimf.engines."))
    {
      GSettingsSchema *schema;
      GSettings       *settings;
      const gchar     *engine_id;
      gboolean         active = TRUE;

      engine_id = schema_ids[i] + strlen ("org.nimf.engines.");
      schema = g_settings_schema_source_lookup (source, schema_ids[i], TRUE);
      settings = g_settings_new (schema_ids[i]);

      if (g_settings_schema_has_key (schema, "active-engine"))
        active = g_settings_get_boolean (settings, "active-engine");

      if (active)
      {
        NimfModule *module;
        NimfEngine *engine;
        gchar      *path;

        path = g_module_build_path (NIMF_MODULE_DIR, engine_id);
        module = nimf_module_new (path);

        if (!g_type_module_use (G_TYPE_MODULE (module)))
        {
          g_warning (G_STRLOC ": Failed to load module: %s", path);

          g_object_unref (module);
          g_free (path);
          g_object_unref (settings);
          g_settings_schema_unref (schema);

          continue;
        }

        g_hash_table_insert (server->priv->modules, g_strdup (engine_id), module);
        engine = g_object_new (module->type, NULL);
        g_type_module_unuse (G_TYPE_MODULE (module));
        server->priv->engines = g_list_prepend (server->priv->engines, engine);
        g_ptr_array_add (engine_ids, g_strdup (engine_id));

        if (g_settings_schema_has_key (schema, "shortcuts-to-lang"))
          g_hash_table_insert (server->priv->shortcuts, g_strdup (engine_id),
                               nimf_server_shortcut_new (server, settings));

        g_free (path);
      }

      g_settings_schema_unref (schema);
    }
  }

  g_strfreev (schema_ids);
  g_ptr_array_add (engine_ids, NULL);
  active_engines = (gchar **) g_ptr_array_free (engine_ids, FALSE);
  g_settings_set_strv (server->priv->settings, "hidden-active-engines",
                       (const gchar *const *) active_engines);
  g_strfreev (active_engines);
}

static void
on_changed_active_engines (GSettings  *settings,
                           gchar      *key,
                           NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettingsSchemaSource *source;
  gchar **engine_ids;
  gint    i;

  source = g_settings_schema_source_get_default ();
  engine_ids = g_settings_get_strv (settings, key);

  for (i = 0; engine_ids[i]; i++)
  {
    if (!nimf_server_get_engine_by_id (server, engine_ids[i]))
    {
      GSettingsSchema *schema;
      gchar      *schema_id;
      NimfEngine *engine;
      NimfServer *server = nimf_server_get_default ();
      NimfModule *module = g_hash_table_lookup (server->priv->modules, engine_ids[i]);
      gint        index;

      if (!module)
      {
        gchar *path;

        path = g_module_build_path (NIMF_MODULE_DIR, engine_ids[i]);
        module = nimf_module_new (path);

        if (!g_type_module_use (G_TYPE_MODULE (module)))
        {
          g_warning (G_STRLOC ": Failed to load module: %s", path);

          g_object_unref (module);
          g_free (path);

          return;
        }

        g_free (path);
        g_hash_table_insert (server->priv->modules, g_strdup (engine_ids[i]), module);
      }

      g_type_module_use (G_TYPE_MODULE (module));
      engine = g_object_new (module->type, NULL);
      g_type_module_unuse (G_TYPE_MODULE (module));

      server->priv->engines = g_list_prepend (server->priv->engines, engine);

      for (index = 0; index < server->priv->ics->len; index++)
      {
        NimfServiceIC *ic = g_ptr_array_index (server->priv->ics, index);
        nimf_service_ic_load_engine (ic, engine_ids[i], server);
      }

      schema_id = g_strdup_printf ("org.nimf.engines.%s", engine_ids[i]);
      schema = g_settings_schema_source_lookup (source, schema_id, TRUE);

      if (g_settings_schema_has_key (schema, "shortcuts-to-lang"))
        g_hash_table_insert (server->priv->shortcuts, g_strdup (engine_ids[i]),
                             nimf_server_shortcut_new (server, g_settings_new (schema_id)));

      g_signal_emit (server, nimf_server_signals[ENGINE_LOADED], 0, engine_ids[i]);

      g_free (schema_id);
      g_settings_schema_unref (schema);
    }
  }

  GList *l = server->priv->engines;

  while (l != NULL)
  {
    const gchar *engine_id = nimf_engine_get_id (l->data);
    GList       *next      = l->next;

    if (g_strcmp0 (engine_id, "nimf-system-keyboard") &&
        !g_strv_contains ((const gchar * const *) engine_ids, engine_id))
    {
      NimfEngine *engine = l->data;
      gint        index;

      g_hash_table_remove (server->priv->shortcuts, engine_id);
      server->priv->engines = g_list_delete_link (server->priv->engines, l);

      for (index = 0; index < server->priv->ics->len; index++)
      {
        NimfServiceIC *ic = g_ptr_array_index (server->priv->ics, index);
        nimf_service_ic_unload_engine (ic, engine_id, engine, server);
      }

      g_signal_emit (server, nimf_server_signals[ENGINE_UNLOADED], 0, engine_id);
      g_object_unref (engine);
    }

    l = next;
  }

  g_strfreev (engine_ids);
}

gboolean
nimf_server_start (NimfServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_SERVER (server), FALSE);

  nimf_server_load_services (server);

  server->priv->candidatable = g_hash_table_lookup (server->priv->services,
                                                    "nimf-candidate");
  server->priv->preeditable  = g_hash_table_lookup (server->priv->services,
                                                    "nimf-preedit-window");
  nimf_service_start (NIMF_SERVICE (server->priv->candidatable));
  nimf_service_start (NIMF_SERVICE (server->priv->preeditable));

  nimf_server_load_engines (server);

  GHashTableIter iter;
  gpointer       service;

  g_hash_table_iter_init (&iter, server->priv->services);

  while (g_hash_table_iter_next (&iter, NULL, &service))
  {
    if (!g_strcmp0 (nimf_service_get_id (NIMF_SERVICE (service)), "nimf-candidate"))
      continue;
    else if (!g_strcmp0 (nimf_service_get_id (NIMF_SERVICE (service)), "nimf-preedit-window"))
      continue;

    if (!nimf_service_start (NIMF_SERVICE (service)))
      g_hash_table_iter_remove (&iter);
  }

  g_signal_connect (server->priv->settings, "changed::hotkeys",
                    G_CALLBACK (on_changed_hotkeys), server);
  g_signal_connect (server->priv->settings, "changed::use-singleton",
                    G_CALLBACK (on_use_singleton), server);
  g_signal_connect (server->priv->settings, "changed::hidden-active-engines",
                    G_CALLBACK (on_changed_active_engines), server);
  return TRUE;
}
