/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/******************************************************************

         Copyright 1994, 1995 by Sun Microsystems, Inc.
         Copyright 1993, 1994 by Hewlett-Packard Company

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

#include "i18nMethod.h"
#include "FrameMgr.h"
#include "IMdkit.h"
#include "Xi18n.h"
#include "XimFunc.h"
#include <glib.h>

/* How to generate SUPPORTED_LOCALES
#!/usr/bin/ruby
# -*- coding: utf-8 -*-

require 'set'

set = Set.new

File.open("/usr/share/i18n/SUPPORTED", "r").each do |line|
  set << line.split(/_| /)[0]
end

puts set.to_a.join(",")
*/
#define SUPPORTED_LOCALES \
  "aa,af,agr,ak,am,an,anp,ar,ayc,az,as,ast,be,bem,ber,bg,bhb,bho,bi,bn,bo,"   \
  "br,brx,bs,byn,ca,ce,chr,cmn,crh,cs,csb,cv,cy,da,de,doi,dsb,dv,dz,el,en,"   \
  "eo,es,et,eu,fa,ff,fi,fil,fo,fr,fur,fy,ga,gd,gez,gl,gu,gv,ha,hak,he,hi,"    \
  "hif,hne,hr,hsb,ht,hu,hy,ia,id,ig,ik,is,it,iu,ja,ka,kab,kk,kl,km,kn,ko,"    \
  "kok,ks,ku,kw,ky,lb,lg,li,lij,ln,lo,lt,lv,lzh,mag,mai,mfe,mg,mhr,mi,miq,"   \
  "mjw,mk,ml,mn,mni,mr,ms,mt,my,nan,nb,nds,ne,nhn,niu,nl,nn,nr,nso,oc,om,or," \
  "os,pa,pap,pl,ps,pt,quz,raj,ro,ru,rw,sa,sah,sat,sc,sd,se,sgs,shn,shs,si,"   \
  "sid,sk,sl,sm,so,sq,sr,ss,st,sv,sw,szl,ta,tcy,te,tg,th,the,ti,tig,tk,tl,"   \
  "tn,to,tpi,tr,ts,tt,ug,uk,unm,ur,uz,ve,vi,wa,wae,wal,wo,xh,yi,yo,yue,yuw,"  \
  "zh,zu"

extern Xi18nClient *_Xi18nFindClient (Xi18n, CARD16);

static void  *xi18n_setup (Display *);
static Status xi18n_openIM (XIMS, Window);
static Status xi18n_closeIM (XIMS);
static Status xi18n_forwardEvent (XIMS, XPointer);
static Status xi18n_commit (XIMS, XPointer);
static int    xi18n_syncXlib (XIMS, XPointer);

#ifndef XIM_SERVERS
#define XIM_SERVERS "XIM_SERVERS"
#endif
static Atom XIM_Servers = None;

IMMethodsRec Xi18n_im_methods =
{
    xi18n_setup,
    xi18n_openIM,
    xi18n_closeIM,
    xi18n_forwardEvent,
    xi18n_commit,
    xi18n_syncXlib,
};

extern Bool _Xi18nCheckXAddress (Xi18n);

