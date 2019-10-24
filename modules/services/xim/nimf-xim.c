/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-xim.c
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

#include "nimf-xim.h"
#include "IMdkit/i18nMethod.h"
#include "IMdkit/i18nX.h"
#include "IMdkit/XimFunc.h"

G_DEFINE_DYNAMIC_TYPE (NimfXim, nimf_xim, NIMF_TYPE_SERVICE);

static void
nimf_xim_change_engine (NimfService *service,
                        const gchar *engine_id,
                        const gchar *method_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXim       *xim = NIMF_XIM (service);
  NimfServiceIC *ic;

  ic = g_hash_table_lookup (xim->ics,
                            GUINT_TO_POINTER (xim->last_focused_icid));
  if (ic)
    nimf_service_ic_change_engine (ic, engine_id, method_id);
}

static void
nimf_xim_change_engine_by_id (NimfService *service,
                              const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXim       *xim = NIMF_XIM (service);
  NimfServiceIC *ic;

  ic = g_hash_table_lookup (xim->ics,
                            GUINT_TO_POINTER (xim->last_focused_icid));
  if (ic)
    nimf_service_ic_change_engine_by_id (ic, engine_id);
}

static int nimf_xim_set_ic_values (NimfXim          *xim,
                                   IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIC *ic;
  NimfXimIC     *xic;
  CARD16 i;

  ic = g_hash_table_lookup (xim->ics, GUINT_TO_POINTER (data->icid));
  xic = NIMF_XIM_IC (ic);

  for (i = 0; i < data->ic_attr_num; i++)
  {
    if (!g_strcmp0 (XNInputStyle, data->ic_attr[i].name))
    {
      xic->input_style = *(CARD32*) data->ic_attr[i].value;
      nimf_service_ic_set_use_preedit (ic, !!(xic->input_style & XIMPreeditCallbacks));
    }
    else if (!g_strcmp0 (XNClientWindow, data->ic_attr[i].name))
    {
      xic->client_window = *(Window *) data->ic_attr[i].value;
    }
    else if (!g_strcmp0 (XNFocusWindow, data->ic_attr[i].name))
    {
      xic->focus_window = *(Window *) data->ic_attr[i].value;
    }
    else
    {
      g_warning (G_STRLOC ": %s %s", G_STRFUNC, data->ic_attr[i].name);
    }
  }

  for (i = 0; i < data->preedit_attr_num; i++)
  {
    if (g_strcmp0 (XNPreeditState, data->preedit_attr[i].name) == 0)
    {
      XIMPreeditState state = *(XIMPreeditState *) data->preedit_attr[i].value;
      switch (state)
      {
        case XIMPreeditEnable:
          nimf_service_ic_set_use_preedit (ic, TRUE);
          break;
        case XIMPreeditDisable:
          nimf_service_ic_set_use_preedit (ic, FALSE);
          break;
        case XIMPreeditUnKnown:
          break;
        default:
          g_warning (G_STRLOC ": %s: XIMPreeditState: %ld is ignored",
                     G_STRFUNC, state);
          break;
      }
    }
    else if (g_strcmp0 (XNSpotLocation, data->preedit_attr[i].name) == 0)
    {
      nimf_xim_ic_set_cursor_location (xic,
                                  ((XPoint *) data->preedit_attr[i].value)->x,
                                  ((XPoint *) data->preedit_attr[i].value)->y);
      NimfServer      *server      = nimf_server_get_default ();
      NimfPreeditable *preeditable = nimf_server_get_preeditable (server);
      if (nimf_preeditable_is_visible (preeditable))
        nimf_preeditable_show (preeditable);
    }
    else
    {
      g_critical (G_STRLOC ": %s: %s is ignored",
                  G_STRFUNC, data->preedit_attr[i].name);
    }
  }

  for (i = 0; i < data->status_attr_num; i++)
  {
    g_critical (G_STRLOC ": %s: %s is ignored",
                G_STRFUNC, data->status_attr[i].name);
  }

  return 1;
}

