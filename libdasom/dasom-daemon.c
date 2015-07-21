/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * dasom-daemon.c
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
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include "dasom-events.h"
#include "dasom-types.h"
#include <glib/gi18n.h>
#include <gio/gunixsocketaddress.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "IMdkit/IMdkit.h"
#include "IMdkit/Xi18n.h"
#include "dasom-context.h"
#include "dasom-daemon.h"
#include "dasom-message.h"
#include "dasom-private.h"

/*
  const gchar *desktop = g_getenv ("XDG_CURRENT_DESKTOP");

  if (g_strcmp0 (desktop, "GNOME") == 0)
    ;
*/

G_DEFINE_TYPE (DasomDaemon, dasom_daemon, G_TYPE_OBJECT);

static gint
on_comparing_engine_with_path (DasomEngine *engine, const gchar *path)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  int    retval;
  gchar *engine_path;

  g_object_get (engine, "path", &engine_path, NULL);
  retval = g_strcmp0 (engine_path, path);

  g_free (engine_path);

  return retval;
}

DasomEngine *
dasom_daemon_get_instance (DasomDaemon *daemon,
                           const gchar *module_name)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;
  DasomEngine *engine = NULL;
  gchar *soname = g_strdup_printf ("%s.so", module_name);
  gchar *path = g_build_path (G_DIR_SEPARATOR_S,
                              DASOM_MODULE_DIR, soname, NULL);
  g_free (soname);

  list = g_list_find_custom (g_list_first (daemon->instances),
                             path,
                             (GCompareFunc) on_comparing_engine_with_path);
  g_free (path);

  if (list)
    engine = list->data;

  return engine;
}

DasomEngine *
dasom_daemon_get_next_instance (DasomDaemon *daemon, DasomEngine *engine)
{
  g_debug (G_STRLOC ": %s: arg: %s", G_STRFUNC, dasom_engine_get_name (engine));

  g_assert (engine != NULL);

  GList *list;

  daemon->instances = g_list_first (daemon->instances);

  g_assert (daemon->instances != NULL);


  daemon->instances = g_list_find (daemon->instances, engine);

  g_assert (daemon->instances != NULL);

  list = g_list_next (daemon->instances);

  if (list == NULL)
  {
    g_debug (G_STRLOC ": %s: list == NULL", G_STRFUNC);
    list = g_list_first (daemon->instances);
    g_debug (G_STRLOC ": %s: g_list_first (daemon->instances);", G_STRFUNC);
  }

  if (list)
  {
    engine = list->data;
    daemon->instances = list;
    g_debug (G_STRLOC ": %s: engine name: %s", G_STRFUNC, dasom_engine_get_name (engine));
  }

  g_assert (list != NULL);

  return engine;
}

DasomEngine *
dasom_daemon_get_default_engine (DasomDaemon *daemon)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings *settings = g_settings_new ("org.freedesktop.Dasom");
  gchar *module_name = g_settings_get_string (settings, "default-engine");

  DasomEngine *engine;
  engine = dasom_daemon_get_instance (daemon, module_name);
  g_free (module_name);

  if (engine == NULL)
    engine = dasom_daemon_get_instance (daemon, "dasom-english");

  g_object_unref (settings);

  /* FIXME: 오타, 설정 파일 등으로 인하여 엔진이 NULL 이 나올 수 있습니다.
   * 이에 대한 처리가 필요합니다. */
  g_assert (engine != NULL);

  return engine;
}

