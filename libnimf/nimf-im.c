/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-im.c
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

#include "nimf-im.h"
#include <string.h>
#include "nimf-marshalers-private.h"
#include "nimf-message-private.h"
#include <errno.h>
#include <glib/gstdio.h>
#include <gio/gunixsocketaddress.h>
#include "nimf-utils.h"
#include <stdlib.h>

enum {
  PREEDIT_START,
  PREEDIT_END,
  PREEDIT_CHANGED,
  COMMIT,
  RETRIEVE_SURROUNDING,
  DELETE_SURROUNDING,
  BEEP,
  LAST_SIGNAL
};

static guint         im_signals[LAST_SIGNAL] = { 0 };
static GMainContext *nimf_im_context         = NULL;
static GSource      *nimf_im_socket_source   = NULL;
static GSource      *nimf_im_default_source  = NULL;
static GHashTable   *nimf_im_table           = NULL;
static NimfResult   *nimf_im_result          = NULL;
static GSocket      *nimf_im_socket          = NULL;
static gchar        *nimf_im_socket_path     = NULL;

struct _NimfIMPrivate
{
  GObject parent_instance;

  gchar            *preedit_string;
  NimfPreeditAttr **preedit_attrs;
  gint              cursor_pos;

  guint16       id;
  GFileMonitor *monitor;
  uid_t         uid;
  gboolean      created;
};

G_DEFINE_TYPE_WITH_PRIVATE (NimfIM, nimf_im, G_TYPE_OBJECT);

static uid_t
get_login_uid (void)
{
  gchar *nptr;
  gsize  length;
  uid_t  uid;

  if (!g_file_get_contents ("/proc/self/loginuid", &nptr, &length, NULL))
    return -1;

  errno = 0;
  uid = strtol (nptr, NULL, 10);

  if (errno)
    return -1;
  else
    return uid;
}

static gboolean
nimf_im_is_connected ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_im_socket && g_socket_is_connected (nimf_im_socket);
}

