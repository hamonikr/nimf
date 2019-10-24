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

#include <stdlib.h>
#include <sys/param.h>
#include <X11/Xlib.h>
#ifndef NEED_EVENTS
#define NEED_EVENTS
#endif
#include <X11/Xproto.h>
#undef NEED_EVENTS
#include "FrameMgr.h"
#include "Xi18n.h"
#include "XimFunc.h"
#include "nimf-xim.h"
#include "i18nX.h"

extern Xi18nClient *_Xi18nFindClient (NimfXim *, CARD16);
extern int
on_incoming_message (NimfXim    *xim,
                     IMProtocol *data);

static void GetProtocolVersion (CARD16 client_major,
                                CARD16 client_minor,
                                CARD16 *server_major,
                                CARD16 *server_minor)
{
    *server_major = client_major;
    *server_minor = client_minor;
}

static void ConnectMessageProc (NimfXim *xim,
                                IMProtocol *call_data,
                                unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec connect_fr[], connect_reply_fr[];
    register int total_size;
    CARD16 server_major_version, server_minor_version;
    unsigned char *reply = NULL;
    IMConnectStruct *imconnect =
        (IMConnectStruct*) &call_data->imconnect;
    CARD16 connect_id = call_data->any.connect_id;

    fm = FrameMgrInit (connect_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));

    /* get data */
    FrameMgrGetToken (fm, imconnect->byte_order);
    FrameMgrGetToken (fm, imconnect->major_version);
    FrameMgrGetToken (fm, imconnect->minor_version);

    FrameMgrFree (fm);

    GetProtocolVersion (imconnect->major_version,
                        imconnect->minor_version,
                        &server_major_version,
                        &server_minor_version);

    fm = FrameMgrInit (connect_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (xim, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (xim, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, server_major_version);
    FrameMgrPutToken (fm, server_minor_version);

    _Xi18nSendMessage (xim,
                       connect_id,
                       XIM_CONNECT_REPLY,
                       0,
                       reply,
                       total_size);

    FrameMgrFree (fm);
    XFree (reply);
}

static void DisConnectMessageProc (NimfXim *xim, IMProtocol *call_data)
{
    unsigned char *reply = NULL;
    CARD16 connect_id = call_data->any.connect_id;

    _Xi18nSendMessage (xim,
                       connect_id,
                       XIM_DISCONNECT_REPLY,
                       0,
                       reply,
                       0);

    Xi18nXDisconnect (xim, connect_id);
}

static void
OpenMessageProc (NimfXim *xim, IMProtocol *call_data, unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec open_fr[];
    extern XimFrameRec open_reply_fr[];
    unsigned char *reply = NULL;
    int str_size;
    register int i, total_size;
    CARD16 connect_id = call_data->any.connect_id;
    int str_length;
    char *name;
    IMOpenStruct *imopen = (IMOpenStruct *) &call_data->imopen;

    fm = FrameMgrInit (open_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));

    /* get data */
    FrameMgrGetToken (fm, str_length);
    FrameMgrSetSize (fm, str_length);
    FrameMgrGetToken (fm, name);
    imopen->lang.length = str_length;
    imopen->lang.name = malloc (str_length + 1);
    strncpy (imopen->lang.name, name, str_length);
    imopen->lang.name[str_length] = (char) 0;

    FrameMgrFree (fm);

    on_incoming_message (xim, call_data);

    XFree (imopen->lang.name);

    fm = FrameMgrInit (open_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (xim, connect_id));

    /* set iteration count for list of imattr */
    FrameMgrSetIterCount (fm, xim->address.im_attr_num);

    /* set length of BARRAY item in ximattr_fr */
    for (i = 0;  i < xim->address.im_attr_num;  i++)
    {
        str_size = strlen (xim->address.xim_attr[i].name);
        FrameMgrSetSize (fm, str_size);
    }
    /*endfor*/
    /* set iteration count for list of icattr */
    FrameMgrSetIterCount (fm, xim->address.ic_attr_num);
    /* set length of BARRAY item in xicattr_fr */
    for (i = 0;  i < xim->address.ic_attr_num;  i++)
    {
        str_size = strlen (xim->address.xic_attr[i].name);
        FrameMgrSetSize (fm, str_size);
    }
    /*endfor*/

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (xim, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    /* input input-method ID */
    FrameMgrPutToken (fm, connect_id);

    for (i = 0;  i < xim->address.im_attr_num;  i++)
    {
        str_size = FrameMgrGetSize (fm);
        FrameMgrPutToken (fm, xim->address.xim_attr[i].attribute_id);
        FrameMgrPutToken (fm, xim->address.xim_attr[i].type);
        FrameMgrPutToken (fm, str_size);
        FrameMgrPutToken (fm, xim->address.xim_attr[i].name);
    }
    /*endfor*/
    for (i = 0;  i < xim->address.ic_attr_num;  i++)
    {
        str_size = FrameMgrGetSize (fm);
        FrameMgrPutToken (fm, xim->address.xic_attr[i].attribute_id);
        FrameMgrPutToken (fm, xim->address.xic_attr[i].type);
        FrameMgrPutToken (fm, str_size);
        FrameMgrPutToken (fm, xim->address.xic_attr[i].name);
    }
    /*endfor*/

    _Xi18nSendMessage (xim,
                       connect_id,
                       XIM_OPEN_REPLY,
                       0,
                       reply,
                       total_size);

    FrameMgrFree (fm);
    XFree (reply);
}

static void CloseMessageProc (NimfXim *xim,
                              IMProtocol *call_data,
                              unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec close_fr[];
    extern XimFrameRec close_reply_fr[];
    unsigned char *reply = NULL;
    register int total_size;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (close_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));

    FrameMgrGetToken (fm, input_method_ID);

    FrameMgrFree (fm);

    on_incoming_message (xim, call_data);

    fm = FrameMgrInit (close_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (xim, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (xim,
                           connect_id,
                           XIM_ERROR,
                           0,
                           0,
                           0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);

    _Xi18nSendMessage (xim,
                       connect_id,
                       XIM_CLOSE_REPLY,
                       0,
                       reply,
                       total_size);

    FrameMgrFree (fm);
    XFree (reply);
}

static XIMExt *MakeExtensionList (NimfXim *xim,
                                  XIMStr *lib_extension,
                                  int number,
                                  int *reply_number)
{
    XIMExt *ext_list;
    XIMExt *im_ext = (XIMExt *) xim->address.extension;
    int im_ext_len = xim->address.ext_num;
    int i;
    int j;

    *reply_number = 0;

    if (number == 0)
    {
        /* query all extensions */
        *reply_number = im_ext_len;
    }
    else
    {
        for (i = 0;  i < im_ext_len;  i++)
        {
            for (j = 0;  j < (int) number;  j++)
            {
                if (strcmp (lib_extension[j].name, im_ext[i].name) == 0)
                {
                    (*reply_number)++;
                    break;
                }
                /*endif*/
            }
            /*endfor*/
        }
        /*endfor*/
    }
    /*endif*/

    if (!(*reply_number))
        return NULL;
    /*endif*/
    ext_list = (XIMExt *) malloc (sizeof (XIMExt)*(*reply_number));
    if (!ext_list)
        return NULL;
    /*endif*/
    memset (ext_list, 0, sizeof (XIMExt)*(*reply_number));

    if (number == 0)
    {
        /* query all extensions */
        for (i = 0;  i < im_ext_len;  i++)
        {
            ext_list[i].major_opcode = im_ext[i].major_opcode;
            ext_list[i].minor_opcode = im_ext[i].minor_opcode;
            ext_list[i].length = im_ext[i].length;
            ext_list[i].name = malloc (im_ext[i].length + 1);
            strcpy (ext_list[i].name, im_ext[i].name);
        }
        /*endfor*/
    }
    else
    {
        int n = 0;

        for (i = 0;  i < im_ext_len;  i++)
        {
            for (j = 0;  j < (int)number;  j++)
            {
                if (strcmp (lib_extension[j].name, im_ext[i].name) == 0)
                {
                    ext_list[n].major_opcode = im_ext[i].major_opcode;
                    ext_list[n].minor_opcode = im_ext[i].minor_opcode;
                    ext_list[n].length = im_ext[i].length;
                    ext_list[n].name = malloc (im_ext[i].length + 1);
                    strcpy (ext_list[n].name, im_ext[i].name);
                    n++;
                    break;
                }
                /*endif*/
            }
            /*endfor*/
        }
        /*endfor*/
    }
    /*endif*/
    return ext_list;
}

static void QueryExtensionMessageProc (NimfXim *xim,
                                       IMProtocol *call_data,
                                       unsigned char *p)
{
    FrameMgr fm;
    FmStatus status;
    extern XimFrameRec query_extension_fr[];
    extern XimFrameRec query_extension_reply_fr[];
    unsigned char *reply = NULL;
    int str_size;
    register int i;
    register int number;
    register int total_size;
    int byte_length;
    int reply_number = 0;
    XIMExt *ext_list;
    IMQueryExtensionStruct *query_ext =
        (IMQueryExtensionStruct *) &call_data->queryext;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (query_extension_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));

    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, byte_length);
    query_ext->extension = (XIMStr *) malloc (sizeof (XIMStr)*10);
    memset (query_ext->extension, 0, sizeof (XIMStr)*10);
    number = 0;
    while (FrameMgrIsIterLoopEnd (fm, &status) == False)
    {
        char *name;
        int str_length;

        FrameMgrGetToken (fm, str_length);
        FrameMgrSetSize (fm, str_length);
        query_ext->extension[number].length = str_length;
        FrameMgrGetToken (fm, name);
        query_ext->extension[number].name = malloc (str_length + 1);
        strncpy (query_ext->extension[number].name, name, str_length);
        query_ext->extension[number].name[str_length] = (char) 0;
        number++;
    }
    /*endwhile*/
    query_ext->number = number;

    FrameMgrFree (fm);

    ext_list = MakeExtensionList (xim,
                                  query_ext->extension,
                                  number,
                                  &reply_number);

    for (i = 0;  i < number;  i++)
        XFree (query_ext->extension[i].name);
    /*endfor*/
    XFree (query_ext->extension);

    fm = FrameMgrInit (query_extension_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (xim, connect_id));

    /* set iteration count for list of extensions */
    FrameMgrSetIterCount (fm, reply_number);

    /* set length of BARRAY item in ext_fr */
    for (i = 0;  i < reply_number;  i++)
    {
        str_size = strlen (ext_list[i].name);
        FrameMgrSetSize (fm, str_size);
    }
    /*endfor*/

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (xim,
                           connect_id,
                           XIM_ERROR,
                           0,
                           0,
                           0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);

    for (i = 0;  i < reply_number;  i++)
    {
        str_size = FrameMgrGetSize (fm);
        FrameMgrPutToken (fm, ext_list[i].major_opcode);
        FrameMgrPutToken (fm, ext_list[i].minor_opcode);
        FrameMgrPutToken (fm, str_size);
        FrameMgrPutToken (fm, ext_list[i].name);
    }
    /*endfor*/
    _Xi18nSendMessage (xim,
                       connect_id,
                       XIM_QUERY_EXTENSION_REPLY,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    for (i = 0;  i < reply_number;  i++)
        XFree (ext_list[i].name);
    /*endfor*/
    XFree ((char *) ext_list);
}

static void SyncReplyMessageProc (NimfXim *xim,
                                  IMProtocol *call_data,
                                  unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec sync_reply_fr[];
    CARD16 connect_id = call_data->any.connect_id;
    Xi18nClient *client;
    CARD16 input_method_ID;
    CARD16 input_context_ID;

    client = (Xi18nClient *)_Xi18nFindClient (xim, connect_id);
    fm = FrameMgrInit (sync_reply_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, input_context_ID);
    FrameMgrFree (fm);

    client->sync = False;

    if (xim->sync == True) {
        xim->sync = False;
        call_data->sync_xlib.major_code = XIM_SYNC_REPLY;
        call_data->sync_xlib.minor_code = 0;
        call_data->sync_xlib.connect_id = input_method_ID;
        call_data->sync_xlib.icid = input_context_ID;
            on_incoming_message (xim, call_data);
    }
}

static void GetIMValueFromName (NimfXim *xim,
                                CARD16 connect_id,
                                char *buf,
                                char *name,
                                int *length)
{
    register int i;

    if (strcmp (name, XNQueryInputStyle) == 0)
    {
        XIMStyles *styles = &xim->im_styles;

        *length = sizeof (CARD16)*2; 	/* count_styles, unused */
        *length += styles->count_styles*sizeof (CARD32);

        if (buf != NULL)
        {
            FrameMgr fm;
            extern XimFrameRec input_styles_fr[];
            unsigned char *data = NULL;
            int total_size;

            fm = FrameMgrInit (input_styles_fr,
                               NULL,
                               _Xi18nNeedSwap (xim, connect_id));

            /* set iteration count for list of input_style */
            FrameMgrSetIterCount (fm, styles->count_styles);

            total_size = FrameMgrGetTotalSize (fm);
            data = (unsigned char *) malloc (total_size);
            if (!data)
                return;
            /*endif*/
            memset (data, 0, total_size);
            FrameMgrSetBuffer (fm, data);

            FrameMgrPutToken (fm, styles->count_styles);
            for (i = 0;  i < (int) styles->count_styles;  i++)
                FrameMgrPutToken (fm, styles->supported_styles[i]);
            /*endfor*/
            memmove (buf, data, total_size);
            FrameMgrFree (fm);

            /* ADDED BY SUZHE */
            free (data);
            /* ADDED BY SUZHE */
        }
        /*endif*/
    }
    /*endif*/

    else if (strcmp (name, XNQueryIMValuesList) == 0) {
    }
}

static XIMAttribute *MakeIMAttributeList (NimfXim *xim,
                                          CARD16 connect_id,
                                          CARD16 *list,
                                          int *number,
                                          int *length)
{
    XIMAttribute *attrib_list;
    int list_num;
    XIMAttr *attr = xim->address.xim_attr;
    int list_len = xim->address.im_attr_num;
    register int i;
    register int j;
    int value_length = 0;
    int number_ret = 0;

    *length = 0;
    list_num = 0;
    for (i = 0;  i < *number;  i++)
    {
        for (j = 0;  j < list_len;  j++)
        {
            if (attr[j].attribute_id == list[i])
            {
                list_num++;
                break;
            }
            /*endif*/
        }
        /*endfor*/
    }
    /*endfor*/
    attrib_list = (XIMAttribute *) malloc (sizeof (XIMAttribute)*(list_num + 1));
    if (!attrib_list)
        return NULL;
    /*endif*/
    memset (attrib_list, 0, sizeof (XIMAttribute)*(list_num + 1));
    number_ret = list_num;
    list_num = 0;
    for (i = 0;  i < *number;  i++)
    {
        for (j = 0;  j < list_len;  j++)
        {
            if (attr[j].attribute_id == list[i])
            {
                attrib_list[list_num].attribute_id = attr[j].attribute_id;
                attrib_list[list_num].name_length = attr[j].length;
                attrib_list[list_num].name = attr[j].name;
                attrib_list[list_num].type = attr[j].type;
                GetIMValueFromName (xim,
                                    connect_id,
                                    NULL,
                                    attr[j].name,
                                    &value_length);
                attrib_list[list_num].value_length = value_length;
                attrib_list[list_num].value = (void *) malloc (value_length);
                memset(attrib_list[list_num].value, 0, value_length);
                GetIMValueFromName (xim,
                                    connect_id,
                                    attrib_list[list_num].value,
                                    attr[j].name,
                                    &value_length);
                *length += sizeof (CARD16)*2;
                *length += value_length;
                *length += IMPAD (value_length);
                list_num++;
                break;
            }
            /*endif*/
        }
        /*endfor*/
    }
    /*endfor*/
    *number = number_ret;
    return attrib_list;
}

static void GetIMValuesMessageProc (NimfXim *xim,
                                    IMProtocol *call_data,
                                    unsigned char *p)
{
    FrameMgr fm;
    FmStatus status;
    extern XimFrameRec get_im_values_fr[];
    extern XimFrameRec get_im_values_reply_fr[];
    CARD16 byte_length;
    int list_len, total_size;
    unsigned char *reply = NULL;
    int iter_count;
    register int i;
    register int j;
    int number;
    CARD16 *im_attrID_list;
    char **name_list;
    CARD16 name_number;
    XIMAttribute *im_attribute_list;
    IMGetIMValuesStruct *getim = (IMGetIMValuesStruct *)&call_data->getim;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    /* create FrameMgr */
    fm = FrameMgrInit (get_im_values_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));

    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, byte_length);
    im_attrID_list = (CARD16 *) malloc (sizeof (CARD16)*20);
    memset (im_attrID_list, 0, sizeof (CARD16)*20);
    name_list = (char **)malloc(sizeof(char *) * 20);
    memset(name_list, 0, sizeof(char *) * 20);
    number = 0;
    while (FrameMgrIsIterLoopEnd (fm, &status) == False)
    {
        FrameMgrGetToken (fm, im_attrID_list[number]);
        number++;
    }
    FrameMgrFree (fm);

    name_number = 0;
    for (i = 0;  i < number;  i++) {
        for (j = 0;  j < xim->address.im_attr_num;  j++) {
            if (xim->address.xim_attr[j].attribute_id ==
                    im_attrID_list[i]) {
                name_list[name_number++] =
			xim->address.xim_attr[j].name;
                break;
            }
        }
    }
    getim->number = name_number;
    getim->im_attr_list = name_list;
    XFree (name_list);

    im_attribute_list = MakeIMAttributeList (xim,
                                             connect_id,
                                             im_attrID_list,
                                             &number,
                                             &list_len);
    if (im_attrID_list)
        XFree (im_attrID_list);
    /*endif*/

    fm = FrameMgrInit (get_im_values_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (xim, connect_id));

    iter_count = number;

    /* set iteration count for list of im_attribute */
    FrameMgrSetIterCount (fm, iter_count);

    /* set length of BARRAY item in ximattribute_fr */
    for (i = 0;  i < iter_count;  i++)
        FrameMgrSetSize (fm, im_attribute_list[i].value_length);
    /*endfor*/

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (xim, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);

    for (i = 0;  i < iter_count;  i++)
    {
        FrameMgrPutToken (fm, im_attribute_list[i].attribute_id);
        FrameMgrPutToken (fm, im_attribute_list[i].value_length);
        FrameMgrPutToken (fm, im_attribute_list[i].value);
    }
    /*endfor*/
    _Xi18nSendMessage (xim,
                       connect_id,
                       XIM_GET_IM_VALUES_REPLY,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    for (i = 0; i < iter_count; i++)
        XFree(im_attribute_list[i].value);
    XFree (im_attribute_list);
}

