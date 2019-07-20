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

#ifndef _IMdkit_h
#define _IMdkit_h

#include <X11/Xmd.h>

typedef struct _NimfXim      NimfXim;

#ifdef __cplusplus
extern "C" {
#endif

/* IM Attributes Name */
#define IMProtocolDepend	"protocolDepend"

/* Masks for IM Attributes Name */
#define I18N_PROTO_DEPEND	0x0400 /* IMProtoDepend */

typedef struct
{
    CARD32	keysym;
    CARD32	modifier;
    CARD32	modifier_mask;
} XIMTriggerKey;

typedef char *XIMEncoding;

typedef struct
{
    unsigned short count_encodings;
    XIMEncoding *supported_encodings;
} XIMEncodings;

typedef struct _XIMS *XIMS;

typedef struct
{
    void*	(*setup) (Display *);
    Status	(*openIM) (XIMS, Window);
    Status	(*closeIM) (XIMS);
    Status	(*forwardEvent) (XIMS, XPointer);
    Status	(*commitString) (XIMS, XPointer);
    int		(*syncXlib) (XIMS, XPointer);
} IMMethodsRec, *IMMethods;

typedef struct _XIMS
{
    IMMethods	methods;
    Bool	sync;
    void	*protocol;
} XIMProtocolRec;

/*
 * X function declarations.
 */
extern XIMS IMOpenIM (NimfXim *);
extern Status IMCloseIM (XIMS);
void IMForwardEvent (XIMS, XPointer);
void IMCommitString (XIMS, XPointer);
int IMSyncXlib (XIMS, XPointer);

#ifdef __cplusplus
}
#endif

#endif /* IMdkit_h */
