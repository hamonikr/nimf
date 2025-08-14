/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-mozc.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2024 Nimf Project
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

#include <nimf.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define NIMF_TYPE_MOZC             (nimf_mozc_get_type ())
#define NIMF_MOZC(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_MOZC, NimfMozc))
#define NIMF_MOZC_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_MOZC, NimfMozcClass))
#define NIMF_IS_MOZC(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_MOZC))
#define NIMF_IS_MOZC_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_MOZC))
#define NIMF_MOZC_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_MOZC, NimfMozcClass))

typedef struct _NimfMozc      NimfMozc;
typedef struct _NimfMozcClass NimfMozcClass;

struct _NimfMozc
{
  NimfEngine parent_instance;

  NimfCandidatable  *candidatable;
  GString           *preedit;
  NimfPreeditState   preedit_state;
  NimfPreeditAttr  **preedit_attrs;
  gchar             *id;
  GSettings         *settings;
  NimfKey          **hiragana_keys;
  NimfKey          **katakana_keys;
  
  /* Mozc specific */
  GSocketConnection *connection;
  gboolean           mozc_server_running;
  gchar             *input_mode;
  gint               current_page;
  gint               n_pages;
  GPtrArray         *candidates;
};

struct _NimfMozcClass
{
  NimfEngineClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE (NimfMozc, nimf_mozc, NIMF_TYPE_ENGINE);

static gboolean nimf_mozc_start_server (NimfMozc *mozc);
static void nimf_mozc_stop_server (NimfMozc *mozc);
static gboolean nimf_mozc_send_command (NimfMozc *mozc, const gchar *command);

static void
nimf_mozc_update_preedit_state (NimfEngine    *engine,
                                NimfServiceIC *target,
                                const gchar   *new_preedit,
                                gint           cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMozc *mozc = NIMF_MOZC (engine);

  if (mozc->preedit_state == NIMF_PREEDIT_STATE_END &&
      mozc->preedit->len > 0)
  {
    mozc->preedit_state = NIMF_PREEDIT_STATE_START;
    nimf_engine_emit_preedit_start (engine, target);
  }

  nimf_engine_emit_preedit_changed (engine, target, new_preedit,
                                    mozc->preedit_attrs, cursor_pos);

  if (!nimf_service_ic_get_use_preedit (target))
    nimf_candidatable_set_auxiliary_text (mozc->candidatable,
                                          mozc->preedit->str,
                                          g_utf8_strlen (mozc->preedit->str, -1));

  if (mozc->preedit_state == NIMF_PREEDIT_STATE_START &&
      mozc->preedit->len == 0)
  {
    mozc->preedit_state = NIMF_PREEDIT_STATE_END;
    nimf_engine_emit_preedit_end (engine, target);
  }
}

static void
nimf_mozc_emit_commit (NimfEngine    *engine,
                       NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMozc *mozc = NIMF_MOZC (engine);

  if (mozc->preedit->len > 0)
  {
    nimf_engine_emit_commit (engine, target, mozc->preedit->str);
    g_string_assign (mozc->preedit, "");
    mozc->preedit_attrs[0]->start_index = 0;
    mozc->preedit_attrs[0]->end_index   = 0;
    nimf_mozc_update_preedit_state (engine, target, "", 0);
  }
}

void
nimf_mozc_reset (NimfEngine    *engine,
                 NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMozc *mozc = NIMF_MOZC (engine);
  
  nimf_candidatable_hide (mozc->candidatable);
  nimf_mozc_emit_commit (engine, target);
  
  if (mozc->mozc_server_running)
    nimf_mozc_send_command (mozc, "RESET");
}

void
nimf_mozc_focus_in (NimfEngine    *engine,
                    NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMozc *mozc = NIMF_MOZC (engine);
  
  if (!mozc->mozc_server_running)
    nimf_mozc_start_server (mozc);
}

void
nimf_mozc_focus_out (NimfEngine    *engine,
                     NimfServiceIC *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_candidatable_hide (NIMF_MOZC (engine)->candidatable);
  nimf_mozc_reset (engine, target);
}

static gboolean
nimf_mozc_start_server (NimfMozc *mozc)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  
  if (mozc->mozc_server_running)
    return TRUE;
    
  /* Start mozc_server process */
  gchar *argv[] = {"mozc_server", NULL};
  GPid pid;
  GError *error = NULL;
  
