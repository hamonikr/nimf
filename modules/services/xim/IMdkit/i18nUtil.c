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

#include <X11/Xlib.h>
#include "Xi18n.h"
#include "FrameMgr.h"
#include "XimFunc.h"
#include "nimf-xim.h"
#include "i18nX.h"

Xi18nClient *_Xi18nFindClient (NimfXim *, CARD16);
void _Xi18nInitOffsetCache (Xi18nOffsetCache *);

int
_Xi18nNeedSwap (NimfXim *xim, CARD16 connect_id)
{
    Xi18nClient *client = _Xi18nFindClient (xim, connect_id);

    return (client->byte_order != xim->byte_order);
}

Xi18nClient *_Xi18nNewClient (NimfXim *xim)
{
    static CARD16 connect_id = 0;
    int new_connect_id;
    Xi18nClient *client;

    if (xim->address.free_clients)
    {
        client = xim->address.free_clients;
        xim->address.free_clients = client->next;
	new_connect_id = client->connect_id;
    }
    else
    {
        client = (Xi18nClient *) malloc (sizeof (Xi18nClient));
	new_connect_id = ++connect_id;
    }
    /*endif*/
    memset (client, 0, sizeof (Xi18nClient));
    client->connect_id = new_connect_id;
    client->pending = (XIMPending *) NULL;
    client->sync = False;
    client->byte_order = '?'; 	/* initial value */
    memset (&client->pending, 0, sizeof (XIMPending *));
    _Xi18nInitOffsetCache (&client->offset_cache);
    client->next = xim->address.clients;
    xim->address.clients = client;

    return (Xi18nClient *) client;
}

Xi18nClient *_Xi18nFindClient (NimfXim *xim, CARD16 connect_id)
{
    Xi18nClient *client = xim->address.clients;

    while (client)
    {
        if (client->connect_id == connect_id)
            return client;
        /*endif*/
        client = client->next;
    }
    /*endwhile*/
    return NULL;
}

void _Xi18nDeleteClient (NimfXim *xim, CARD16 connect_id)
{
    Xi18nClient *target = _Xi18nFindClient (xim, connect_id);
    Xi18nClient *ccp;
    Xi18nClient *ccp0;

    for (ccp = xim->address.clients, ccp0 = NULL;
         ccp != NULL;
         ccp0 = ccp, ccp = ccp->next)
    {
        if (ccp == target)
        {
            if (ccp0 == NULL)
                xim->address.clients = ccp->next;
            else
                ccp0->next = ccp->next;
            /*endif*/
            /* put it back to free list */
            target->next = xim->address.free_clients;
            xim->address.free_clients = target;
            return;
        }
        /*endif*/
    }
    /*endfor*/
}

void _Xi18nSendMessage (NimfXim *xim,
                        CARD16 connect_id,
                        CARD8 major_opcode,
                        CARD8 minor_opcode,
                        unsigned char *data,
                        long length)
{
    FrameMgr fm;
    extern XimFrameRec packet_header_fr[];
    unsigned char *reply_hdr = NULL;
    int header_size;
    unsigned char *reply = NULL;
    unsigned char *replyp;
    int reply_length;
    long p_len = length/4;

    fm = FrameMgrInit (packet_header_fr,
                       NULL,
                       _Xi18nNeedSwap (xim, connect_id));

    header_size = FrameMgrGetTotalSize (fm);
    reply_hdr = (unsigned char *) malloc (header_size);
    if (reply_hdr == NULL)
    {
        _Xi18nSendMessage (xim, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    FrameMgrSetBuffer (fm, reply_hdr);

    /* put data */
    FrameMgrPutToken (fm, major_opcode);
    FrameMgrPutToken (fm, minor_opcode);
    FrameMgrPutToken (fm, p_len);

    reply_length = header_size + length;
    reply = (unsigned char *) malloc (reply_length);
    replyp = reply;
    memmove (reply, reply_hdr, header_size);
    replyp += header_size;

    if (length > 0 && data != NULL)
        memmove (replyp, data, length);

    Xi18nXSend (xim, connect_id, reply, reply_length);

    XFree (reply);
    XFree (reply_hdr);
    FrameMgrFree (fm);
}

void _Xi18nSetEventMask (NimfXim *xim,
                         CARD16 connect_id,
                         CARD16 im_id,
                         CARD16 ic_id,
                         CARD32 forward_mask,
                         CARD32 sync_mask)
{
    FrameMgr fm;
    extern XimFrameRec set_event_mask_fr[];
    unsigned char *reply = NULL;
    register int total_size;

    fm = FrameMgrInit (set_event_mask_fr,
                       NULL,
                       _Xi18nNeedSwap (xim, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
        return;
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, im_id); 	/* input-method-id */
    FrameMgrPutToken (fm, ic_id); 	/* input-context-id */
    FrameMgrPutToken (fm, forward_mask);
    FrameMgrPutToken (fm, sync_mask);

    _Xi18nSendMessage (xim,
                       connect_id,
                       XIM_SET_EVENT_MASK,
                       0,
                       reply,
                       total_size);

    FrameMgrFree (fm);
    XFree(reply);
}