static gboolean
on_incoming_message_dasom (GSocket      *socket,
                           GIOCondition  condition,
                           gpointer      user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomMessage *message;
  DasomContext *context = user_data;
  gboolean retval;

  DasomContext *saved_context = context->daemon->target;

  context->daemon->target = context;

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    g_socket_close (socket, NULL);

    dasom_message_unref (context->reply);
    context->reply = NULL;

    if (G_UNLIKELY (context->type == DASOM_CONNECTION_DASOM_AGENT))
      context->daemon->agents_list = g_list_remove (context->daemon->agents_list, context);

    g_hash_table_remove (context->daemon->contexts,
                         GUINT_TO_POINTER (dasom_context_get_id (context)));

    g_debug (G_STRLOC ": %s: condition & (G_IO_HUP | G_IO_ERR)", G_STRFUNC);

    context->daemon->target = saved_context;

    return G_SOURCE_REMOVE;
  }

  message = dasom_recv_message (socket);
  dasom_message_unref (context->reply);
  context->reply = message;

  switch (message->header->type)
  {
    case DASOM_MESSAGE_FILTER_EVENT:
      dasom_message_ref (message);
      retval = dasom_context_filter_event (context,
                                           (const DasomEvent *) message->data);
      dasom_message_unref (message);
      dasom_send_message (socket, DASOM_MESSAGE_FILTER_EVENT_REPLY, &retval,
                          sizeof (gboolean), NULL);
      break;
    case DASOM_MESSAGE_GET_PREEDIT_STRING:
      {
        gchar *data = NULL;
        gint   cursor_pos;
        gint   str_len = 0;

        dasom_context_get_preedit_string (context, &data, &cursor_pos);

        str_len = strlen (data);
        data = g_realloc (data, str_len + 1 + sizeof (gint));
        *(gint *) (data + str_len + 1) = cursor_pos;

        dasom_send_message (socket, DASOM_MESSAGE_GET_PREEDIT_STRING_REPLY,
                            data,
                            str_len + 1 + sizeof (gint),
                            NULL);
        g_free (data);
      }
      break;
    case DASOM_MESSAGE_RESET:
      dasom_context_reset (context);
      dasom_send_message (socket, DASOM_MESSAGE_RESET_REPLY, NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_FOCUS_IN:
      dasom_context_focus_in (context);
      g_debug (G_STRLOC ": %s: context id = %d", G_STRFUNC, dasom_context_get_id (context));
      dasom_send_message (socket, DASOM_MESSAGE_FOCUS_IN_REPLY, NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_FOCUS_OUT:
      dasom_context_focus_out (context);
      g_debug (G_STRLOC ": %s: context id = %d", G_STRFUNC, dasom_context_get_id (context));
      dasom_send_message (socket, DASOM_MESSAGE_FOCUS_OUT_REPLY, NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_SET_SURROUNDING:
      {
        dasom_message_ref (message);
        gchar   *data     = message->data;
        guint16  data_len = message->header->data_len;

        gint   str_len      = data_len - 1 - 2 * sizeof (gint);
        gint   cursor_index = *(gint *) (data + data_len - sizeof (gint));

        dasom_context_set_surrounding (context, data, str_len, cursor_index);
        dasom_message_unref (message);
        dasom_send_message (socket, DASOM_MESSAGE_SET_SURROUNDING_REPLY, NULL, 0, NULL);
      }
      break;
    case DASOM_MESSAGE_GET_SURROUNDING:
      {
        gchar *data;
        gint   cursor_index;
        gint   str_len = 0;

        retval = dasom_context_get_surrounding (context, &data, &cursor_index);

        str_len = strlen (data);
        data = g_realloc (data, str_len + 1 + sizeof (gint) + sizeof (gboolean));
        *(gint *) (data + str_len + 1) = cursor_index;
        *(gboolean *) (data + str_len + 1 + sizeof (gint)) = retval;

        dasom_send_message (socket, DASOM_MESSAGE_GET_SURROUNDING_REPLY,
                            data,
                            str_len + 1 + sizeof (gint) + sizeof (gboolean),
                            NULL);
        g_free (data);
      }
      break;
    case DASOM_MESSAGE_SET_CURSOR_LOCATION:
      dasom_message_ref (message);
      dasom_context_set_cursor_location (context,
                                         (DasomRectangle *) message->data);
      dasom_message_unref (message);
      dasom_send_message (socket, DASOM_MESSAGE_SET_CURSOR_LOCATION_REPLY,
                          NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_SET_USE_PREEDIT:
      dasom_message_ref (message);
      dasom_context_set_use_preedit (context, *(gboolean *) message->data);
      dasom_message_unref (message);
      dasom_send_message (socket, DASOM_MESSAGE_SET_USE_PREEDIT_REPLY,
                          NULL, 0, NULL);
      break;
    case DASOM_MESSAGE_PREEDIT_START_REPLY:
    case DASOM_MESSAGE_PREEDIT_CHANGED_REPLY:
    case DASOM_MESSAGE_PREEDIT_END_REPLY:
    case DASOM_MESSAGE_COMMIT_REPLY:
    case DASOM_MESSAGE_RETRIEVE_SURROUNDING_REPLY:
    case DASOM_MESSAGE_DELETE_SURROUNDING_REPLY:
      break;
    default:
      g_warning ("Unknown message type: %d", message->header->type);
      break;
  }

  context->daemon->target = saved_context;

  return G_SOURCE_CONTINUE;
}

static void
dasom_daemon_init (DasomDaemon *daemon)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings *settings = g_settings_new ("org.freedesktop.Dasom");
  daemon->hotkey_names = g_settings_get_strv (settings, "hotkeys");
  g_object_unref (settings);

  daemon->module_manager = dasom_module_manager_get_default ();
  daemon->instances = dasom_module_manager_create_instances (daemon->module_manager);

  GList *l;
  for (l = daemon->instances; l != NULL; l = l->next)
    {
      DASOM_ENGINE (l->data)->priv->daemon = daemon;
    }

  /* FIXME: daemon->candidate = dasom_candidate_new (); */
  daemon->loop = g_main_loop_new (NULL, FALSE);
  daemon->contexts = g_hash_table_new_full (g_direct_hash,
                                            g_direct_equal,
                                            NULL, /* FIXME */
                                            (GDestroyNotify) g_object_unref);
  daemon->agents_list = NULL;
}

void
dasom_daemon_stop (DasomDaemon *daemon)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_socket_service_stop (daemon->service);

  if (g_main_loop_is_running (daemon->loop))
    g_main_loop_quit (daemon->loop);
}

static void
dasom_daemon_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomDaemon *daemon = DASOM_DAEMON (object);

  g_object_unref (daemon->module_manager);

  if (daemon->instances)
  {
    g_list_free_full (daemon->instances, g_object_unref);
    daemon->instances = NULL;
  }

  /* FIXME: g_object_unref (daemon->candidate); */
  g_hash_table_unref (daemon->contexts);
  g_list_free (daemon->agents_list);
  g_strfreev (daemon->hotkey_names);

  dasom_daemon_stop (daemon);
  g_main_loop_unref (daemon->loop);

  if (daemon->xevent_source)
  {
    g_source_destroy (daemon->xevent_source);
    g_source_unref   (daemon->xevent_source);
  }

  G_OBJECT_CLASS (dasom_daemon_parent_class)->finalize (object);
}