static int nimf_xim_create_ic (NimfXim          *xim,
                               IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s, data->connect_id: %d", G_STRFUNC, data->connect_id);

  NimfXimIC *xic;
  xic = g_hash_table_lookup (xim->ics, GUINT_TO_POINTER (data->icid));

  if (!xic)
  {
    guint16 icid;

    do
      icid = xim->next_icid++;
    while (icid == 0 ||
           g_hash_table_contains (xim->ics, GUINT_TO_POINTER (icid)));

    xic = nimf_xim_ic_new (xim, data->connect_id, icid);
    g_hash_table_insert (xim->ics, GUINT_TO_POINTER (icid), xic);
    data->icid = icid;
    g_debug (G_STRLOC ": icid = %d", data->icid);
  }

  nimf_xim_set_ic_values (xim, data);

  return 1;
}

static int nimf_xim_destroy_ic (NimfXim           *xim,
                                IMDestroyICStruct *data)
{
  g_debug (G_STRLOC ": %s, data->icid = %d", G_STRFUNC, data->icid);

  return g_hash_table_remove (xim->ics, GUINT_TO_POINTER (data->icid));
}

static int nimf_xim_get_ic_values (NimfXim          *xim,
                                   IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIC *ic;
  ic = g_hash_table_lookup (xim->ics, GUINT_TO_POINTER (data->icid));
  CARD16 i;

  for (i = 0; i < data->ic_attr_num; i++)
  {
    if (g_strcmp0 (XNFilterEvents, data->ic_attr[i].name) == 0)
    {
      data->ic_attr[i].value_length = sizeof (CARD32);
      data->ic_attr[i].value = g_malloc (sizeof (CARD32));
      *(CARD32 *) data->ic_attr[i].value = KeyPressMask | KeyReleaseMask;
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

      if (nimf_service_ic_get_use_preedit (ic))
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

static int nimf_xim_forward_event (NimfXim              *xim,
                                   IMForwardEventStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  XKeyEvent        *xevent;
  NimfEvent        *event;
  gboolean          retval;
  KeySym            keysym;
  unsigned int      consumed;
  NimfModifierType  state;

  xevent = (XKeyEvent*) &(data->event);

  event = nimf_event_new (NIMF_EVENT_NOTHING);

  if (xevent->type == KeyPress)
    event->key.type = NIMF_EVENT_KEY_PRESS;
  else
    event->key.type = NIMF_EVENT_KEY_RELEASE;

  event->key.state = (NimfModifierType) xevent->state;
  event->key.keyval = NIMF_KEY_VoidSymbol;
  event->key.hardware_keycode = xevent->keycode;

  XkbLookupKeySym (xim->display,
                   event->key.hardware_keycode,
                   event->key.state,
                   &consumed, &keysym);
  event->key.keyval = (guint) keysym;

  state = event->key.state & ~consumed;
  event->key.state |= (NimfModifierType) state;

  NimfServiceIC *ic;
  ic = g_hash_table_lookup (xim->ics, GUINT_TO_POINTER (data->icid));
  retval = nimf_service_ic_filter_event (ic, event);
  nimf_event_free (event);

  if (G_UNLIKELY (!retval))
    return xi18n_forwardEvent (xim, (XPointer) data);

  return 1;
}

static const gchar *
nimf_xim_get_id (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_SERVICE (service), NULL);

  return NIMF_XIM (service)->id;
}

static int nimf_xim_set_ic_focus (NimfXim             *xim,
                                  IMChangeFocusStruct *data)
{
  NimfServiceIC *ic;
  NimfXimIC     *xic;

  ic  = g_hash_table_lookup (xim->ics, GUINT_TO_POINTER (data->icid));
  xic = NIMF_XIM_IC (ic);

  g_debug (G_STRLOC ": %s, icid = %d, connection id = %d",
           G_STRFUNC, data->icid, xic->icid);

  nimf_service_ic_focus_in (ic);
  xim->last_focused_icid = xic->icid;

  if (xic->input_style & XIMPreeditNothing)
    nimf_xim_ic_set_cursor_location (xic, -1, -1);

  return 1;
}

static int nimf_xim_unset_ic_focus (NimfXim             *xim,
                                    IMChangeFocusStruct *data)
{
  NimfServiceIC *ic;
  ic = g_hash_table_lookup (xim->ics, GUINT_TO_POINTER (data->icid));

  g_debug (G_STRLOC ": %s, icid = %d", G_STRFUNC, data->icid);

  nimf_service_ic_focus_out (ic);

  return 1;
}

static int nimf_xim_reset_ic (NimfXim         *xim,
                              IMResetICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceIC *ic;
  ic = g_hash_table_lookup (xim->ics, GUINT_TO_POINTER (data->icid));
  nimf_service_ic_reset (ic);

  return 1;
}
/* FIXME */
int
on_incoming_message (NimfXim    *xim,
                     IMProtocol *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (data != NULL, True);

  int retval;

  switch (data->major_code)
  {
    case XIM_OPEN:
      g_debug (G_STRLOC ": XIM_OPEN: connect_id: %u", data->imopen.connect_id);
      retval = 1;
      break;
    case XIM_CLOSE:
      g_debug (G_STRLOC ": XIM_CLOSE: connect_id: %u",
               data->imclose.connect_id);
      retval = 1;
      break;
    case XIM_PREEDIT_START_REPLY:
      g_debug (G_STRLOC ": XIM_PREEDIT_START_REPLY");
      retval = 1;
      break;
    case XIM_CREATE_IC:
      retval = nimf_xim_create_ic (xim, &data->changeic);
      break;
    case XIM_DESTROY_IC:
      retval = nimf_xim_destroy_ic (xim, &data->destroyic);
      break;
    case XIM_SET_IC_VALUES:
      retval = nimf_xim_set_ic_values (xim, &data->changeic);
      break;
    case XIM_GET_IC_VALUES:
      retval = nimf_xim_get_ic_values (xim, &data->changeic);
      break;
    case XIM_FORWARD_EVENT:
      retval = nimf_xim_forward_event (xim, &data->forwardevent);
      break;
    case XIM_SET_IC_FOCUS:
      retval = nimf_xim_set_ic_focus (xim, &data->changefocus);
      break;
    case XIM_UNSET_IC_FOCUS:
      retval = nimf_xim_unset_ic_focus (xim, &data->changefocus);
      break;
    case XIM_RESET_IC:
      retval = nimf_xim_reset_ic (xim, &data->resetic);
      break;
    default:
      g_warning (G_STRLOC ": %s: major op code %d not handled", G_STRFUNC,
                 data->major_code);
      retval = 0;
      break;
  }

  return retval;
}

static int
on_xerror (Display     *display,
           XErrorEvent *error)
{
  gchar buf[64];

  XGetErrorText (display, error->error_code, buf, 63);

  g_warning (G_STRLOC ": %s: %s", G_STRFUNC, buf);
  g_warning ("type: %d",         error->type);
  g_warning ("display name: %s", DisplayString (error->display));
  g_warning ("serial: %lu",      error->serial);
  g_warning ("error_code: %d",   error->error_code);
  g_warning ("request_code: %d", error->request_code);
  g_warning ("minor_code: %d",   error->minor_code);
  g_warning ("resourceid: %lu",  error->resourceid);

  return 1;
}

typedef struct
{
  GSource  source;
  NimfXim *xim;
  GPollFD  poll_fd;
} NimfXEventSource;

static gboolean nimf_xevent_source_prepare (GSource *source,
                                            gint    *timeout)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  Display *display = ((NimfXEventSource *) source)->xim->display;
  *timeout = -1;
  return XPending (display) > 0;
}

static gboolean nimf_xevent_source_check (GSource *source)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXEventSource *display_source = (NimfXEventSource *) source;

  if (display_source->poll_fd.revents & G_IO_IN)
    return XPending (display_source->xim->display) > 0;
  else
    return FALSE;
}

