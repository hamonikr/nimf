/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/******************************************************************

         Copyright (C) 1994-1995 Sun Microsystems, Inc.
         Copyright (C) 1993-1994 Hewlett-Packard Company
         Copyright (C) 2014 Peng Huang <shawn.p.huang@gmail.com>
         Copyright (C) 2014 Red Hat, Inc.

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and Hewlett-Packard not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and Hewlett-Packard make no representations about
the suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

SUN MICROSYSTEMS INC. AND HEWLETT-PACKARD COMPANY DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
SUN MICROSYSTEMS, INC. AND HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  Author: Hidetoshi Tajima(tajima@Eng.Sun.COM) Sun Microsystems, Inc.

    This version tidied and debugged by Steve Underwood May 1999

******************************************************************/

#include "i18nX.h"
#include "FrameMgr.h"
#include "Xi18n.h"
#include "XimFunc.h"

extern Xi18nClient *_Xi18nFindClient (NimfXim *, CARD16);
extern Xi18nClient *_Xi18nNewClient (NimfXim *);
extern void _Xi18nDeleteClient (NimfXim *, CARD16);
extern unsigned long _Xi18nLookupPropertyOffset (Xi18nOffsetCache *, Atom);
extern void _Xi18nSetPropertyOffset (Xi18nOffsetCache *, Atom, unsigned long);

#define XCM_DATA_LIMIT		20

typedef struct _XClient
{
    Window	client_win;	/* client window */
    Window	accept_win;	/* accept window */
} XClient;

static XClient *NewXClient (NimfXim *xim, Window new_client)
{
    Xi18nClient *client = _Xi18nNewClient (xim);
    XClient *x_client;

    x_client = (XClient *) malloc (sizeof (XClient));
    x_client->client_win = new_client;
    x_client->accept_win = XCreateSimpleWindow (xim->display,
                                                DefaultRootWindow (xim->display),
                                                0, 0, 1, 1, 0, 0, 0);
    client->trans_rec = x_client;
    return ((XClient *) x_client);
}

static unsigned char *ReadXIMMessage (NimfXim *xim,
                                      XClientMessageEvent *ev,
                                      int *connect_id)
{
    Xi18nClient *client = xim->address.clients;
    XClient *x_client = NULL;
    FrameMgr fm;
    extern XimFrameRec packet_header_fr[];
    unsigned char *p = NULL;
    unsigned char *p1;

    while (client != NULL) {
        x_client = (XClient *) client->trans_rec;
        if (x_client->accept_win == ev->window) {
            *connect_id = client->connect_id;
            break;
        }
        client = client->next;
    }

    if (ev->format == 8) {
        /* ClientMessage only */
        XimProtoHdr *hdr = (XimProtoHdr *) ev->data.b;
        unsigned char *rec = (unsigned char *) (hdr + 1);
        register int total_size;
        CARD8 major_opcode;
        CARD8 minor_opcode;
        CARD16 length;
        extern int _Xi18nNeedSwap (NimfXim *, CARD16);

        if (client->byte_order == '?')
        {
            if (hdr->major_opcode != XIM_CONNECT)
                return (unsigned char *) NULL; /* can do nothing */
            client->byte_order = (CARD8) rec[0];
        }

        fm = FrameMgrInit (packet_header_fr,
                           (char *) hdr,
                           _Xi18nNeedSwap (xim, *connect_id));
        total_size = FrameMgrGetTotalSize (fm);
        /* get data */
        FrameMgrGetToken (fm, major_opcode);
        FrameMgrGetToken (fm, minor_opcode);
        FrameMgrGetToken (fm, length);
        FrameMgrFree (fm);

        if ((p = (unsigned char *) malloc (total_size + length * 4)) == NULL)
            return (unsigned char *) NULL;

        p1 = p;
        memmove (p1, &major_opcode, sizeof (CARD8));
        p1 += sizeof (CARD8);
        memmove (p1, &minor_opcode, sizeof (CARD8));
        p1 += sizeof (CARD8);
        memmove (p1, &length, sizeof (CARD16));
        p1 += sizeof (CARD16);
        memmove (p1, rec, length * 4);
    }
    else if (ev->format == 32) {
        /* ClientMessage and WindowProperty */
        unsigned long length = (unsigned long) ev->data.l[0];
        Atom atom = (Atom) ev->data.l[1];
        int return_code;
        Atom actual_type_ret;
        int actual_format_ret;
        unsigned long bytes_after_ret;
        unsigned char *prop;
        unsigned long nitems;
        Xi18nOffsetCache *offset_cache = &client->offset_cache;
        unsigned long offset;
        unsigned long end;
        unsigned long long_begin;
        unsigned long long_end;

        if (length == 0) {
            fprintf (stderr, "%s: invalid length 0\n", __func__);
            return NULL;
        }

        offset = _Xi18nLookupPropertyOffset (offset_cache, atom);
        end = offset + length;

        /* The property data is retrieved in 32-bit chunks */
        long_begin = offset / 4;
        long_end = (end + 3) / 4;
        return_code = XGetWindowProperty (xim->display,
                                          x_client->accept_win,
                                          atom,
                                          long_begin,
                                          long_end - long_begin,
                                          True,
                                          AnyPropertyType,
                                          &actual_type_ret,
                                          &actual_format_ret,
                                          &nitems,
                                          &bytes_after_ret,
                                          &prop);
        if (return_code != Success || actual_format_ret == 0 || nitems == 0) {
            if (return_code == Success)
                XFree (prop);
            fprintf (stderr,
                    "(XIM-IMdkit) ERROR: XGetWindowProperty failed.\n"
                    "Protocol data is likely to be inconsistent.\n");
            _Xi18nSetPropertyOffset (offset_cache, atom, 0);
            return (unsigned char *) NULL;
        }
        /* Update the offset to read next time as needed */
        if (bytes_after_ret > 0)
            _Xi18nSetPropertyOffset (offset_cache, atom, offset + length);
        else
            _Xi18nSetPropertyOffset (offset_cache, atom, 0);
        /* if hit, it might be an error */
        if ((p = (unsigned char *) malloc (length)) == NULL)
            return (unsigned char *) NULL;

        memcpy (p, prop + (offset % 4), length);
        XFree (prop);
    }
    return (unsigned char *) p;
}