static gboolean
on_incoming_message (GSocket      *socket,
                     GIOCondition  condition,
                     gpointer      user_data)
{
  g_debug (G_STRLOC ": %s: socket fd:%d", G_STRFUNC, g_socket_get_fd (socket));

  nimf_message_unref (nimf_im_result->reply);
  nimf_im_result->is_dispatched = TRUE;

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    g_debug (G_STRLOC ": %s: G_IO_HUP | G_IO_ERR", G_STRFUNC);
    /* Because two GSource is created over one socket,
     * when G_IO_HUP | G_IO_ERR, callback can run two times.
     * the following code avoid that callback runs two times. */
    GSource *source = g_main_current_source ();

    if (source == nimf_im_default_source)
      g_source_destroy (nimf_im_socket_source);
    else if (source == nimf_im_socket_source)
      g_source_destroy (nimf_im_default_source);

    if (!g_socket_is_closed (socket))
      g_socket_close (socket, NULL);

    nimf_im_result->reply = NULL;

    gpointer       im;
    GHashTableIter iter;

    g_hash_table_iter_init (&iter, nimf_im_table);

    while (g_hash_table_iter_next (&iter, NULL, &im))
      NIMF_IM (im)->priv->created = FALSE;

    return G_SOURCE_REMOVE;
  }

  NimfMessage *message;
  message = nimf_recv_message (socket);
  nimf_im_result->reply = message;
  gboolean retval;

  if (G_UNLIKELY (message == NULL))
  {
    g_debug (G_STRLOC ": NULL message");
    return G_SOURCE_CONTINUE;
  }

  NimfIM *im;
  im = g_hash_table_lookup (nimf_im_table,
                            GUINT_TO_POINTER (message->header->icid));

  switch (message->header->type)
  {
    /* signals */
    case NIMF_MESSAGE_PREEDIT_START:
      g_signal_emit (im, im_signals[PREEDIT_START], 0);
      nimf_send_message (socket, im->priv->id, NIMF_MESSAGE_PREEDIT_START_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_PREEDIT_END:
      g_signal_emit (im, im_signals[PREEDIT_END], 0);
      nimf_send_message (socket, im->priv->id, NIMF_MESSAGE_PREEDIT_END_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_PREEDIT_CHANGED:
      {
        gint    i;

        g_free (im->priv->preedit_string);
        im->priv->preedit_string = g_strndup (message->data,
                                              message->header->data_len - 1 - sizeof (gint));

        gint str_len = strlen (message->data);
        gint n_attr = (message->header->data_len - str_len - 1 - sizeof (gint)) /
                       sizeof (NimfPreeditAttr);

        nimf_preedit_attr_freev (im->priv->preedit_attrs);
        im->priv->preedit_attrs = g_malloc0_n (n_attr + 1,
                                               sizeof (NimfPreeditAttr *));

        for (i = 0; i < n_attr; i++)
          im->priv->preedit_attrs[i] = g_memdup (message->data + str_len + 1 + i * sizeof (NimfPreeditAttr),
                                                 sizeof (NimfPreeditAttr));

        im->priv->preedit_attrs[n_attr] = NULL;

        im->priv->cursor_pos = *(gint *) (message->data +
                                          message->header->data_len - sizeof (gint));
        g_signal_emit (im, im_signals[PREEDIT_CHANGED], 0);
        nimf_send_message (socket, im->priv->id,
                           NIMF_MESSAGE_PREEDIT_CHANGED_REPLY, NULL, 0, NULL);
      }
      break;
    case NIMF_MESSAGE_COMMIT:
      nimf_message_ref (message);
      g_signal_emit (im, im_signals[COMMIT], 0, (const gchar *) message->data);
      nimf_message_unref (message);
      nimf_send_message (socket, im->priv->id, NIMF_MESSAGE_COMMIT_REPLY,
                         NULL, 0, NULL);
      break;
    case NIMF_MESSAGE_RETRIEVE_SURROUNDING:
      g_signal_emit (im, im_signals[RETRIEVE_SURROUNDING], 0, &retval);
      nimf_send_message (socket, im->priv->id,
                         NIMF_MESSAGE_RETRIEVE_SURROUNDING_REPLY,
                         &retval, sizeof (gboolean), NULL);
      break;
    case NIMF_MESSAGE_DELETE_SURROUNDING:
      nimf_message_ref (message);
      g_signal_emit (im, im_signals[DELETE_SURROUNDING], 0,
                             ((gint *) message->data)[0],
                             ((gint *) message->data)[1], &retval);
      nimf_message_unref (message);
      nimf_send_message (socket, im->priv->id,
                         NIMF_MESSAGE_DELETE_SURROUNDING_REPLY,
                         &retval, sizeof (gboolean), NULL);
      break;
    case NIMF_MESSAGE_BEEP:
      g_signal_emit (im, im_signals[BEEP], 0);
      nimf_send_message (socket, im->priv->id, NIMF_MESSAGE_BEEP_REPLY,
                         NULL, 0, NULL);
      break;
    /* reply */
    case NIMF_MESSAGE_CREATE_CONTEXT_REPLY:
    case NIMF_MESSAGE_DESTROY_CONTEXT_REPLY:
    case NIMF_MESSAGE_FILTER_EVENT_REPLY:
    case NIMF_MESSAGE_RESET_REPLY:
    case NIMF_MESSAGE_FOCUS_IN_REPLY:
    case NIMF_MESSAGE_FOCUS_OUT_REPLY:
    case NIMF_MESSAGE_SET_SURROUNDING_REPLY:
    case NIMF_MESSAGE_SET_CURSOR_LOCATION_REPLY:
    case NIMF_MESSAGE_SET_USE_PREEDIT_REPLY:
      break;
    default:
      g_debug (G_STRLOC ": %s: Unknown message type: %d",
               G_STRFUNC, message->header->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

static void
nimf_im_connect (NimfIM *im)
{
  GMutex mutex;

  g_mutex_init (&mutex);
  g_mutex_lock (&mutex);

  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!nimf_im_is_connected ())
  {
    GSocketAddress *address;
    GStatBuf        info;
    gint            retry_limit = 4;
    gint            retry_count = 0;
    GError         *error = NULL;

    if (nimf_im_socket)
    {
      if (nimf_im_socket_source)
      {
        g_source_destroy (nimf_im_socket_source);
        g_source_unref   (nimf_im_socket_source);
        nimf_im_socket_source = NULL;
      }

      if (nimf_im_default_source)
      {
        g_source_destroy (nimf_im_default_source);
        g_source_unref   (nimf_im_default_source);
        nimf_im_default_source = NULL;
      }

      g_object_unref (nimf_im_socket);
      nimf_im_socket = NULL;
    }

    nimf_im_socket = g_socket_new (G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM,
                                   G_SOCKET_PROTOCOL_DEFAULT, NULL);
    address = g_unix_socket_address_new_with_type (nimf_im_socket_path, -1,
                                                   G_UNIX_SOCKET_ADDRESS_PATH);

    for (retry_count = 0; retry_count < retry_limit; retry_count++)
    {
      if (g_stat (nimf_im_socket_path, &info) == 0)
      {
        if (im->priv->uid == info.st_uid)
        {
          if (g_socket_connect (nimf_im_socket, address, NULL, &error))
          {
            break;
          }
          else
          {
            g_debug (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
            g_clear_error (&error);
          }
        }
        else
        {
          g_debug (G_STRLOC ": %s: Can't authenticate", G_STRFUNC);
          break;
        }
      }

      g_debug ("Trying to execute nimf");

      if (!g_spawn_command_line_sync ("nimf", NULL, NULL, NULL, &error))
      {
        g_debug ("Couldn't execute 'nimf': %s", error->message);
        g_clear_error (&error);
        break;
      }

      g_debug ("Waiting for 1 sec");
      g_usleep (G_USEC_PER_SEC);
    }

    g_object_unref (address);

    if (nimf_im_is_connected ())
    {
      /* when g_main_context_iteration(), iterate only socket */
      nimf_im_socket_source = g_socket_create_source (nimf_im_socket, G_IO_IN, NULL);
      g_source_set_can_recurse (nimf_im_socket_source, TRUE);
      g_source_set_callback (nimf_im_socket_source,
                             (GSourceFunc) on_incoming_message, NULL, NULL);
      g_source_attach (nimf_im_socket_source, nimf_im_context);

      nimf_im_default_source = g_socket_create_source (nimf_im_socket, G_IO_IN, NULL);
      g_source_set_can_recurse (nimf_im_default_source, TRUE);
      g_source_set_callback (nimf_im_default_source,
                             (GSourceFunc) on_incoming_message, NULL, NULL);
      g_source_attach (nimf_im_default_source, NULL);
    }
    else
    {
      g_debug (G_STRLOC ": %s: Couldn't connect to nimf server", G_STRFUNC);
    }
  }

  g_mutex_unlock (&mutex);
}

static void
nimf_im_create_context (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_send_message (nimf_im_socket, im->priv->id,
                     NIMF_MESSAGE_CREATE_CONTEXT, NULL, 0, NULL);
  nimf_result_iteration_until (nimf_im_result, nimf_im_context, im->priv->id,
                               NIMF_MESSAGE_CREATE_CONTEXT_REPLY);
  im->priv->created = TRUE;
}

static void
on_changed (GFileMonitor     *monitor,
            GFile            *file,
            GFile            *other_file,
            GFileMonitorEvent event_type,
            gpointer          user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIM *im = user_data;

  if (event_type == G_FILE_MONITOR_EVENT_CREATED)
  {
    if (!nimf_im_is_connected ())
      nimf_im_connect (im);

    if (nimf_im_is_connected () && !im->priv->created)
      nimf_im_create_context (im);
  }
}

/**
 * nimf_im_focus_out:
 * @im: a #NimfIM
 *
 * Notifies the input method that the caller has lost focus.
 */
void
nimf_im_focus_out (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!NIMF_IS_IM (im) || !nimf_im_is_connected ())
    return;

  nimf_send_message (nimf_im_socket, im->priv->id, NIMF_MESSAGE_FOCUS_OUT,
                     NULL, 0, NULL);
  nimf_result_iteration_until (nimf_im_result, nimf_im_context, im->priv->id,
                               NIMF_MESSAGE_FOCUS_OUT_REPLY);
}

/**
 * nimf_im_set_cursor_location:
 * @im: a #NimfIM
 * @area: new location
 *
 * Notifies the input method that a change in cursor position has been made. The
 * location is the position of a window position in root window coordinates.
 */
void
nimf_im_set_cursor_location (NimfIM              *im,
                             const NimfRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!NIMF_IS_IM (im) || !nimf_im_is_connected ())
    return;

  nimf_send_message (nimf_im_socket, im->priv->id,
                     NIMF_MESSAGE_SET_CURSOR_LOCATION,
                     (gchar *) area, sizeof (NimfRectangle), NULL);
  nimf_result_iteration_until (nimf_im_result, nimf_im_context, im->priv->id,
                               NIMF_MESSAGE_SET_CURSOR_LOCATION_REPLY);
}

/**
 * nimf_im_set_use_preedit:
 * @im: a #NimfIM
 * @use_preedit: whether the input method should use an on-the-spot input style
 *
 * If @use_preedit is %FALSE (default is %TRUE), then the input method may use
 * some other input styles, such as over-the-spot, off-the-spot or root-window.
 */
void
nimf_im_set_use_preedit (NimfIM   *im,
                         gboolean  use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!NIMF_IS_IM (im) || !nimf_im_is_connected ())
    return;

  nimf_send_message (nimf_im_socket, im->priv->id,
                     NIMF_MESSAGE_SET_USE_PREEDIT,
                     (gchar *) &use_preedit, sizeof (gboolean), NULL);
  nimf_result_iteration_until (nimf_im_result, nimf_im_context, im->priv->id,
                               NIMF_MESSAGE_SET_USE_PREEDIT_REPLY);
}

/**
 * nimf_im_set_surrounding:
 * @im: a #NimfIM
 * @text: surrounding text
 * @len: the byte length of @text, or -1 if @text is nul-terminated.
 * @cursor_index: the character index of the cursor within @text.
 *
 * Sets surrounding text to input method. This function is expected to be
 * called in response to NimfIM::retrieve-surrounding which is emitted by
 * nimf_engine_emit_retrieve_surrounding().
 */
void
nimf_im_set_surrounding (NimfIM     *im,
                         const char *text,
                         gint        len,
                         gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!NIMF_IS_IM (im) || !nimf_im_is_connected ())
    return;

  gchar *data = NULL;
  gint   str_len;

  if (len == -1)
    str_len = strlen (text);
  else
    str_len = len;

  data = g_strndup (text, str_len);
  data = g_realloc (data, str_len + 1 + 2 * sizeof (gint));

  *(gint *) (data + str_len + 1) = len;
  *(gint *) (data + str_len + 1 + sizeof (gint)) = cursor_index;

  nimf_send_message (nimf_im_socket, im->priv->id,
                     NIMF_MESSAGE_SET_SURROUNDING,
                     data, str_len + 1 + 2 * sizeof (gint), g_free);
  nimf_result_iteration_until (nimf_im_result, nimf_im_context, im->priv->id,
                               NIMF_MESSAGE_SET_SURROUNDING_REPLY);
}