  if (!g_spawn_async (NULL, argv, NULL, 
                      G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                      NULL, NULL, &pid, &error))
  {
    g_warning ("Failed to start mozc_server: %s", error->message);
    g_error_free (error);
    return FALSE;
  }
  
  /* Give some time for server to start */
  g_usleep (100000); /* 100ms */
  
  mozc->mozc_server_running = TRUE;
  return TRUE;
}

static void
nimf_mozc_stop_server (NimfMozc *mozc)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  
  if (!mozc->mozc_server_running)
    return;
    
  if (mozc->connection)
  {
    g_object_unref (mozc->connection);
    mozc->connection = NULL;
  }
  
  mozc->mozc_server_running = FALSE;
}

static gboolean
nimf_mozc_send_command (NimfMozc *mozc, const gchar *command)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  
  /* This is a simplified implementation */
  /* In a real implementation, you would need to communicate with mozc_server */
  /* using its protocol (usually IPC or named pipes) */
  
  return TRUE;
}

static gboolean
nimf_mozc_filter_event (NimfEngine    *engine,
                        NimfServiceIC *target,
                        NimfEvent     *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMozc *mozc = NIMF_MOZC (engine);

  if (event->key.type == NIMF_EVENT_KEY_RELEASE)
    return FALSE;

  if (!mozc->mozc_server_running)
  {
    if (!nimf_mozc_start_server (mozc))
      return FALSE;
  }

  /* Handle special keys */
  if (nimf_candidatable_is_visible (mozc->candidatable))
  {
    switch (event->key.keyval)
    {
      case NIMF_KEY_Return:
      case NIMF_KEY_KP_Enter:
        nimf_candidatable_hide (mozc->candidatable);
        nimf_mozc_emit_commit (engine, target);
        return TRUE;
      case NIMF_KEY_Escape:
        nimf_candidatable_hide (mozc->candidatable);
        return TRUE;
      case NIMF_KEY_space:
      case NIMF_KEY_Down:
        nimf_candidatable_select_next_item (mozc->candidatable);
        return TRUE;
      case NIMF_KEY_Up:
        nimf_candidatable_select_previous_item (mozc->candidatable);
        return TRUE;
      default:
        break;
    }
  }

  /* Handle mode conversion keys */
  if (G_UNLIKELY (nimf_event_matches (event, (const NimfKey **) mozc->hiragana_keys)))
  {
    g_free (mozc->input_mode);
    mozc->input_mode = g_strdup ("hiragana");
    return TRUE;
  }
  else if (G_UNLIKELY (nimf_event_matches (event, (const NimfKey **) mozc->katakana_keys)))
  {
    g_free (mozc->input_mode);
    mozc->input_mode = g_strdup ("katakana");
    return TRUE;
  }

  /* Handle regular input */
  if (event->key.keyval >= 0x20 && event->key.keyval < 0x7F)
  {
    gchar input_char = (gchar) event->key.keyval;
    
    /* For demonstration, we'll just add to preedit */
    /* In real implementation, this should go through Mozc conversion */
    g_string_append_c (mozc->preedit, input_char);
    
    nimf_mozc_update_preedit_state (engine, target, mozc->preedit->str,
                                    g_utf8_strlen (mozc->preedit->str, -1));
    return TRUE;
  }

  /* Handle space for conversion */
  if (event->key.keyval == NIMF_KEY_space && mozc->preedit->len > 0)
  {
    /* Show dummy candidates for demonstration */
    nimf_candidatable_clear (mozc->candidatable, target);
    nimf_candidatable_append (mozc->candidatable, "変換1", NULL);
    nimf_candidatable_append (mozc->candidatable, "変換2", NULL);
    nimf_candidatable_append (mozc->candidatable, "変換3", NULL);
    
    if (!nimf_candidatable_is_visible (mozc->candidatable))
      nimf_candidatable_show (mozc->candidatable, target,
                              !nimf_service_ic_get_use_preedit (target));
    
    return TRUE;
  }

  /* Handle backspace */
  if (event->key.keyval == NIMF_KEY_BackSpace)
  {
    if (mozc->preedit->len > 0)
    {
      g_string_truncate (mozc->preedit, mozc->preedit->len - 1);
      nimf_mozc_update_preedit_state (engine, target, mozc->preedit->str,
                                      g_utf8_strlen (mozc->preedit->str, -1));
      return TRUE;
    }
  }

  /* Handle Enter */
  if (event->key.keyval == NIMF_KEY_Return)
  {
    if (mozc->preedit->len > 0)
    {
      nimf_mozc_emit_commit (engine, target);
      return TRUE;
    }
  }

  return FALSE;
}

