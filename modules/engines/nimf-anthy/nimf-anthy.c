/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * nimf-anthy.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2016 Hodong Kim <cogniti@gmail.com>
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

#include <nimf.h>
#include <anthy/anthy.h>

#define NIMF_TYPE_ANTHY             (nimf_anthy_get_type ())
#define NIMF_ANTHY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_ANTHY, NimfAnthy))
#define NIMF_ANTHY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIMF_TYPE_ANTHY, NimfAnthyClass))
#define NIMF_IS_ANTHY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_ANTHY))
#define NIMF_IS_ANTHY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIMF_TYPE_ANTHY))
#define NIMF_ANTHY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_ANTHY, NimfAnthyClass))

typedef struct _NimfAnthy      NimfAnthy;
typedef struct _NimfAnthyClass NimfAnthyClass;

struct _NimfAnthy
{
  NimfEngine parent_instance;

  gchar    *preedit_string;
  gchar    *id;
  gchar    *ja_name;
  gboolean  is_english_mode;

  anthy_context_t context;
};

struct _NimfAnthyClass
{
  /*< private >*/
  NimfEngineClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE (NimfAnthy, nimf_anthy, NIMF_TYPE_ENGINE);

void
nimf_anthy_reset (NimfEngine     *engine,
                  NimfConnection *target,
                  guint16         client_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
nimf_anthy_focus_in (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
nimf_anthy_focus_out (NimfEngine     *engine,
                      NimfConnection *target,
                      guint16         client_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
on_candidate_clicked (NimfEngine     *engine,
                      NimfConnection *target,
                      guint16         client_id,
                      gchar          *text,
                      gint            index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

gboolean
nimf_anthy_filter_event (NimfEngine     *engine,
                         NimfConnection *target,
                         guint16         client_id,
                         NimfEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return FALSE;
}

static void
nimf_anthy_init (NimfAnthy *anthy)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  anthy->preedit_string = g_strdup ("");
  anthy->id      = g_strdup ("nimf-anthy");
  anthy->ja_name = g_strdup ("ja");
  anthy->is_english_mode = TRUE;

  anthy_init ();
  anthy->context = anthy_create_context ();
  /* experimental and unstable api */
  anthy_context_set_encoding(anthy->context, ANTHY_UTF8_ENCODING);
}

static void
nimf_anthy_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfAnthy *anthy = NIMF_ANTHY (object);

  g_free (anthy->preedit_string);
/* commented due to segmentation fault
  anthy_release_context (anthy->context);
  anthy_quit (); */

  G_OBJECT_CLASS (nimf_anthy_parent_class)->finalize (object);
}

void
nimf_anthy_get_preedit_string (NimfEngine  *engine,
                               gchar      **str,
                               gint        *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_ENGINE (engine));

  NimfAnthy *anthy = NIMF_ANTHY (engine);

  if (str)
  {
    if (anthy->preedit_string)
      *str = g_strdup (anthy->preedit_string);
    else
      *str = g_strdup ("");
  }

  if (cursor_pos)
  {
    if (anthy->preedit_string)
      *cursor_pos = g_utf8_strlen (anthy->preedit_string, -1);
    else
      *cursor_pos = 0;
  }
}

const gchar *
nimf_anthy_get_id (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_ANTHY (engine)->id;
}

const gchar *
nimf_anthy_get_icon_name (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (NIMF_IS_ENGINE (engine), NULL);

  return NIMF_ANTHY (engine)->ja_name;
}

void
nimf_anthy_set_english_mode (NimfEngine *engine,
                             gboolean    is_english_mode)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NIMF_ANTHY (engine)->is_english_mode = is_english_mode;
}

gboolean
nimf_anthy_get_english_mode (NimfEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_ANTHY (engine)->is_english_mode;
}

static void
nimf_anthy_class_init (NimfAnthyClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  NimfEngineClass *engine_class = NIMF_ENGINE_CLASS (class);

  engine_class->filter_event       = nimf_anthy_filter_event;
  engine_class->get_preedit_string = nimf_anthy_get_preedit_string;
  engine_class->reset              = nimf_anthy_reset;
  engine_class->focus_in           = nimf_anthy_focus_in;
  engine_class->focus_out          = nimf_anthy_focus_out;

  engine_class->candidate_clicked  = on_candidate_clicked;

  engine_class->get_id             = nimf_anthy_get_id;
  engine_class->get_icon_name      = nimf_anthy_get_icon_name;
  engine_class->set_english_mode   = nimf_anthy_set_english_mode;
  engine_class->get_english_mode   = nimf_anthy_get_english_mode;

  object_class->finalize = nimf_anthy_finalize;
}

static void
nimf_anthy_class_finalize (NimfAnthyClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_register_type (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  nimf_anthy_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return nimf_anthy_get_type ();
}