static void
dasom_daemon_class_init (DasomDaemonClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dasom_daemon_finalize;
}

DasomDaemon *
dasom_daemon_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (DASOM_TYPE_DAEMON, NULL);
}

typedef struct
{
  GSource  source;
  Display *display;
  GPollFD  poll_fd;
} DasomXEventSource;

static gboolean dasom_xevent_source_prepare (GSource *source,
                                             gint    *timeout)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  Display *display = ((DasomXEventSource *) source)->display;
  *timeout = -1;
  return XPending (display);
}

static gboolean dasom_xevent_source_check (GSource *source)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomXEventSource *display_source = (DasomXEventSource *) source;

  if (display_source->poll_fd.revents & G_IO_IN)
    return XPending (display_source->display) > 0;
  else
    return FALSE;
}

int dasom_daemon_xim_open (DasomDaemon *daemon, XIMS xims, IMOpenStruct *data)
{
  g_debug (G_STRLOC ": %s, data->connect_id = %d", G_STRFUNC, data->connect_id);

  return 1;
}

int dasom_daemon_xim_close (DasomDaemon *daemon, XIMS xims, IMCloseStruct *data)
{
  g_debug (G_STRLOC ": %s, data->connect_id = %d", G_STRFUNC, data->connect_id);

  return 1;
}

