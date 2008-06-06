/*
 * Copyright (C) 2008 Intel Corporation
 *
 * Author:  Horace Li <horace.li@intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef PROBOT_CONTROLLER_H
#define PROBOT_CONTROLLER_H

G_BEGIN_DECLS

typedef struct _ProbotController ProbotController;
typedef struct _ProbotControllerClass ProbotControllerClass;
typedef struct _ProbotControllerPrivate ProbotControllerPrivate;

#define PROBOT_TYPE_CONTROLLER                (probot_controller_get_type ())
#define PROBOT_CONTROLLER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), PROBOT_TYPE_CONTROLLER, ProbotController))
#define PROBOT_CONTROLLER_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), PROBOT_TYPE_CONTROLLER, ProbotControllerClass))
#define IS_PROBOT_CONTROLLER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PROBOT_TYPE_CONTROLLER))
#define IS_PROBOT_CONTROLLER_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), PROBOT_TYPE_CONTROLLER))
#define PROBOT_CONTROLLER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), PROBOT_TYPE_CONTROLLER, ProbotControllerClass))

struct _ProbotController {
  GObject parent;

  ProbotControllerPrivate *priv;
};

struct _ProbotControllerClass {
  GObjectClass parent;
};

GType probot_controller_get_type (void);

/* Public functions */

ProbotController *probot_controller_get_singleton (void);
gboolean probot_controller_start_app_from_path (ProbotController *controller, gchar *path);
gboolean probot_controller_start_app_from_name (ProbotController *controller, gchar* name);
ProbotAppEntry *probot_controller_get_nth_entry (ProbotController *controller, guint n);
ProbotAppEntry *probot_controller_get_entry_by_name (ProbotController *controller, gchar* name);
void probot_controller_remove_entry (ProbotController *controller, ProbotAppEntry *entry);

G_END_DECLS

#endif /* PROBOT_CONTROLLER_H */