/**
 * nimf_im_focus_in:
 * @im: a #NimfIM
 *
 * Notifies the input method that the caller has gained focus.
 */
void
nimf_im_focus_in (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!NIMF_IS_IM (im) || !nimf_im_is_connected ())
    return;

  nimf_send_message (nimf_im_socket, im->priv->id,
                     NIMF_MESSAGE_FOCUS_IN, NULL, 0, NULL);
  nimf_result_iteration_until (nimf_im_result, nimf_im_context, im->priv->id,
                               NIMF_MESSAGE_FOCUS_IN_REPLY);
}

/**
 * nimf_im_get_preedit_string:
 * @im:         a #NimfIM
 * @str:        (out) (transfer full): location to store the retrieved
 *              string. The string retrieved must be freed with g_free().
 * @attrs:      (out) (transfer full): location to store the retrieved
 *              attribute array. When you are done with this array, you
 *              must free it with nimf_preedit_attr_freev().
 * @cursor_pos: (out) (transfer full): location to store position of cursor (in
 *              characters) within the preedit string.
 *
 * Retrieve the current preedit string, an array of attributes to apply to the
 * string and position of cursor within the preedit string from the input
 * method.
 */
void
nimf_im_get_preedit_string (NimfIM            *im,
                            gchar            **str,
                            NimfPreeditAttr ***attrs,
                            gint              *cursor_pos)
{
  g_debug (G_STRLOC ":%s", G_STRFUNC);

  if (!NIMF_IS_IM (im))
    return;

  if (str)
    *str = g_strdup (im->priv->preedit_string);

  if (attrs)
    *attrs = nimf_preedit_attrs_copy (im->priv->preedit_attrs);

  if (cursor_pos)
    *cursor_pos = im->priv->cursor_pos;
}

