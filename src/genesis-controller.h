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

#ifndef GENESIS_CONTROLLER_H
#define GENESIS_CONTROLLER_H

G_BEGIN_DECLS

typedef struct _GenesisController GenesisController;
typedef struct _GenesisControllerClass GenesisControllerClass;
typedef struct _GenesisControllerPrivate GenesisControllerPrivate;

#define GENESIS_TYPE_CONTROLLER                (genesis_controller_get_type ())
#define GENESIS_CONTROLLER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), GENESIS_TYPE_CONTROLLER, GenesisController))
#define GENESIS_CONTROLLER_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), GENESIS_TYPE_CONTROLLER, GenesisControllerClass))
#define IS_GENESIS_CONTROLLER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GENESIS_TYPE_CONTROLLER))
#define IS_GENESIS_CONTROLLER_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), GENESIS_TYPE_CONTROLLER))
#define GENESIS_CONTROLLER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), GENESIS_TYPE_CONTROLLER, GenesisControllerClass))

struct _GenesisController {
  GObject parent;

  GenesisControllerPrivate *priv;
};

struct _GenesisControllerClass {
  GObjectClass parent;
};

GType genesis_controller_get_type (void);

/* Public functions */

GenesisController *genesis_controller_get_singleton (void);
gboolean genesis_controller_start_app_from_path (GenesisController *controller, gchar *path);
gboolean genesis_controller_start_app_from_name (GenesisController *controller, gchar* name);
GenesisAppEntry *genesis_controller_get_nth_entry (GenesisController *controller, guint n);
GenesisAppEntry *genesis_controller_get_entry_by_name (GenesisController *controller, gchar* name);
void genesis_controller_remove_entry (GenesisController *controller, GenesisAppEntry *entry);
GList* genesis_controller_get_categories (GenesisController *controller);
GList* genesis_controller_get_applications_by_category (GenesisController *controller, const gchar *name);

G_END_DECLS

#endif /* GENESIS_CONTROLLER_H */