static gboolean
nimf_xevent_source_dispatch (GSource     *source,
                             GSourceFunc  callback,
                             gpointer     user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXim *xim = ((NimfXEventSource*) source)->xim;
  XEvent   event;

  while (XPending (xim->display) > 0)
  {
    XNextEvent (xim->display, &event);
    if (!XFilterEvent (&event, None))
    {
      switch (event.type)
      {
        case SelectionRequest:
          WaitXSelectionRequest (xim, &event);
          break;
        case ClientMessage:
          {
            XClientMessageEvent cme = *(XClientMessageEvent *) &event;

            if (cme.message_type == xim->_xconnect)
              ReadXConnectMessage (xim, (XClientMessageEvent *) &event);
            else if (cme.message_type == xim->_protocol)
              WaitXIMProtocol (xim, &event);
            else
              g_warning (G_STRLOC ": %s: ClientMessage type: %ld not handled",
                         G_STRFUNC, cme.message_type);
          }
          break;
        case MappingNotify:
          g_message (G_STRLOC ": %s: MappingNotify", G_STRFUNC);
          break;
        default:
          g_warning (G_STRLOC ": %s: event type: %d not filtered",
                     G_STRFUNC, event.type);
          break;
      }
    }
  }

  return TRUE;
}

static void nimf_xevent_source_finalize (GSource *source)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static GSourceFuncs event_funcs = {
  nimf_xevent_source_prepare,
  nimf_xevent_source_check,
  nimf_xevent_source_dispatch,
  nimf_xevent_source_finalize
};