static void CreateICMessageProc (NimfXim *xim,
                                 IMProtocol *call_data,
                                 unsigned char *p)
{
    _Xi18nChangeIC (xim, call_data, p, True);
}

static void SetICValuesMessageProc (NimfXim *xim,
                                    IMProtocol *call_data,
                                    unsigned char *p)
{
    _Xi18nChangeIC (xim, call_data, p, False);
}

static void GetICValuesMessageProc (NimfXim *xim,
                                    IMProtocol *call_data,
                                    unsigned char *p)
{
    _Xi18nGetIC (xim, call_data, p);
}

static void SetICFocusMessageProc (NimfXim *xim,
                                   IMProtocol *call_data,
                                   unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec set_ic_focus_fr[];
    IMChangeFocusStruct *setfocus;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    setfocus = (IMChangeFocusStruct *) &call_data->changefocus;

    fm = FrameMgrInit (set_ic_focus_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));

    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, setfocus->icid);

    FrameMgrFree (fm);

    on_incoming_message (xim, call_data);
}

static void UnsetICFocusMessageProc (NimfXim *xim,
                                     IMProtocol *call_data,
                                     unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec unset_ic_focus_fr[];
    IMChangeFocusStruct *unsetfocus;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    unsetfocus = (IMChangeFocusStruct *) &call_data->changefocus;

    fm = FrameMgrInit (unset_ic_focus_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));

    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, unsetfocus->icid);

    FrameMgrFree (fm);

    on_incoming_message (xim, call_data);
}