int dasom_daemon_xim_set_ic_values (DasomDaemon      *daemon,
                                    XIMS              xims,
                                    IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomContext *context = g_hash_table_lookup (daemon->contexts,
                                               GUINT_TO_POINTER (data->icid));

  CARD16 i;

  for (i = 0; i < data->ic_attr_num; i++)
  {
    if (g_strcmp0 (XNInputStyle, data->ic_attr[i].name) == 0)
      ; /* XNInputStyle is ignored */
    else if (g_strcmp0 (XNClientWindow, data->ic_attr[i].name) == 0)
      context->client_window = *(Window *) data->ic_attr[i].value;
    else
      g_warning (G_STRLOC ": %s %s", G_STRFUNC, data->ic_attr[i].name);
  }

  for (i = 0; i < data->preedit_attr_num; i++)
  {
    if (g_strcmp0 (XNPreeditState, data->preedit_attr[i].name) == 0)
    {
      XIMPreeditState state = *(XIMPreeditState *) data->preedit_attr[i].value;
      switch (state)
      {
        case XIMPreeditEnable:
          dasom_context_set_use_preedit (context, TRUE);
          break;
        case XIMPreeditDisable:
          dasom_context_set_use_preedit (context, FALSE);
          break;
        default:
          g_message ("XIMPreeditState: %ld is ignored", state);
          break;
      }
    }
    else
      g_critical (G_STRLOC ": %s: %s is ignored",
                  G_STRFUNC, data->preedit_attr[i].name);
  }

  for (i = 0; i < data->status_attr_num; i++)
  {
    g_critical (G_STRLOC ": %s: %s is ignored",
                G_STRFUNC, data->status_attr[i].name);
  }

  dasom_context_xim_set_cursor_location (context, xims);

  return 1;
}

int dasom_daemon_xim_create_ic (DasomDaemon      *daemon,
                                XIMS              xims,
                                IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s, data->icid = %d", G_STRFUNC, data->icid);

  DasomContext *context;
  context = g_hash_table_lookup (daemon->contexts,
                                 GUINT_TO_POINTER ((gint) data->icid));

  if (!context)
  {
    g_debug (G_STRLOC ": %s: context == NULL", G_STRFUNC);

    context = dasom_context_new (DASOM_CONNECTION_XIM,
                                 dasom_daemon_get_default_engine (daemon),
                                 xims);

    g_debug (G_STRLOC ": %s: icid = %d", G_STRFUNC, dasom_context_get_id (context));

    context->xim_connect_id = data->connect_id;
    context->daemon = daemon;

    g_hash_table_insert (daemon->contexts,
                         GUINT_TO_POINTER (dasom_context_get_id (context)),
                         context);
    data->icid = dasom_context_get_id (context);
  }

  dasom_daemon_xim_set_ic_values (daemon, xims, data);

  return 1;
}

int dasom_daemon_xim_destroy_ic (DasomDaemon       *daemon,
                                 XIMS               xims,
                                 IMDestroyICStruct *data)
{
  g_debug (G_STRLOC ": %s, data->icid = %d", G_STRFUNC, data->icid);

  /* FIXME: 클라이언트 프로그램이 정상 종료하지 않을 경우 destroy_ic 함수가
     호출되지 않아 IC가 제거되지 않습니다 */
  /* FIXME: dasom_xevent_source에서 처리가 가능한지 확인해봅시다 */
  return g_hash_table_remove (daemon->contexts,
                              GUINT_TO_POINTER (data->icid));
}