static int SetXi18nSelectionOwner(Xi18n i18n_core, Window im_window)
{
    Display *dpy = i18n_core->address.dpy;
    Window root = RootWindow (dpy, DefaultScreen (dpy));
    Atom realtype;
    int realformat;
    unsigned long bytesafter;
    long *data=NULL;
    unsigned long length;
    Atom atom;
    int i;
    int found;
    int forse = False;

    if ((atom = XInternAtom (dpy, "@server=nimf", False)) == 0)
        return False;

    i18n_core->address.selection = atom;

    if (XIM_Servers == None)
        XIM_Servers = XInternAtom (dpy, XIM_SERVERS, False);
    /*endif*/
    XGetWindowProperty (dpy,
                        root,
                        XIM_Servers,
                        0L,
                        1000000L,
                        False,
                        XA_ATOM,
                        &realtype,
                        &realformat,
                        &length,
                        &bytesafter,
                        (unsigned char **) (&data));
    if (realtype != None && (realtype != XA_ATOM || realformat != 32)) {
        if (data != NULL)
            XFree ((char *) data);
        return False;
    }

    found = False;
    for (i = 0; i < length; i++) {
        if (data[i] == atom) {
            Window owner;
            found = True;
            if ((owner = XGetSelectionOwner (dpy, atom)) != im_window) {
                if (owner == None  ||  forse == True)
                    XSetSelectionOwner (dpy, atom, im_window, CurrentTime);
                else
                    return False;
            }
            break;
        }
    }

    if (found == False) {
        XSetSelectionOwner (dpy, atom, im_window, CurrentTime);
        XChangeProperty (dpy,
                         root,
                         XIM_Servers,
                         XA_ATOM,
                         32,
                         PropModePrepend,
                         (unsigned char *) &atom,
                         1);
    }
    else {
	/*
	 * We always need to generate the PropertyNotify to the Root Window
	 */
        XChangeProperty (dpy,
                         root,
                         XIM_Servers,
                         XA_ATOM,
                         32,
                         PropModePrepend,
                         (unsigned char *) data,
                         0);
    }
    if (data != NULL)
        XFree ((char *) data);

    /* Intern "LOCALES" and "TRANSOPORT" Target Atoms */
    i18n_core->address.Localename = XInternAtom (dpy, LOCALES, False);
    i18n_core->address.Transportname = XInternAtom (dpy, TRANSPORT, False);
    return (XGetSelectionOwner (dpy, atom) == im_window);
}

static int DeleteXi18nAtom(Xi18n i18n_core)
{
    Display *dpy = i18n_core->address.dpy;
    Window root = RootWindow (dpy, DefaultScreen (dpy));
    Atom realtype;
    int realformat;
    unsigned long bytesafter;
    long *data=NULL;
    unsigned long length;
    Atom atom;
    int i, ret;
    int found;

    if ((atom = XInternAtom(dpy, "@server=nimf", False)) == 0)
        return False;

    i18n_core->address.selection = atom;

    if (XIM_Servers == None)
        XIM_Servers = XInternAtom (dpy, XIM_SERVERS, False);
    XGetWindowProperty (dpy,
                        root,
                        XIM_Servers,
                        0L,
                        1000000L,
                        False,
                        XA_ATOM,
                        &realtype,
                        &realformat,
                        &length,
                        &bytesafter,
                        (unsigned char **) (&data));
    if (realtype != XA_ATOM || realformat != 32) {
        if (data != NULL)
            XFree ((char *) data);
        return False;
    }

    found = False;
    for (i = 0; i < length; i++) {
        if (data[i] == atom) {
            found = True;
            break;
        }
    }

    if (found == True) {
        for (i=i+1; i<length; i++)
            data[i-1] = data[i];
        XChangeProperty (dpy,
                         root,
                         XIM_Servers,
                         XA_ATOM,
                         32,
                         PropModeReplace,
                         (unsigned char *)data,
                         length-1);
        ret = True;
    }
    else {
        XChangeProperty (dpy,
                         root,
                         XIM_Servers,
                         XA_ATOM,
                         32,
                         PropModePrepend,
                         (unsigned char *)data,
                         0);
        ret = False;
    }
    if (data != NULL)
        XFree ((char *) data);
    return ret;
}


/* XIM protocol methods */
static void *xi18n_setup (Display *dpy)
{
    Xi18n i18n_core;

    i18n_core = (Xi18n) g_malloc0 (sizeof (Xi18nCore));
    i18n_core->address.dpy = dpy;

    if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
        i18n_core->address.im_byteOrder = 'l';
    else
        i18n_core->address.im_byteOrder = 'B';
    /*endif*/

    /* install IMAttr and ICAttr list in i18n_core */
    _Xi18nInitAttrList (i18n_core);

    /* install IMExtension list in i18n_core */
    _Xi18nInitExtension (i18n_core);

    return i18n_core;
}