void ReadXConnectMessage (NimfXim *xim, XClientMessageEvent *ev)
{
    XEvent event;
    Window new_client = ev->data.l[0];
    CARD32 major_version = ev->data.l[1];
    CARD32 minor_version = ev->data.l[2];
    XClient *x_client = NewXClient (xim, new_client);

    if (ev->window != xim->im_window)
        return; /* incorrect connection request */

    if (major_version != 0  ||  minor_version != 0)
    {
        major_version =
        minor_version = 0;
        /* Only supporting only-CM & Property-with-CM method */
    }
    /*endif*/
    event.xclient.type = ClientMessage;
    event.xclient.display = xim->display;
    event.xclient.window = new_client;
    event.xclient.message_type = xim->_xconnect;
    event.xclient.format = 32;
    event.xclient.data.l[0] = x_client->accept_win;
    event.xclient.data.l[1] = major_version;
    event.xclient.data.l[2] = minor_version;
    event.xclient.data.l[3] = XCM_DATA_LIMIT;

    XSendEvent (xim->display,
                new_client,
                False,
                NoEventMask,
                &event);
    XFlush (xim->display);
}

static char *MakeNewAtom (CARD16 connect_id, char *atomName)
{
    static uint8_t sequence = 0;

    sprintf (atomName,
             "_server%d_%d",
             connect_id,
             ((sequence > 20)  ?  (sequence = 0)  :  (0x1f & sequence)));
    sequence++;
    return atomName;
}