int dasom_daemon_xim_get_ic_values (DasomDaemon      *daemon,
                                    XIMS              xims,
                                    IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomContext *context = g_hash_table_lookup (daemon->contexts,
                                               GUINT_TO_POINTER (data->icid));

  CARD16 i;

  for (i = 0; i < data->ic_attr_num; i++)
  {
    if (g_strcmp0 (XNFilterEvents, data->ic_attr[i].name) == 0)
    {
      data->ic_attr[i].value_length = sizeof (CARD32);
      data->ic_attr[i].value = g_malloc (sizeof (CARD32));
      *(CARD32 *) data->ic_attr[i].value = KeyPressMask | KeyReleaseMask;
    }
    else if (g_strcmp0 (XNSeparatorofNestedList, data->ic_attr[i].name) == 0)
    {
      data->ic_attr[i].value_length = sizeof (CARD16);
      data->ic_attr[i].value = g_malloc (sizeof (CARD16));
      *(CARD16 *) data->ic_attr[i].value = 0;
    }
    else
      g_critical (G_STRLOC ": %s: %s is ignored",
                  G_STRFUNC, data->ic_attr[i].name);
  }

  for (i = 0; i < data->preedit_attr_num; i++)
  {
    if (g_strcmp0 (XNPreeditState, data->preedit_attr[i].name) == 0)
    {
      data->preedit_attr[i].value_length = sizeof (XIMPreeditState);
      data->preedit_attr[i].value = g_malloc (sizeof (XIMPreeditState));

      if (context->use_preedit)
        *(XIMPreeditState *) data->preedit_attr[i].value = XIMPreeditEnable;
      else
        *(XIMPreeditState *) data->preedit_attr[i].value = XIMPreeditDisable;
    }
    else
      g_critical (G_STRLOC ": %s: %s is ignored",
                  G_STRFUNC, data->preedit_attr[i].name);
  }

  for (i = 0; i < data->status_attr_num; i++)
    g_critical (G_STRLOC ": %s: %s is ignored",
                G_STRFUNC, data->status_attr[i].name);

  return 1;
}

int dasom_daemon_xim_set_ic_focus (DasomDaemon         *daemon,
                                   XIMS                 xims,
                                   IMChangeFocusStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomContext *context = g_hash_table_lookup (daemon->contexts,
                                               GUINT_TO_POINTER (data->icid));
  daemon->target = context;
  dasom_context_focus_in (context);
  daemon->target = NULL;

  return 1;
}

int dasom_daemon_xim_unset_ic_focus (DasomDaemon         *daemon,
                                     XIMS                 xims,
                                     IMChangeFocusStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomContext *context = g_hash_table_lookup (daemon->contexts,
                                               GUINT_TO_POINTER (data->icid));
  daemon->target = context;
  dasom_context_focus_out (context);
  daemon->target = NULL;

  return 1;
}