static void
ReturnSelectionNotify (NimfXim *xim, XSelectionRequestEvent *ev)
{
  Xi18n i18n_core = xim->xims->protocol;
  XEvent event;
  const char *data = NULL;

  event.type = SelectionNotify;
  event.xselection.requestor = ev->requestor;
  event.xselection.selection = ev->selection;
  event.xselection.target = ev->target;
  event.xselection.time = ev->time;
  event.xselection.property = ev->property;

  if (ev->target == i18n_core->address.Localename)
    data = "@locale=C" SUPPORTED_LOCALES;
  else if (ev->target == i18n_core->address.Transportname)
    data = "@transport=X/";

  XChangeProperty (xim->display,
                   event.xselection.requestor,
                   ev->target,
                   ev->target,
                   8,
                   PropModeReplace,
                   (unsigned char *) data,
                   strlen (data));
  XSendEvent (xim->display, event.xselection.requestor, False, NoEventMask, &event);
  XFlush (xim->display);
}

Bool
WaitXSelectionRequest (NimfXim *xim,
                       XEvent  *ev)
{
    Xi18n i18n_core = xim->xims->protocol;

    if (((XSelectionRequestEvent *) ev)->selection
        == i18n_core->address.selection)
    {
        ReturnSelectionNotify (xim, (XSelectionRequestEvent *) ev);
        return True;
    }
    /*endif*/
    return False;
}

static Status
xi18n_openIM (XIMS ims, Window im_window)
{
    Xi18n i18n_core = ims->protocol;
    Display *dpy = i18n_core->address.dpy;

    if (!_Xi18nCheckXAddress (i18n_core)
        ||
        !SetXi18nSelectionOwner (i18n_core, im_window)
        ||
        !i18n_core->methods.begin (ims))
    {
        XFree (i18n_core);
        return False;
    }
    /*endif*/

    XFlush(dpy);
    return True;
}

static Status xi18n_closeIM(XIMS ims)
{
    Xi18n i18n_core = ims->protocol;

    DeleteXi18nAtom(i18n_core);
    if (!i18n_core->methods.end (ims))
        return False;

    XFree (i18n_core);
    return True;
}

static void EventToWireEvent (XEvent *ev, xEvent *event,
			      CARD16 *serial, Bool byte_swap)
{
    FrameMgr fm;
    extern XimFrameRec wire_keyevent_fr[];
    extern XimFrameRec short_fr[];
    BYTE b;
    CARD16 c16;
    CARD32 c32;

    *serial = (CARD16)(ev->xany.serial >> 16);
    switch (ev->type) {
      case KeyPress:
      case KeyRelease:
	{
	    XKeyEvent *kev = (XKeyEvent*)ev;
	    /* create FrameMgr */
	    fm = FrameMgrInit(wire_keyevent_fr, (char *)(&(event->u)), byte_swap);

	    /* set values */
	    b = (BYTE)kev->type;          FrameMgrPutToken(fm, b);
	    b = (BYTE)kev->keycode;       FrameMgrPutToken(fm, b);
	    c16 = (CARD16)(kev->serial & (unsigned long)0xffff);
					  FrameMgrPutToken(fm, c16);
	    c32 = (CARD32)kev->time;      FrameMgrPutToken(fm, c32);
	    c32 = (CARD32)kev->root;      FrameMgrPutToken(fm, c32);
	    c32 = (CARD32)kev->window;    FrameMgrPutToken(fm, c32);
	    c32 = (CARD32)kev->subwindow; FrameMgrPutToken(fm, c32);
	    c16 = (CARD16)kev->x_root;    FrameMgrPutToken(fm, c16);
	    c16 = (CARD16)kev->y_root;    FrameMgrPutToken(fm, c16);
	    c16 = (CARD16)kev->x;         FrameMgrPutToken(fm, c16);
	    c16 = (CARD16)kev->y;         FrameMgrPutToken(fm, c16);
	    c16 = (CARD16)kev->state;     FrameMgrPutToken(fm, c16);
	    b = (BYTE)kev->same_screen;   FrameMgrPutToken(fm, b);
	}
	break;
      default:
	  /* create FrameMgr */
	  fm = FrameMgrInit(short_fr, (char *)(&(event->u.u.sequenceNumber)),
			    byte_swap);
	  c16 = (CARD16)(ev->xany.serial & (unsigned long)0xffff);
	  FrameMgrPutToken(fm, c16);
	  break;
    }
    /* free FrameMgr */
    FrameMgrFree(fm);
}