Bool Xi18nXSend (NimfXim *xim,
                 CARD16 connect_id,
                 unsigned char *reply,
                 long length)
{
    Xi18nClient *client = _Xi18nFindClient (xim, connect_id);
    XClient *x_client = (XClient *) client->trans_rec;
    XEvent event;

    event.type = ClientMessage;
    event.xclient.window = x_client->client_win;
    event.xclient.message_type = xim->_protocol;

    if (length > XCM_DATA_LIMIT)
    {
        Atom atom;
        char atomName[16];
        Atom actual_type_ret;
        int actual_format_ret;
        int return_code;
        unsigned long nitems_ret;
        unsigned long bytes_after_ret;
        unsigned char *win_data;

        event.xclient.format = 32;
        atom = XInternAtom (xim->display,
                            MakeNewAtom (connect_id, atomName),
                            False);
        return_code = XGetWindowProperty (xim->display,
                                          x_client->client_win,
                                          atom,
                                          0L,
                                          10000L,
                                          False,
                                          XA_STRING,
                                          &actual_type_ret,
                                          &actual_format_ret,
                                          &nitems_ret,
                                          &bytes_after_ret,
                                          &win_data);
        if (return_code != Success)
            return False;
        /*endif*/
        if (win_data)
            XFree ((char *) win_data);
        /*endif*/
        XChangeProperty (xim->display,
                         x_client->client_win,
                         atom,
                         XA_STRING,
                         8,
                         PropModeAppend,
                         (unsigned char *) reply,
                         length);
        event.xclient.data.l[0] = length;
        event.xclient.data.l[1] = atom;
    }
    else
    {
        unsigned char buffer[XCM_DATA_LIMIT];
        int i;

        event.xclient.format = 8;

        /* Clear unused field with NULL */
        memmove(buffer, reply, length);
        for (i = length; i < XCM_DATA_LIMIT; i++)
            buffer[i] = (char) 0;
        /*endfor*/
        length = XCM_DATA_LIMIT;
        memmove (event.xclient.data.b, buffer, length);
    }
    XSendEvent (xim->display,
                x_client->client_win,
                False,
                NoEventMask,
                &event);
    XFlush (xim->display);
    return True;
}

static Bool CheckCMEvent (Display *display, XEvent *event, XPointer arg)
{
  NimfXim *xim = NIMF_XIM (arg);

  if ((event->type == ClientMessage) &&
    (event->xclient.message_type == xim->_protocol))
    return  True;

  return  False;
}

Bool Xi18nXWait (NimfXim *xim,
                 CARD16 connect_id,
                 CARD8 major_opcode,
                 CARD8 minor_opcode)
{
    XEvent event;
    Xi18nClient *client = _Xi18nFindClient (xim, connect_id);
    XClient *x_client = (XClient *) client->trans_rec;

    for (;;)
    {
        unsigned char *packet;
        XimProtoHdr *hdr;
        int connect_id_ret = 0;

        XIfEvent (xim->display,
                  &event,
                  CheckCMEvent,
                  (XPointer) xim);
        if (event.xclient.window == x_client->accept_win)
        {
            if ((packet = ReadXIMMessage (xim,
                                          (XClientMessageEvent *) & event,
                                          &connect_id_ret))
                == (unsigned char*) NULL)
            {
                return False;
            }
            /*endif*/
            hdr = (XimProtoHdr *)packet;

            if ((hdr->major_opcode == major_opcode)
                &&
                (hdr->minor_opcode == minor_opcode))
            {
                XFree (packet);
                return True;
            }
            else if (hdr->major_opcode == XIM_ERROR)
            {
                XFree (packet);
                return False;
            }

            XFree (packet);
        }
        /*endif*/
    }
    /*endfor*/
}

Bool Xi18nXDisconnect (NimfXim *xim, CARD16 connect_id)
{
    Xi18nClient *client = _Xi18nFindClient (xim, connect_id);
    XClient *x_client = (XClient *) client->trans_rec;

    XDestroyWindow (xim->display, x_client->accept_win);
    XFree (x_client);
    _Xi18nDeleteClient (xim, connect_id);
    return True;
}

Bool WaitXIMProtocol (NimfXim *xim,
                      XEvent  *ev)
{
    extern void _Xi18nMessageHandler (NimfXim *, CARD16, unsigned char *, Bool *);
    Bool delete = True;
    unsigned char *packet;
    int connect_id = 0;

    if (((XClientMessageEvent *) ev)->message_type
        == xim->_protocol)
    {
        if ((packet = ReadXIMMessage (xim,
                                      (XClientMessageEvent *) ev,
                                      &connect_id))
            == (unsigned char *)  NULL)
        {
            return False;
        }
        /*endif*/
        _Xi18nMessageHandler (xim, connect_id, packet, &delete);
        if (delete == True)
            XFree (packet);
        /*endif*/
        return True;
    }
    /*endif*/
    return False;
}