/**
 * nimf_im_reset:
 * @im: a #NimfIM
 *
 * Reset the input method.
 */
void
nimf_im_reset (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!NIMF_IS_IM (im) || !nimf_im_is_connected ())
    return;

  nimf_send_message (nimf_im_socket, im->priv->id,
                     NIMF_MESSAGE_RESET, NULL, 0, NULL);
  nimf_result_iteration_until (nimf_im_result, nimf_im_context, im->priv->id,
                               NIMF_MESSAGE_RESET_REPLY);
}

/**
 * nimf_im_filter_event:
 * @im: a #NimfIM
 * @event: a #NimfEvent
 *
 * Let the input method handle the @event.
 *
 * Returns: %TRUE if the input method handled the @event.
 */
gboolean
nimf_im_filter_event (NimfIM    *im,
                      NimfEvent *event)
{
  g_debug (G_STRLOC ":%s", G_STRFUNC);

  if (!NIMF_IS_IM (im) || !nimf_im_is_connected ())
    return FALSE;

  nimf_send_message (nimf_im_socket, im->priv->id, NIMF_MESSAGE_FILTER_EVENT,
                     event, sizeof (NimfEvent), NULL);
  nimf_result_iteration_until (nimf_im_result, nimf_im_context,
                               im->priv->id, NIMF_MESSAGE_FILTER_EVENT_REPLY);

  if (nimf_im_result->reply &&
      *(gboolean *) (nimf_im_result->reply->data))
    return TRUE;

  return FALSE;
}