static void DestroyICMessageProc (NimfXim *xim,
                                  IMProtocol *call_data,
                                  unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec destroy_ic_fr[];
    extern XimFrameRec destroy_ic_reply_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMDestroyICStruct *destroy =
        (IMDestroyICStruct *) &call_data->destroyic;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (destroy_ic_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));

    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, destroy->icid);

    FrameMgrFree (fm);

    on_incoming_message (xim, call_data);

    fm = FrameMgrInit (destroy_ic_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (xim, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (xim, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);
    FrameMgrPutToken (fm, destroy->icid);

    _Xi18nSendMessage (xim,
                       connect_id,
                       XIM_DESTROY_IC_REPLY,
                       0,
                       reply,
                       total_size);
    XFree(reply);
    FrameMgrFree (fm);
}

static void ResetICMessageProc (NimfXim *xim,
                                IMProtocol *call_data,
                                unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec reset_ic_fr[];
    extern XimFrameRec reset_ic_reply_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMResetICStruct *resetic =
        (IMResetICStruct *) &call_data->resetic;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (reset_ic_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));

    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, resetic->icid);

    FrameMgrFree (fm);

    on_incoming_message (xim, call_data);

    /* create FrameMgr */
    fm = FrameMgrInit (reset_ic_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (xim, connect_id));

    /* set length of STRING8 */
    FrameMgrSetSize (fm, resetic->length);

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (xim, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);
    FrameMgrPutToken (fm, resetic->icid);
    FrameMgrPutToken(fm, resetic->length);
    FrameMgrPutToken (fm, resetic->commit_string);

    _Xi18nSendMessage (xim,
                       connect_id,
                       XIM_RESET_IC_REPLY,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree(reply);
}

