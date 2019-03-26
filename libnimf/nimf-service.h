/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-service.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2017-2019 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_SERVICE_H__
#define __NIMF_SERVICE_H__

#if !defined (__NIMF_H_INSIDE__) && !defined (NIMF_COMPILATION)
#error "Only <nimf.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define NIMF_TYPE_SERVICE             (nimf_service_get_type ())
#define NIMF_SERVICE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIMF_TYPE_SERVICE, NimfService))
#define NIMF_SERVICE_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), NIMF_TYPE_SERVICE, NimfServiceClass))
#define NIMF_IS_SERVICE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIMF_TYPE_SERVICE))
#define NIMF_SERVICE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIMF_TYPE_SERVICE, NimfServiceClass))

typedef struct _NimfService       NimfService;
typedef struct _NimfServiceClass  NimfServiceClass;

struct _NimfService
{
  GObject parent_instance;
};

/**
 * NimfServiceClass:
 * @get_id: Returns a service id.
 * @start: Starts a service.
 * @stop: Stops a service.
 * @is_active: Whether a service is active or not
 * @change_engine_by_id: Changes an engine by engine id.
 * @change_engine: Changes an engine with engine id and method id.
 */
struct _NimfServiceClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Virtual functions */
  const gchar * (* get_id)              (NimfService *service);
  gboolean      (* start)               (NimfService *service);
  void          (* stop)                (NimfService *service);
  gboolean      (* is_active)           (NimfService *service);
  void          (* change_engine_by_id) (NimfService *service,
                                         const gchar *engine_id);
  void          (* change_engine)       (NimfService *service,
                                         const gchar *engine_id,
                                         const gchar *method_id);
};

GType        nimf_service_get_type            (void) G_GNUC_CONST;
gboolean     nimf_service_start               (NimfService *service);
void         nimf_service_stop                (NimfService *service);
gboolean     nimf_service_is_active           (NimfService *service);
const gchar *nimf_service_get_id              (NimfService *service);
void         nimf_service_change_engine_by_id (NimfService *service,
                                               const gchar *engine_id);
void         nimf_service_change_engine       (NimfService *service,
                                               const gchar *engine_id,
                                               const gchar *method_id);

G_END_DECLS

#endif /* __NIMF_SERVICE_H__ */
