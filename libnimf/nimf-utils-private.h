/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * nimf-utils-private.h
 * This file is part of Nimf.
 *
 * Copyright (C) 2019 Hodong Kim <cogniti@gmail.com>
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

#ifndef __NIMF_UTILS_PRIVATE_H__
#define __NIMF_UTILS_PRIVATE_H__

#include <glib.h>

G_BEGIN_DECLS

gboolean gnome_is_running       (void);
gboolean gnome_xkb_is_available (void);
G_END_DECLS

#endif /* __NIMF_UTILS_PRIVATE_H__ */