static void
on_changed_keys (GSettings *settings,
                 gchar     *key,
                 NimfMozc  *mozc)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **keys = g_settings_get_strv (settings, key);

  if (g_strcmp0 (key, "hiragana-keys") == 0)
  {
    nimf_key_freev (mozc->hiragana_keys);
    mozc->hiragana_keys = nimf_key_newv ((const gchar **) keys);
  }
  else if (g_strcmp0 (key, "katakana-keys") == 0)
  {
    nimf_key_freev (mozc->katakana_keys);
    mozc->katakana_keys = nimf_key_newv ((const gchar **) keys);
  }

  g_strfreev (keys);
}

static void
nimf_mozc_init (NimfMozc *mozc)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **hiragana_keys;
  gchar **katakana_keys;

  mozc->id            = g_strdup ("nimf-mozc");
  mozc->preedit       = g_string_new ("");
  mozc->preedit_attrs = g_malloc0_n (2, sizeof (NimfPreeditAttr *));
  mozc->preedit_attrs[0] = nimf_preedit_attr_new (NIMF_PREEDIT_ATTR_UNDERLINE, 0, 0);
  mozc->preedit_attrs[1] = NULL;
  mozc->candidates    = g_ptr_array_new_with_free_func (g_free);
  mozc->input_mode    = g_strdup ("hiragana");
  mozc->mozc_server_running = FALSE;
  mozc->connection    = NULL;

  mozc->settings = g_settings_new ("org.nimf.engines.nimf-mozc");
  hiragana_keys = g_settings_get_strv (mozc->settings, "hiragana-keys");
  katakana_keys = g_settings_get_strv (mozc->settings, "katakana-keys");
  mozc->hiragana_keys = nimf_key_newv ((const gchar **) hiragana_keys);
  mozc->katakana_keys = nimf_key_newv ((const gchar **) katakana_keys);

  g_strfreev (hiragana_keys);
  g_strfreev (katakana_keys);

  g_signal_connect_data (mozc->settings, "changed::hiragana-keys",
          G_CALLBACK (on_changed_keys), mozc, NULL, G_CONNECT_AFTER);
  g_signal_connect_data (mozc->settings, "changed::katakana-keys",
          G_CALLBACK (on_changed_keys), mozc, NULL, G_CONNECT_AFTER);
}

static void
nimf_mozc_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMozc *mozc = NIMF_MOZC (object);

  nimf_mozc_stop_server (mozc);
  
  nimf_preedit_attr_freev (mozc->preedit_attrs);
  g_free (mozc->id);
  g_string_free (mozc->preedit, TRUE);
  nimf_key_freev (mozc->hiragana_keys);
  nimf_key_freev (mozc->katakana_keys);
  g_free (mozc->input_mode);
  g_object_unref (mozc->settings);
  g_ptr_array_unref (mozc->candidates);

  G_OBJECT_CLASS (nimf_mozc_parent_class)->finalize (object);
}

const gchar *
nimf_mozc_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_MOZC (engine)->id;
}

const gchar *
nimf_mozc_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_MOZC (engine)->id;
}

static void
nimf_mozc_constructed (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfMozc *mozc = NIMF_MOZC (object);

  mozc->candidatable = nimf_engine_get_candidatable (NIMF_ENGINE (mozc));
}

static void
nimf_mozc_class_init (NimfMozcClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  NimfEngineClass *engine_class = NIMF_ENGINE_CLASS (class);

  engine_class->filter_event       = nimf_mozc_filter_event;
  engine_class->reset              = nimf_mozc_reset;
  engine_class->focus_in           = nimf_mozc_focus_in;
  engine_class->focus_out          = nimf_mozc_focus_out;

  engine_class->get_id        = nimf_mozc_get_id;
  engine_class->get_icon_name = nimf_mozc_get_icon_name;

  object_class->constructed = nimf_mozc_constructed;
  object_class->finalize    = nimf_mozc_finalize;
}

static void
nimf_mozc_class_finalize (NimfMozcClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_mozc_register_type (type_module);
}

GType
module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_mozc_get_type ();
}