/**
 * nimf_im_new:
 *
 * Creates a new #NimfIM.
 *
 * Returns: a new #NimfIM
 */
NimfIM *
nimf_im_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (NIMF_TYPE_IM, NULL);
}

static void
nimf_im_init (NimfIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  im->priv = nimf_im_get_instance_private (im);

  static guint16 next_id = 0;
  guint16 id;

  if ((im->priv->uid = get_login_uid ()) == (uid_t) -1)
    im->priv->uid = getuid ();

  if (!nimf_im_socket_path)
    nimf_im_socket_path = nimf_get_socket_path ();

  if (nimf_im_context == NULL)
    nimf_im_context = g_main_context_new ();
  else
    g_main_context_ref (nimf_im_context);

  if (nimf_im_result == NULL)
    nimf_im_result = nimf_result_new ();

  if (nimf_im_table == NULL)
    nimf_im_table = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                           NULL, NULL);
  else
    g_hash_table_ref (nimf_im_table);

  if (im->priv->monitor == NULL)
  {
    GFile *file;

    file = g_file_new_for_path (nimf_im_socket_path);
    im->priv->monitor = g_file_monitor_file (file, G_FILE_MONITOR_NONE, NULL, NULL);
    g_file_monitor_set_rate_limit (im->priv->monitor, G_PRIORITY_LOW);
    g_signal_connect (im->priv->monitor, "changed", G_CALLBACK (on_changed), im);

    g_object_unref (file);
  }

  do {
    id = next_id++;
  } while (id == 0 || g_hash_table_contains (nimf_im_table,
                                             GUINT_TO_POINTER (id)));

  im->priv->id = id;

  g_hash_table_insert (nimf_im_table, GUINT_TO_POINTER (im->priv->id), im);

  if (!nimf_im_is_connected ())
    nimf_im_connect (im);

  if (nimf_im_is_connected () && !im->priv->created)
    nimf_im_create_context (im);

  im->priv->preedit_string = g_strdup ("");
  im->priv->preedit_attrs  = g_malloc0_n (1, sizeof (NimfPreeditAttr *));
}

