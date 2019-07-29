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

#ifndef I18N_X_H
#define I18N_X_H

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "nimf-xim.h"

void ReadXConnectMessage (NimfXim *xim, XClientMessageEvent *ev);
Bool WaitXIMProtocol     (NimfXim*, XEvent*);
Bool Xi18nXWait (NimfXim *xim,
                 CARD16 connect_id,
                 CARD8 major_opcode,
                 CARD8 minor_opcode);
Bool Xi18nXDisconnect (NimfXim *xim, CARD16 connect_id);
Bool Xi18nXSend (NimfXim *xim,
                 CARD16 connect_id,
                 unsigned char *reply,
                 long length);
#endif /* I18N_X_H */
