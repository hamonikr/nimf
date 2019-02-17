/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-service.c
 * This file is part of Nimf.
 *
 * Copyright (C) 2017 Hodong Kim <cogniti@gmail.com>
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

#include "nimf-service.h"

G_DEFINE_ABSTRACT_TYPE (NimfService, nimf_service, G_TYPE_OBJECT);

enum
{
  PROP_0,
  PROP_SERVER
};

static void
nimf_service_init (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
nimf_service_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  G_OBJECT_CLASS (nimf_service_parent_class)->finalize (object);
}

const gchar *nimf_service_real_get_id (NimfService *service)
{
  g_error (G_STRLOC ": %s: You should implement your_service_get_id ()",
           G_STRFUNC);

  return NULL;
}

const gchar *nimf_service_get_id (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_SERVICE_GET_CLASS (service)->get_id (service);
}

gboolean nimf_service_start (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceClass *class = NIMF_SERVICE_GET_CLASS (service);

  if (class->start)
    return class->start (service);
  else
    return FALSE;
}

void nimf_service_stop (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceClass *class = NIMF_SERVICE_GET_CLASS (service);

  if (class->stop)
    class->stop (service);
}

gboolean nimf_service_real_is_active (NimfService *service)
{
  g_error (G_STRLOC ": %s: You should implement your_service_is_active ()",
           G_STRFUNC);
  return FALSE;
}

gboolean nimf_service_is_active (NimfService *service)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return NIMF_SERVICE_GET_CLASS (service)->is_active (service);
}

void
nimf_service_set_engine_by_id (NimfService *service,
                               const gchar *engine_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  NimfServiceClass *class = NIMF_SERVICE_GET_CLASS (service);

  if (class->set_engine_by_id)
    class->set_engine_by_id (service, engine_id);
}

static void
nimf_service_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_SERVICE (object));

  NimfService *service = NIMF_SERVICE (object);

  switch (prop_id)
  {
    case PROP_SERVER:
      service->server = g_value_get_object (value);
      g_object_notify_by_pspec (object, pspec);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
nimf_service_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (NIMF_IS_SERVICE (object));

  NimfService *service = NIMF_SERVICE (object);

  switch (prop_id)
  {
    case PROP_SERVER:
      g_value_set_object (value, service->server);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
nimf_service_class_init (NimfServiceClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize     = nimf_service_finalize;
  object_class->set_property = nimf_service_set_property;
  object_class->get_property = nimf_service_get_property;

  class->get_id    = nimf_service_real_get_id;
  class->is_active = nimf_service_real_is_active;

  g_object_class_install_property (object_class,
                                   PROP_SERVER,
                                   g_param_spec_object ("server",
                                                        "server",
                                                        "server",
                                                        NIMF_TYPE_SERVER,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}