static GSource *nimf_xevent_source_new (NimfXim *xim)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSource *source;
  NimfXEventSource *xevent_source;

  source = g_source_new (&event_funcs, sizeof (NimfXEventSource));
  xevent_source = (NimfXEventSource *) source;
  xevent_source->xim = xim;

  xevent_source->poll_fd.fd = ConnectionNumber (xevent_source->xim->display);
  xevent_source->poll_fd.events = G_IO_IN;
  g_source_add_poll (source, &xevent_source->poll_fd);

  g_source_set_priority (source, G_PRIORITY_DEFAULT);
  g_source_set_can_recurse (source, TRUE);

  return source;
}

static gboolean nimf_xim_is_active (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_XIM (service)->active;
}

static gboolean nimf_xim_start (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXim *xim = NIMF_XIM (service);

  if (xim->active)
    return TRUE;

  xim->display = XOpenDisplay (NULL);

  if (xim->display == NULL)
  {
    g_warning (G_STRLOC ": %s: Can't open display", G_STRFUNC);
    return FALSE;
  }

/*
 * https://www.x.org/releases/X11R7.7/doc/libX11/libX11/libX11.html
 * https://www.x.org/releases/X11R7.7/doc/libX11/XIM/xim.html
 *
 * The preedit category defines what type of support is provided by the input
 * method for preedit information.
 *
 * XIMPreeditArea      (as known as off-the-spot)
 *
 * The client application provides display windows for the pre-edit data to the
 * input method which displays into them directly.
 * If chosen, the input method would require the client to provide some area
 * values for it to do its preediting. Refer to XIC values XNArea and
 * XNAreaNeeded.
 *
 * XIMPreeditCallbacks (as known as on-the-spot)
 *
 * The client application is directed by the IM Server to display all pre-edit
 * data at the site of text insertion. The client registers callbacks invoked by
 * the input method during pre-editing.
 * If chosen, the input method would require the client to define the set of
 * preedit callbacks. Refer to XIC values XNPreeditStartCallback,
 * XNPreeditDoneCallback, XNPreeditDrawCallback, and XNPreeditCaretCallback.
 *
 * XIMPreeditPosition  (as known as over-the-spot)
 *
 * The input method displays pre-edit data in a window which it brings up
 * directly over the text insertion position.
 * If chosen, the input method would require the client to provide positional
 * values. Refer to XIC values XNSpotLocation and XNFocusWindow.
 *
 * XIMPreeditNothing   (as known as root-window)
 *
 * The input method displays all pre-edit data in a separate area of the screen
 * in a window specific to the input method.
 * If chosen, the input method can function without any preedit values.
 *
 * XIMPreeditNone      none
 *
 * The input method does not provide any preedit feedback. Any preedit value is
 * ignored. This style is mutually exclusive with the other preedit styles.
 *
 *
 * The status category defines what type of support is provided by the input
 * method for status information.
 *
 * XIMStatusArea
 *
 * The input method requires the client to provide some area values for it to do
 * its status feedback. See XNArea and XNAreaNeeded.
 *
 * XIMStatusCallbacks
 *
 * The input method requires the client to define the set of status callbacks,
 * XNStatusStartCallback, XNStatusDoneCallback, and XNStatusDrawCallback.
 *
 * XIMStatusNothing
 *
 * The input method can function without any status values.
 *
 * XIMStatusNone
 *
 * The input method does not provide any status feedback. If chosen, any status
 * value is ignored. This style is mutually exclusive with the other status
 * styles.
 */

  xim->im_styles.count_styles = 6;
  xim->im_styles.supported_styles = g_malloc (sizeof (XIMStyle) * xim->im_styles.count_styles);
  /* on-the-spot */
  xim->im_styles.supported_styles[0] = XIMPreeditCallbacks | XIMStatusNothing;
  xim->im_styles.supported_styles[1] = XIMPreeditCallbacks | XIMStatusNone;
  /* over-the-spot */
  xim->im_styles.supported_styles[2] = XIMPreeditPosition  | XIMStatusNothing;
  xim->im_styles.supported_styles[3] = XIMPreeditPosition  | XIMStatusNone;
  /* root-window */
  xim->im_styles.supported_styles[4] = XIMPreeditNothing   | XIMStatusNothing;
  xim->im_styles.supported_styles[5] = XIMPreeditNothing   | XIMStatusNone;

  xim->im_event_mask = KeyPressMask | KeyReleaseMask;

  XSetWindowAttributes attrs;

  attrs.event_mask = xim->im_event_mask;
  attrs.override_redirect = True;

  xim->im_window = XCreateWindow (xim->display, /* Display *display */
                                  DefaultRootWindow (xim->display),  /* Window parent */
                                  0, 0,         /* int x, y */
                                  1, 1,         /* unsigned int width, height */
                                  0,            /* unsigned int border_width */
                                  0,            /* int depth */
                                  InputOutput,  /* unsigned int class */
                                  CopyFromParent, /* Visual *visual */
                                  CWOverrideRedirect | CWEventMask, /* unsigned long valuemask */
                                  &attrs);      /* XSetWindowAttributes *attributes */

  if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
      xim->byte_order = 'l';
  else
      xim->byte_order = 'B';

  _Xi18nInitAttrList  (xim);
  _Xi18nInitExtension (xim);

  if (!xi18n_openIM (xim, xim->im_window))
  {
    XDestroyWindow (xim->display, xim->im_window);
    XCloseDisplay  (xim->display);
    xim->im_window = 0;
    xim->display = NULL;
    g_warning (G_STRLOC ": %s: XIM is not started.", G_STRFUNC);

    return FALSE;
  }

  xim->xevent_source = nimf_xevent_source_new (xim);
  g_source_attach (xim->xevent_source, NULL);
  XSetErrorHandler (on_xerror);

  xim->active = TRUE;

  return TRUE;
}