static int WireEventToEvent (NimfXim *xim,
                             xEvent *event,
                             CARD16 serial,
                             XEvent *ev,
                             Bool byte_swap)
{
    FrameMgr fm;
    extern XimFrameRec wire_keyevent_fr[];
    BYTE b;
    CARD16 c16;
    CARD32 c32;
    int ret = False;

    /* create FrameMgr */
    fm = FrameMgrInit(wire_keyevent_fr, (char *)(&(event->u)), byte_swap);


    /* get & set type */
    FrameMgrGetToken(fm, b);
    ev->type = (unsigned int)b;
    /* get detail */
    FrameMgrGetToken(fm, b);
    /* get & set serial */
    FrameMgrGetToken(fm, c16);
    ev->xany.serial = (unsigned long)c16;
    ev->xany.serial |= ((unsigned long) serial) << 16;
    ev->xany.send_event = False;
    ev->xany.display = xim->display;

    /* Remove SendEvent flag from event type to emulate KeyPress/Release */
    ev->type &= 0x7F;

    switch (ev->type) {
      case KeyPress:
      case KeyRelease:
      {
          XKeyEvent *kev = (XKeyEvent*)ev;

          /* set keycode (detail) */
          kev->keycode = (unsigned int)b;

          /* get & set values */
          FrameMgrGetToken(fm, c32); kev->time = (Time)c32;
          FrameMgrGetToken(fm, c32); kev->root = (Window)c32;
          FrameMgrGetToken(fm, c32); kev->window = (Window)c32;
          FrameMgrGetToken(fm, c32); kev->subwindow = (Window)c32;
          FrameMgrGetToken(fm, c16); kev->x_root = (int)c16;
          FrameMgrGetToken(fm, c16); kev->y_root = (int)c16;
          FrameMgrGetToken(fm, c16); kev->x = (int)c16;
          FrameMgrGetToken(fm, c16); kev->y = (int)c16;
          FrameMgrGetToken(fm, c16); kev->state = (unsigned int)c16;
          FrameMgrGetToken(fm, b);   kev->same_screen = (Bool)b;
      }
      ret = True;
      break;
      default:
      break;
    }
    /* free FrameMgr */
    FrameMgrFree(fm);
    return ret;
}