static Status xi18n_forwardEvent (XIMS ims, XPointer xp)
{
    Xi18n i18n_core = ims->protocol;
    IMForwardEventStruct *call_data = (IMForwardEventStruct *)xp;
    FrameMgr fm;
    extern XimFrameRec forward_event_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    unsigned char *replyp;
    CARD16 serial;
    int event_size;
    Xi18nClient *client;

    client = (Xi18nClient *) _Xi18nFindClient (i18n_core, call_data->connect_id);

    /* create FrameMgr */
    fm = FrameMgrInit (forward_event_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, call_data->connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    event_size = sizeof (xEvent);
    reply = (unsigned char *) malloc (total_size + event_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims,
                           call_data->connect_id,
                           XIM_ERROR,
                           0,
                           0,
                           0);
        return False;
    }
    /*endif*/
    memset (reply, 0, total_size + event_size);
    FrameMgrSetBuffer (fm, reply);
    replyp = reply;

    call_data->sync_bit = 1; 	/* always sync */
    client->sync = True;

    FrameMgrPutToken (fm, call_data->connect_id);
    FrameMgrPutToken (fm, call_data->icid);
    FrameMgrPutToken (fm, call_data->sync_bit);

    replyp += total_size;
    EventToWireEvent (&(call_data->event),
                      (xEvent *) replyp,
                      &serial,
                      _Xi18nNeedSwap (i18n_core, call_data->connect_id));

    FrameMgrPutToken (fm, serial);

    _Xi18nSendMessage (ims,
                       call_data->connect_id,
                       XIM_FORWARD_EVENT,
                       0,
                       reply,
                       total_size + event_size);

    XFree (reply);
    FrameMgrFree (fm);

    return True;
}