static void nimf_xim_stop (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfXim *xim = NIMF_XIM (service);

  if (!xim->active)
    return;

  if (xim->xevent_source)
  {
    g_source_destroy (xim->xevent_source);
    g_source_unref   (xim->xevent_source);
  }

  if (xim->im_window)
  {
    XDestroyWindow (xim->display, xim->im_window);
    xim->im_window = 0;
  }

  g_free (xim->im_styles.supported_styles);

  xi18n_closeIM (xim);

  if (xim->display)
  {
    XCloseDisplay (xim->display);
    xim->display = NULL;
  }

  xim->active = FALSE;
}

static void
nimf_xim_init (NimfXim *xim)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  xim->id  = g_strdup ("nimf-xim");
  xim->ics = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
                                    (GDestroyNotify) g_object_unref);
}

static void nimf_xim_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfService *service = NIMF_SERVICE (object);
  NimfXim     *xim     = NIMF_XIM (object);

  if (nimf_xim_is_active (service))
    nimf_xim_stop (service);

  g_hash_table_unref (xim->ics);
  g_free (xim->id);

  G_OBJECT_CLASS (nimf_xim_parent_class)->finalize (object);
}

static void
nimf_xim_class_init (NimfXimClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass     *object_class  = G_OBJECT_CLASS (class);
  NimfServiceClass *service_class = NIMF_SERVICE_CLASS (class);

  service_class->get_id              = nimf_xim_get_id;
  service_class->start               = nimf_xim_start;
  service_class->stop                = nimf_xim_stop;
  service_class->is_active           = nimf_xim_is_active;
  service_class->change_engine       = nimf_xim_change_engine;
  service_class->change_engine_by_id = nimf_xim_change_engine_by_id;

  object_class->finalize = nimf_xim_finalize;
}

static void
nimf_xim_class_finalize (NimfXimClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_xim_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_xim_get_type ();
}