static void ForwardEventMessageProc (NimfXim *xim,
                                     IMProtocol *call_data,
                                     unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec forward_event_fr[];
    xEvent wire_event;
    IMForwardEventStruct *forward =
        (IMForwardEventStruct*) &call_data->forwardevent;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (forward_event_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, forward->icid);
    FrameMgrGetToken (fm, forward->sync_bit);
    FrameMgrGetToken (fm, forward->serial_number);
    p += sizeof (CARD16)*4;
    memmove (&wire_event, p, sizeof (xEvent));

    FrameMgrFree (fm);

    if (WireEventToEvent (xim,
                          &wire_event,
                          forward->serial_number,
                          &forward->event,
			  _Xi18nNeedSwap (xim, connect_id)) == True)
    {
        on_incoming_message (xim, call_data);
    }
}

static void ExtForwardKeyEventMessageProc (NimfXim *xim,
                                           IMProtocol *call_data,
                                           unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec ext_forward_keyevent_fr[];
    CARD8 type, keycode;
    CARD16 state;
    CARD32 ev_time, window;
    IMForwardEventStruct *forward =
        (IMForwardEventStruct *) &call_data->forwardevent;
    XEvent *ev = (XEvent *) &forward->event;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (ext_forward_keyevent_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, forward->icid);
    FrameMgrGetToken (fm, forward->sync_bit);
    FrameMgrGetToken (fm, forward->serial_number);
    FrameMgrGetToken (fm, type);
    FrameMgrGetToken (fm, keycode);
    FrameMgrGetToken (fm, state);
    FrameMgrGetToken (fm, ev_time);
    FrameMgrGetToken (fm, window);

    FrameMgrFree (fm);

    if (type != KeyPress)
    {
        _Xi18nSendMessage (xim, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/

    /* make a faked keypress event */
    ev->type = (int)type;
    ev->xany.send_event = True;
    ev->xany.display = xim->display;
    ev->xany.serial = (unsigned long) forward->serial_number;
    ((XKeyEvent *) ev)->keycode = (unsigned int) keycode;
    ((XKeyEvent *) ev)->state = (unsigned int) state;
    ((XKeyEvent *) ev)->time = (Time) ev_time;
    ((XKeyEvent *) ev)->window = (Window) window;
    ((XKeyEvent *) ev)->root = DefaultRootWindow (ev->xany.display);
    ((XKeyEvent *) ev)->x = 0;
    ((XKeyEvent *) ev)->y = 0;
    ((XKeyEvent *) ev)->x_root = 0;
    ((XKeyEvent *) ev)->y_root = 0;

    on_incoming_message (xim, call_data);
}

static void ExtMoveMessageProc (NimfXim *xim,
                                IMProtocol *call_data,
                                unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec ext_move_fr[];
    IMMoveStruct *extmove =
        (IMMoveStruct*) & call_data->extmove;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (ext_move_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, extmove->icid);
    FrameMgrGetToken (fm, extmove->x);
    FrameMgrGetToken (fm, extmove->y);

    FrameMgrFree (fm);

    on_incoming_message (xim, call_data);
}

static void ExtensionMessageProc (NimfXim *xim,
                                  IMProtocol *call_data,
                                  unsigned char *p)
{
    switch (call_data->any.minor_code)
    {
    case XIM_EXT_FORWARD_KEYEVENT:
        ExtForwardKeyEventMessageProc (xim, call_data, p);
        break;

    case XIM_EXT_MOVE:
        ExtMoveMessageProc (xim, call_data, p);
        break;
    }
    /*endswitch*/
}

static INT16 ChooseEncoding (NimfXim *xim,
                             IMEncodingNegotiationStruct *enc_nego)
{
    int i;
    int enc_index=0;

    for (i = 0;  i < (int) enc_nego->encoding_number; i++)
    {
        if (strcmp (enc_nego->encoding[i].name, "COMPOUND_TEXT") == 0)
        {
            enc_index = i;
            break;
        }
    }

    return (INT16) enc_index;
}

static void EncodingNegotiatonMessageProc (NimfXim *xim,
                                           IMProtocol *call_data,
                                           unsigned char *p)
{
    FrameMgr fm;
    FmStatus status;
    CARD16 byte_length;
    extern XimFrameRec encoding_negotiation_fr[];
    extern XimFrameRec encoding_negotiation_reply_fr[];
    register int i, total_size;
    unsigned char *reply = NULL;
    IMEncodingNegotiationStruct *enc_nego =
        (IMEncodingNegotiationStruct *) &call_data->encodingnego;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (encoding_negotiation_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));

    FrameMgrGetToken (fm, input_method_ID);

    /* get ENCODING STR field */
    FrameMgrGetToken (fm, byte_length);
    if (byte_length > 0)
    {
        enc_nego->encoding = (XIMStr *) malloc (sizeof (XIMStr)*10);
        memset (enc_nego->encoding, 0, sizeof (XIMStr)*10);
        i = 0;
        while (FrameMgrIsIterLoopEnd (fm, &status) == False)
        {
            char *name;
            int str_length;

            FrameMgrGetToken (fm, str_length);
            FrameMgrSetSize (fm, str_length);
            enc_nego->encoding[i].length = str_length;
            FrameMgrGetToken (fm, name);
            enc_nego->encoding[i].name = malloc (str_length + 1);
            strncpy (enc_nego->encoding[i].name, name, str_length);
            enc_nego->encoding[i].name[str_length] = '\0';
            i++;
        }
        /*endwhile*/
        enc_nego->encoding_number = i;
    }
    /*endif*/
    /* get ENCODING INFO field */
    FrameMgrGetToken (fm, byte_length);
    if (byte_length > 0)
    {
        enc_nego->encodinginfo = (XIMStr *) malloc (sizeof (XIMStr)*10);
        memset (enc_nego->encodinginfo, 0, sizeof (XIMStr)*10);
        i = 0;
        while (FrameMgrIsIterLoopEnd (fm, &status) == False)
        {
            char *name;
            int str_length;

            FrameMgrGetToken (fm, str_length);
            FrameMgrSetSize (fm, str_length);
            enc_nego->encodinginfo[i].length = str_length;
            FrameMgrGetToken (fm, name);
            enc_nego->encodinginfo[i].name = malloc (str_length + 1);
            strncpy (enc_nego->encodinginfo[i].name, name, str_length);
            enc_nego->encodinginfo[i].name[str_length] = '\0';
            i++;
        }
        /*endwhile*/
        enc_nego->encoding_info_number = i;
    }
    /*endif*/

    enc_nego->enc_index = ChooseEncoding (xim, enc_nego);
    enc_nego->category = 0;

    FrameMgrFree (fm);

    fm = FrameMgrInit (encoding_negotiation_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (xim, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (xim, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);
    FrameMgrPutToken (fm, enc_nego->category);
    FrameMgrPutToken (fm, enc_nego->enc_index);

    _Xi18nSendMessage (xim,
                       connect_id,
                       XIM_ENCODING_NEGOTIATION_REPLY,
                       0,
                       reply,
                       total_size);
    XFree (reply);

    /* free data for encoding list */
    if (enc_nego->encoding)
    {
        for (i = 0;  i < (int) enc_nego->encoding_number;  i++)
            XFree (enc_nego->encoding[i].name);
        /*endfor*/
        XFree (enc_nego->encoding);
    }
    /*endif*/
    if (enc_nego->encodinginfo)
    {
        for (i = 0;  i < (int) enc_nego->encoding_info_number;  i++)
            XFree (enc_nego->encodinginfo[i].name);
        /*endfor*/
        XFree (enc_nego->encodinginfo);
    }
    /*endif*/
    FrameMgrFree (fm);
}

void PreeditStartReplyMessageProc (NimfXim *xim,
                                   IMProtocol *call_data,
                                   unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec preedit_start_reply_fr[];
    IMPreeditCBStruct *preedit_CB =
        (IMPreeditCBStruct *) &call_data->preedit_callback;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (preedit_start_reply_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, preedit_CB->icid);
    FrameMgrGetToken (fm, preedit_CB->todo.return_value);

    FrameMgrFree (fm);

    on_incoming_message (xim, call_data);
}

void PreeditCaretReplyMessageProc (NimfXim *xim,
                                   IMProtocol *call_data,
                                   unsigned char *p)
{
    FrameMgr fm;
    extern XimFrameRec preedit_caret_reply_fr[];
    IMPreeditCBStruct *preedit_CB =
        (IMPreeditCBStruct *) &call_data->preedit_callback;
    XIMPreeditCaretCallbackStruct *caret =
        (XIMPreeditCaretCallbackStruct *) & preedit_CB->todo.caret;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (preedit_caret_reply_fr,
                       (char *) p,
                       _Xi18nNeedSwap (xim, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, preedit_CB->icid);
    FrameMgrGetToken (fm, caret->position);

    FrameMgrFree (fm);

    on_incoming_message (xim, call_data);
}

static void AddQueue (Xi18nClient *client, unsigned char *p)
{
    XIMPending *new;
    XIMPending *last;

    if ((new = (XIMPending *) malloc (sizeof (XIMPending))) == NULL)
        return;
    /*endif*/
    new->p = p;
    new->next = (XIMPending *) NULL;
    if (!client->pending)
    {
        client->pending = new;
    }
    else
    {
        for (last = client->pending;  last->next;  last = last->next)
            ;
        /*endfor*/
        last->next = new;
    }
    /*endif*/
    return;
}

static void ProcessQueue (NimfXim *xim, CARD16 connect_id)
{
    Xi18nClient *client = (Xi18nClient *) _Xi18nFindClient (xim,
                                                            connect_id);

    while (client->sync == False  &&  client->pending)
    {
        XimProtoHdr *hdr = (XimProtoHdr *) client->pending->p;
        unsigned char *p1 = (unsigned char *) (hdr + 1);
        IMProtocol call_data;

        call_data.major_code = hdr->major_opcode;
        call_data.any.minor_code = hdr->minor_opcode;
        call_data.any.connect_id = connect_id;

        switch (hdr->major_opcode)
        {
        case XIM_FORWARD_EVENT:
            ForwardEventMessageProc(xim, &call_data, p1);
            break;
        }
        /*endswitch*/
        XFree (hdr);
        {
            XIMPending *old = client->pending;

            client->pending = old->next;
            XFree (old);
        }
    }
    /*endwhile*/
    return;
}


void _Xi18nMessageHandler (NimfXim *xim,
                           CARD16 connect_id,
                           unsigned char *p,
                           Bool *delete)
{
    XimProtoHdr	*hdr = (XimProtoHdr *)p;
    unsigned char *p1 = (unsigned char *)(hdr + 1);
    IMProtocol call_data;
    Xi18nClient *client;

    client = (Xi18nClient *) _Xi18nFindClient (xim, connect_id);
    if (hdr == (XimProtoHdr *) NULL)
        return;
    /*endif*/

    memset (&call_data, 0, sizeof(IMProtocol));

    call_data.major_code = hdr->major_opcode;
    call_data.any.minor_code = hdr->minor_opcode;
    call_data.any.connect_id = connect_id;

    switch (call_data.major_code)
    {
    case XIM_CONNECT:
        ConnectMessageProc (xim, &call_data, p1);
        break;

    case XIM_DISCONNECT:
        DisConnectMessageProc (xim, &call_data);
        break;

    case XIM_OPEN:
        OpenMessageProc (xim, &call_data, p1);
        break;

    case XIM_CLOSE:
        CloseMessageProc (xim, &call_data, p1);
        break;

    case XIM_QUERY_EXTENSION:
        QueryExtensionMessageProc (xim, &call_data, p1);
        break;

    case XIM_GET_IM_VALUES:
        GetIMValuesMessageProc (xim, &call_data, p1);
        break;

    case XIM_CREATE_IC:
        CreateICMessageProc (xim, &call_data, p1);
        break;

    case XIM_SET_IC_VALUES:
        SetICValuesMessageProc (xim, &call_data, p1);
        break;

    case XIM_GET_IC_VALUES:
        GetICValuesMessageProc (xim, &call_data, p1);
        break;

    case XIM_SET_IC_FOCUS:
        SetICFocusMessageProc (xim, &call_data, p1);
        break;

    case XIM_UNSET_IC_FOCUS:
        UnsetICFocusMessageProc (xim, &call_data, p1);
        break;

    case XIM_DESTROY_IC:
        DestroyICMessageProc (xim, &call_data, p1);
        break;

    case XIM_RESET_IC:
        ResetICMessageProc (xim, &call_data, p1);
        break;

    case XIM_FORWARD_EVENT:
        if (client->sync == True)
        {
            AddQueue (client, p);
            *delete = False;
        }
        else
        {
            ForwardEventMessageProc (xim, &call_data, p1);
        }
        break;

    case XIM_EXTENSION:
        ExtensionMessageProc (xim, &call_data, p1);
        break;

    case XIM_SYNC:
        break;

    case XIM_SYNC_REPLY:
        SyncReplyMessageProc (xim, &call_data, p1);
        ProcessQueue (xim, connect_id);
        break;

    case XIM_ENCODING_NEGOTIATION:
        EncodingNegotiatonMessageProc (xim, &call_data, p1);
        break;

    case XIM_PREEDIT_START_REPLY:
        PreeditStartReplyMessageProc (xim, &call_data, p1);
        break;

    case XIM_PREEDIT_CARET_REPLY:
        PreeditCaretReplyMessageProc (xim, &call_data, p1);
        break;

    case XIM_STR_CONVERSION_REPLY:
        break;
    }
    /*endswitch*/
}