int dasom_daemon_xim_forward_event (DasomDaemon          *daemon,
                                    XIMS                  xims,
                                    IMForwardEventStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  XKeyEvent            *xevent;
  DasomEvent           *event;
  DasomEventType        type;
  IMForwardEventStruct *fw_event;
  gboolean              retval;

  xevent = (XKeyEvent*) &(data->event);

  type = (xevent->type == KeyPress) ? DASOM_EVENT_KEY_PRESS : DASOM_EVENT_KEY_RELEASE;

  event = dasom_event_new (type);

  if (xevent->type == KeyRelease)  event->key.state |= DASOM_RELEASE_MASK;
  if (xevent->state & Button1Mask) event->key.state |= DASOM_BUTTON1_MASK;
  if (xevent->state & Button2Mask) event->key.state |= DASOM_BUTTON2_MASK;
  if (xevent->state & Button3Mask) event->key.state |= DASOM_BUTTON3_MASK;
  if (xevent->state & Button4Mask) event->key.state |= DASOM_BUTTON4_MASK;
  if (xevent->state & Button5Mask) event->key.state |= DASOM_BUTTON5_MASK;
  if (xevent->state & ShiftMask)   event->key.state |= DASOM_SHIFT_MASK;
  if (xevent->state & LockMask)    event->key.state |= DASOM_LOCK_MASK;
  if (xevent->state & ControlMask) event->key.state |= DASOM_CONTROL_MASK;
  if (xevent->state & Mod1Mask)    event->key.state |= DASOM_MOD1_MASK;
  if (xevent->state & Mod2Mask)    event->key.state |= DASOM_MOD2_MASK;
  if (xevent->state & Mod3Mask)    event->key.state |= DASOM_MOD3_MASK;
  if (xevent->state & Mod4Mask)    event->key.state |= DASOM_MOD4_MASK;
  if (xevent->state & Mod5Mask)    event->key.state |= DASOM_MOD5_MASK;
  /* FIXME: I don't know how I process SUPER, HYPER, META */

  /* TODO: test for qwerty and dvorak */
  event->key.keyval = XLookupKeysym (xevent,
                                     (xevent->state & ShiftMask) ? 1 : 0);
  event->key.hardware_keycode = xevent->keycode;

  DasomContext *context = g_hash_table_lookup (daemon->contexts,
                                               GUINT_TO_POINTER (data->icid));
  daemon->target = context;
  retval = dasom_context_filter_event (context, event);
  daemon->target = NULL;
  dasom_event_free (event);

  if (retval)
    return 1;

  /* forward event */
  fw_event = g_slice_new0 (IMForwardEventStruct);
  *fw_event = *data;
  fw_event->sync_bit = 0;

  IMForwardEvent (xims, (XPointer) fw_event);

  g_slice_free (IMForwardEventStruct, fw_event);

  return 1;
}

int dasom_daemon_xim_reset_ic (DasomDaemon     *daemon,
                               XIMS             xims,
                               IMResetICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomContext *context = g_hash_table_lookup (daemon->contexts,
                                               GUINT_TO_POINTER (data->icid));
  daemon->target = context;
  dasom_context_reset (context);
  daemon->target = NULL;

  return 1;
}

static int
on_incoming_message_xim (XIMS        xims,
                         IMProtocol *data,
                         gpointer    user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (xims != NULL, True);
  g_return_val_if_fail (data != NULL, True);

  DasomDaemon *daemon = user_data;
  if (!DASOM_IS_DAEMON (daemon))
    g_error ("ERROR: IMUserData");

  int retval;

  switch (data->major_code)
  {
    case XIM_OPEN:
      retval = dasom_daemon_xim_open (daemon, xims, &data->imopen);
      break;
    case XIM_CLOSE:
      retval = dasom_daemon_xim_close (daemon, xims, &data->imclose);
      break;
    case XIM_CREATE_IC:
      retval = dasom_daemon_xim_create_ic (daemon, xims, &data->changeic);
      break;
    case XIM_DESTROY_IC:
      retval = dasom_daemon_xim_destroy_ic (daemon, xims, &data->destroyic);
      break;
    case XIM_SET_IC_VALUES:
      retval = dasom_daemon_xim_set_ic_values (daemon, xims, &data->changeic);
      break;
    case XIM_GET_IC_VALUES:
      retval = dasom_daemon_xim_get_ic_values (daemon, xims, &data->changeic);
      break;
    case XIM_FORWARD_EVENT:
      retval = dasom_daemon_xim_forward_event (daemon, xims, &data->forwardevent);
      break;
    case XIM_SET_IC_FOCUS:
      retval = dasom_daemon_xim_set_ic_focus (daemon, xims, &data->changefocus);
      break;
    case XIM_UNSET_IC_FOCUS:
      retval = dasom_daemon_xim_unset_ic_focus (daemon, xims, &data->changefocus);
      break;
    case XIM_RESET_IC:
      retval = dasom_daemon_xim_reset_ic (daemon, xims, &data->resetic);
      break;
    default:
      retval = 0;
      break;
  }

  return retval;
}

