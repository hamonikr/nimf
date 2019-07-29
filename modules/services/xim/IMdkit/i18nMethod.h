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

#ifndef I18N_METHOD_H
#define I18N_METHOD_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#ifndef NEED_EVENTS
#define NEED_EVENTS
#endif
#include <X11/Xproto.h>
#undef NEED_EVENTS

#include "nimf-xim.h"

Bool WaitXSelectionRequest (NimfXim *xim, XEvent *ev);
int  nimf_xim_call_callback (NimfXim *, XPointer);

Status xi18n_openIM (NimfXim *, Window);
Status xi18n_closeIM (NimfXim *);
Status xi18n_forwardEvent (NimfXim *, XPointer);
Status xi18n_commit (NimfXim *, XPointer);
int    xi18n_syncXlib (NimfXim *, XPointer);

#endif /* I18N_METHOD_H */