static void
nimf_im_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfIM *im = NIMF_IM (object);

  if (nimf_im_is_connected () && im->priv->created)
  {
    nimf_send_message (nimf_im_socket, im->priv->id,
                       NIMF_MESSAGE_DESTROY_CONTEXT, NULL, 0, NULL);
    nimf_result_iteration_until (nimf_im_result, nimf_im_context, im->priv->id,
                                 NIMF_MESSAGE_DESTROY_CONTEXT_REPLY);
  }

  g_hash_table_remove (nimf_im_table, GUINT_TO_POINTER (im->priv->id));
  g_main_context_unref (nimf_im_context);

  if (im->priv->monitor)
    g_object_unref (im->priv->monitor);

  if (g_hash_table_size (nimf_im_table) == 0)
  {
    g_hash_table_unref (nimf_im_table);
    nimf_result_unref  (nimf_im_result);

    if (nimf_im_socket)
      g_object_unref (nimf_im_socket);

    if (nimf_im_socket_source)
      g_source_destroy (nimf_im_socket_source);

    if (nimf_im_socket_source)
      g_source_unref   (nimf_im_socket_source);

    if (nimf_im_default_source)
      g_source_destroy (nimf_im_default_source);

    if (nimf_im_default_source)
      g_source_unref   (nimf_im_default_source);

    g_free (nimf_im_socket_path);

    nimf_im_socket         = NULL;
    nimf_im_socket_source  = NULL;
    nimf_im_default_source = NULL;
    nimf_im_context        = NULL;
    nimf_im_result         = NULL;
    nimf_im_table          = NULL;
    nimf_im_socket_path    = NULL;
  }

  g_free (im->priv->preedit_string);
  nimf_preedit_attr_freev (im->priv->preedit_attrs);

  G_OBJECT_CLASS (nimf_im_parent_class)->finalize (object);
}

static void
nimf_im_class_init (NimfIMClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = nimf_im_finalize;

  /**
   * NimfIM::preedit-start:
   * @im: a #NimfIM
   *
   * The #NimfIM::preedit-start signal is emitted when a new preediting
   * sequence starts.
   */
  im_signals[PREEDIT_START] =
    g_signal_new (g_intern_static_string ("preedit-start"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, preedit_start),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * NimfIM::preedit-end:
   * @im: a #NimfIM
   *
   * The #NimfIM::preedit-end signal is emitted when a preediting sequence has
   * been completed or canceled.
   */
  im_signals[PREEDIT_END] =
    g_signal_new (g_intern_static_string ("preedit-end"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, preedit_end),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * NimfIM::preedit-changed:
   * @im: a #NimfIM
   *
   * The #NimfIM::preedit-changed signal is emitted whenever the preedit
   * sequence has been changed.
   */
  im_signals[PREEDIT_CHANGED] =
    g_signal_new (g_intern_static_string ("preedit-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, preedit_changed),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * NimfIM::commit:
   * @im: a #NimfIM
   * @text: text to commit
   *
   * The #NimfIM::commit signal is emitted when a complete input sequence has
   * been entered by the user.
   */
  im_signals[COMMIT] =
    g_signal_new (g_intern_static_string ("commit"),
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, commit),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  /**
   * NimfIM::retrieve-surrounding:
   * @im: a #NimfIM
   *
   * The #NimfIM::retrieve-surrounding signal is emitted when the input method
   * requires the text surrounding the cursor. The callback should set the
   * input method surrounding text by calling the nimf_im_set_surrounding()
   * method.
   *
   * Returns: %TRUE if the signal was handled.
   */
  im_signals[RETRIEVE_SURROUNDING] =
    g_signal_new (g_intern_static_string ("retrieve-surrounding"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, retrieve_surrounding),
                  g_signal_accumulator_true_handled, NULL,
                  nimf_cclosure_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);

  /**
   * NimfIM::delete-surrounding:
   * @im: a #NimfIM
   * @offset:  the character offset from the cursor position of the text to be
   *           deleted. A negative value indicates a position before the cursor.
   * @n_chars: the number of characters to be deleted
   *
   * The #NimfIM::delete-surrounding signal is emitted when the input method
   * needs to delete all or part of the text surrounding the cursor.
   *
   * Returns: %TRUE if the signal was handled.
   */
  im_signals[DELETE_SURROUNDING] =
    g_signal_new (g_intern_static_string ("delete-surrounding"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, delete_surrounding),
                  g_signal_accumulator_true_handled, NULL,
                  nimf_cclosure_marshal_BOOLEAN__INT_INT,
                  G_TYPE_BOOLEAN, 2,
                  G_TYPE_INT,
                  G_TYPE_INT);

  /**
   * NimfIM::beep:
   * @im: a #NimfIM
   *
   * The #NimfIM::beep signal is emitted when the input method needs to beep,
   * if supported.
   */
  im_signals[BEEP] =
    g_signal_new (g_intern_static_string ("beep"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NimfIMClass, beep),
                  NULL, NULL,
                  nimf_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}