static gboolean dasom_xevent_source_dispatch (GSource     *source,
                                              GSourceFunc  callback,
                                              gpointer     user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  Display *display = ((DasomXEventSource*) source)->display;
  XEvent   event;

  while (XPending (display))
  {
    XNextEvent (display, &event);
    if (XFilterEvent (&event, None))
      continue;
  }

  return TRUE;
}

static void dasom_xevent_source_finalize (GSource *source)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static GSourceFuncs event_funcs = {
  dasom_xevent_source_prepare,
  dasom_xevent_source_check,
  dasom_xevent_source_dispatch,
  dasom_xevent_source_finalize
};

GSource *
dasom_xevent_source_new (Display *display)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSource *source;
  DasomXEventSource *xevent_source;
  int connection_number;

  source = g_source_new (&event_funcs, sizeof (DasomXEventSource));
  xevent_source = (DasomXEventSource *) source;
  xevent_source->display = display;

  connection_number = ConnectionNumber (xevent_source->display);

  xevent_source->poll_fd.fd = connection_number;
  xevent_source->poll_fd.events = G_IO_IN;
  g_source_add_poll (source, &xevent_source->poll_fd);

  g_source_set_priority (source, G_PRIORITY_DEFAULT);
  g_source_set_can_recurse (source, FALSE);

  return source;
}

static int
on_xerror (Display *display, XErrorEvent *error)
{
  gchar err_msg[64];

  XGetErrorText (display, error->error_code, err_msg, 63);
  g_warning (G_STRLOC ": %s: XError: %s "
    "serial=%lu, error_code=%d request_code=%d minor_code=%d resourceid=%lu",
    G_STRFUNC, err_msg, error->serial, error->error_code, error->request_code,
    error->minor_code, error->resourceid);

  return 1;
}

static gboolean
dasom_daemon_init_xims (DasomDaemon *daemon)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  Display *display;
  Window   window;

  display = XOpenDisplay (NULL);

  if (display == NULL)
    return FALSE;

  XIMStyle ims_styles_on_spot [] = {
    XIMPreeditPosition  | XIMStatusNothing,
    XIMPreeditCallbacks | XIMStatusNothing,
    XIMPreeditNothing   | XIMStatusNothing,
    XIMPreeditPosition  | XIMStatusCallbacks,
    XIMPreeditCallbacks | XIMStatusCallbacks,
    XIMPreeditNothing   | XIMStatusCallbacks,
    0
  };

  XIMEncoding ims_encodings[] = {
      "COMPOUND_TEXT",
      NULL
  };

  XIMStyles styles;
  XIMEncodings encodings;

  styles.count_styles = sizeof (ims_styles_on_spot) / sizeof (XIMStyle) - 1;
  styles.supported_styles = ims_styles_on_spot;

  encodings.count_encodings = sizeof (ims_encodings) / sizeof (XIMEncoding) - 1;
  encodings.supported_encodings = ims_encodings;

  XSetWindowAttributes attrs;

  attrs.event_mask = KeyPressMask | KeyReleaseMask;
  attrs.override_redirect = True;

  window = XCreateWindow (display,      /* Display *display */
                          DefaultRootWindow (display),  /* Window parent */
                          0, 0,         /* int x, y */
                          1, 1,         /* unsigned int width, height */
                          0,            /* unsigned int border_width */
                          0,            /* int depth */
                          InputOutput,  /* unsigned int class */
                          CopyFromParent, /* Visual *visual */
                          CWOverrideRedirect | CWEventMask, /* unsigned long valuemask */
                          &attrs);      /* XSetWindowAttributes *attributes */

  IMOpenIM (display,
            IMModifiers,        "Xi18n",
            IMServerWindow,     window,
            IMServerName,       PACKAGE,
            IMLocale,           "C,en,ko",
            IMServerTransport,  "X/",
            IMInputStyles,      &styles,
            IMEncodingList,     &encodings,
            IMProtocolHandler,  on_incoming_message_xim,
            IMUserData,         daemon,
            IMFilterEventMask,  KeyPressMask | KeyReleaseMask,
            NULL);

  daemon->xevent_source = dasom_xevent_source_new (display);
  g_source_attach (daemon->xevent_source, NULL);
  XSetErrorHandler (on_xerror);

  return TRUE;
}

