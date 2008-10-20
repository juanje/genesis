/*
 * Copyright (C) 2008 Intel Corporation
 *
 * Author:  Raymond Liu <raymond.liu@intel.com>
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

#ifndef GENESIS_DAEMON_DBUS_H
#define GENESIS_DAEMON_DBUS_H

typedef struct {

  GObject parent;

} GenesisDbusObj;

/* Per class state. */

typedef struct {

  GObjectClass parent;
} GenesisDbusObjClass;

GType genesis_dbusobj_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define GENESIS_TYPE_DBUSOBJ (genesis_dbusobj_get_type())

#define GENESIS_DBUSOBJ(object) \
        (G_TYPE_CHECK_INSTANCE_CAST((object), GENESIS_TYPE_DBUSOBJ, GenesisDbusObj))
#define GENESIS_DBUSOBJ_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST((klass), GENESIS_TYPE_DBUSOBJ, GenesisDbusObjClass))
#define GENESIS_IS_DBUSOBJ(object) \
        (G_TYPE_CHECK_INSTANCE_TYPE((object), GENESIS_TYPE_DBUSOBJ))
#define GENESIS_IS_DBUSOBJ_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE((klass), GENESIS_TYPE_DBUSOBJ))
#define GENESIS_DBUSOBJ_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS((obj), GENESIS_TYPE_DBUSOBJ, GenesisDbusObjClass))


/* Public functions */
GenesisDbusObj *genesis_dbus_daemon_init(void);

#endif /* GENESIS_DAEMON_DBUS_H */
