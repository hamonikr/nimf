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

#include <X11/Xlib.h>
#include <stdlib.h>
#include <string.h>
#include "IMdkit.h"
#include <stdarg.h>
#include <glib.h>

static void _IMCountVaList(va_list var, int *total_count)
{
    char *attr;

    *total_count = 0;

    for (attr = va_arg (var, char*);  attr;  attr = va_arg (var, char*))
    {
	(void)va_arg (var, XIMArg *);
	++(*total_count);
    }
    /*endfor*/
}

static void _IMVaToNestedList(va_list var, int max_count, XIMArg **args_return)
{
    XIMArg *args;
    char   *attr;

    if (max_count <= 0)
    {
	*args_return = (XIMArg *) NULL;
	return;
    }
    /*endif*/

    args = (XIMArg *) malloc ((unsigned) (max_count + 1)*sizeof (XIMArg));
    *args_return = args;
    if (!args)
        return;
    /*endif*/

    for (attr = va_arg (var, char*);  attr;  attr = va_arg (var, char *))
    {
	args->name = attr;
	args->value = va_arg (var, XPointer);
	args++;
    }
    /*endfor*/
    args->name = (char*)NULL;
}

XIMS IMOpenIM (Display *display, ...)
{
    va_list var;
    int total_count;
    XIMArg *args;
    XIMS ims;
    Status ret;

    va_start (var, display);
    _IMCountVaList (var, &total_count);
    va_end (var);

    va_start (var, display);
    _IMVaToNestedList (var, total_count, &args);
    va_end (var);

    extern IMMethodsRec Xi18n_im_methods;

    ims = (XIMS) g_malloc0 (sizeof (XIMProtocolRec));
    ims->methods = &Xi18n_im_methods;
    ims->core.display = display;
    ims->protocol = (*ims->methods->setup) (display, args);

    XFree (args);

    if (ims->protocol == (void *) NULL)
    {
	XFree (ims);
	return (XIMS) NULL;
    }

    ret = (ims->methods->openIM) (ims);
    if (ret == False)
    {
	XFree (ims);
	return (XIMS) NULL;
    }

    return (XIMS) ims;
}

Status IMCloseIM (XIMS ims)
{
    (ims->methods->closeIM) (ims);
    XFree (ims);
    return True;
}