static gboolean
on_new_connection (GSocketService    *service,
                   GSocketConnection *connection,
                   GObject           *source_object,
                   gpointer           user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  /* TODO: simple authentication */

  /* TODO: agent 처리를 담당할 부분을 따로 만들어주면 좋겠지만,
   * 시간이 걸리므로, 일단은 DaemonContext, on_incoming_message_dasom 에서 처리토록 하자. */

  DasomDaemon *daemon = user_data;

  GSocket *socket = g_socket_connection_get_socket (connection);

  DasomMessage *message;
  message = dasom_recv_message (socket);

  if (message->header->type == DASOM_MESSAGE_CONNECT)
    dasom_send_message (socket, DASOM_MESSAGE_CONNECT_REPLY, NULL, 0, NULL);
  else
  {
    dasom_send_message (socket, DASOM_MESSAGE_ERROR, NULL, 0, NULL);
    return TRUE; /* TODO: return 값을 FALSE 로 하면 어떻 일이 벌어지는가 */
  }

  DasomContext *context;
  context = dasom_context_new (*(DasomConnectionType *) message->data,
                               dasom_daemon_get_default_engine (daemon), NULL);
  dasom_message_unref (message);
  context->daemon = user_data;
  context->socket = socket;

  /* TODO: agent 처리를 담당할 부분을 따로 만들어주면 좋겠지만,
   * 시간이 걸리므로, 일단은 DaemonContext, on_incoming_message_dasom 에서 처리토록 하자. */
  g_hash_table_insert (daemon->contexts,
                       GUINT_TO_POINTER (dasom_context_get_id (context)),
                       context);

  if (context->type == DASOM_CONNECTION_DASOM_AGENT)
    daemon->agents_list = g_list_prepend (daemon->agents_list, context);

  context->source = g_socket_create_source (socket, G_IO_IN, NULL);
  context->connection = g_object_ref (connection);
  g_source_set_can_recurse (context->source, TRUE);
  g_source_set_callback (context->source,
                         (GSourceFunc) on_incoming_message_dasom,
                         context, NULL);
  g_source_attach (context->source, NULL);

  return TRUE;
}

int
dasom_daemon_start (DasomDaemon *daemon)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GError *error = NULL;
  GSocketAddress *address;

  address = g_unix_socket_address_new_with_type ("unix:abstract=dasom",
                                                  -1,
                                                  G_UNIX_SOCKET_ADDRESS_ABSTRACT);
  daemon->service = g_socket_service_new ();
  g_signal_connect (daemon->service,
                    "incoming",
                    (GCallback) on_new_connection,
                    daemon);
  g_socket_listener_add_address (G_SOCKET_LISTENER (daemon->service),
                                 address,
                                 G_SOCKET_TYPE_STREAM,
                                 G_SOCKET_PROTOCOL_DEFAULT,
                                 NULL,
                                 NULL,
                                 &error);
  g_object_unref (address);
  g_socket_service_start (daemon->service);

  if (dasom_daemon_init_xims (daemon) == FALSE)
    g_warning ("Can't Open Display");

  g_main_loop_run (daemon->loop);

  return daemon->status;
}