static Status xi18n_commit (XIMS ims, XPointer xp)
{
    Xi18n i18n_core = ims->protocol;
    IMCommitStruct *call_data = (IMCommitStruct *)xp;
    FrameMgr fm;
    extern XimFrameRec commit_chars_fr[];
    extern XimFrameRec commit_both_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    CARD16 str_length;

    call_data->flag |= XimSYNCHRONUS;  /* always sync */

    if (!(call_data->flag & XimLookupKeySym)
        &&
        (call_data->flag & XimLookupChars))
    {
        fm = FrameMgrInit (commit_chars_fr,
                           NULL,
                           _Xi18nNeedSwap (i18n_core, call_data->connect_id));

        /* set length of STRING8 */
        str_length = strlen (call_data->commit_string);
        FrameMgrSetSize (fm, str_length);
        total_size = FrameMgrGetTotalSize (fm);
        reply = (unsigned char *) malloc (total_size);
        if (!reply)
        {
            _Xi18nSendMessage (ims,
                               call_data->connect_id,
                               XIM_ERROR,
                               0,
                               0,
                               0);
            return False;
        }
        /*endif*/
        memset (reply, 0, total_size);
        FrameMgrSetBuffer (fm, reply);

        str_length = FrameMgrGetSize (fm);
        FrameMgrPutToken (fm, call_data->connect_id);
        FrameMgrPutToken (fm, call_data->icid);
        FrameMgrPutToken (fm, call_data->flag);
        FrameMgrPutToken (fm, str_length);
        FrameMgrPutToken (fm, call_data->commit_string);
    }
    else
    {
        fm = FrameMgrInit (commit_both_fr,
                           NULL,
                           _Xi18nNeedSwap (i18n_core, call_data->connect_id));
        /* set length of STRING8 */
        str_length = strlen (call_data->commit_string);
        if (str_length > 0)
            FrameMgrSetSize (fm, str_length);
        /*endif*/
        total_size = FrameMgrGetTotalSize (fm);
        reply = (unsigned char *) malloc (total_size);
        if (!reply)
        {
            _Xi18nSendMessage (ims,
                               call_data->connect_id,
                               XIM_ERROR,
                               0,
                               0,
                               0);
            return False;
        }
        /*endif*/
        FrameMgrSetBuffer (fm, reply);
        FrameMgrPutToken (fm, call_data->connect_id);
        FrameMgrPutToken (fm, call_data->icid);
        FrameMgrPutToken (fm, call_data->flag);
        FrameMgrPutToken (fm, call_data->keysym);
        if (str_length > 0)
        {
            str_length = FrameMgrGetSize (fm);
            FrameMgrPutToken (fm, str_length);
            FrameMgrPutToken (fm, call_data->commit_string);
        }
        /*endif*/
    }
    /*endif*/
    _Xi18nSendMessage (ims,
                       call_data->connect_id,
                       XIM_COMMIT,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    return True;
}

int nimf_xim_call_callback (XIMS ims, XPointer xp)
{
    IMProtocol *call_data = (IMProtocol *)xp;
    switch (call_data->major_code)
    {
    case XIM_GEOMETRY:
        return _Xi18nGeometryCallback (ims, call_data);

    case XIM_PREEDIT_START:
        return _Xi18nPreeditStartCallback (ims, call_data);

    case XIM_PREEDIT_DRAW:
        return _Xi18nPreeditDrawCallback (ims, call_data);

    case XIM_PREEDIT_CARET:
        return _Xi18nPreeditCaretCallback (ims, call_data);

    case XIM_PREEDIT_DONE:
        return _Xi18nPreeditDoneCallback (ims, call_data);

    case XIM_STATUS_START:
        return _Xi18nStatusStartCallback (ims, call_data);

    case XIM_STATUS_DRAW:
        return _Xi18nStatusDrawCallback (ims, call_data);

    case XIM_STATUS_DONE:
        return _Xi18nStatusDoneCallback (ims, call_data);

    case XIM_STR_CONVERSION:
        return _Xi18nStringConversionCallback (ims, call_data);
    }
    /*endswitch*/
    return False;
}

static int xi18n_syncXlib (XIMS ims, XPointer xp)
{
    IMProtocol *call_data = (IMProtocol *)xp;
    Xi18n i18n_core = ims->protocol;
    IMSyncXlibStruct *sync_xlib;

    extern XimFrameRec sync_fr[];
    FrameMgr fm;
    CARD16 connect_id = call_data->any.connect_id;
    int total_size;
    unsigned char *reply;

    sync_xlib = (IMSyncXlibStruct *) &call_data->sync_xlib;
    fm = FrameMgrInit (sync_fr, NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));
    total_size = FrameMgrGetTotalSize(fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply) {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return False;
    }
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    /* input input-method ID */
    FrameMgrPutToken (fm, connect_id);
    /* input input-context ID */
    FrameMgrPutToken (fm, sync_xlib->icid);
    _Xi18nSendMessage (ims, connect_id, XIM_SYNC, 0, reply, total_size);

    FrameMgrFree (fm);
    XFree(reply);
    return True;
}
