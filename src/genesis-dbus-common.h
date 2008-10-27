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

#ifndef GENESIS_DBUS_COMMON_H
#define GENESIS_DBUS_COMMON_H

/* Well-known name for this service. */
#define GENESIS_DBUSOBJ_SERVICE_NAME        "org.moblin.genesis"
/* Object path to the provided object. */
#define GENESIS_DBUSOBJ_SERVICE_OBJECT_PATH	"/org/moblin/genesis"
/* And we're interested in using it through this interface.
 *    This must match the entry in the interface definition XML. */
#define GENESIS_DBUSOBJ_SERVICE_INTERFACE   "org.moblin.genesis"


/* Signal Defination */

#define SIGNAL_GENESIS_ENTRY_UPDATED    "entry_updated"

typedef enum {
	E_SIGNAL_GENESIS_ENTRY_UPDATED,
	E_SIGNAL_GENESIS_COUNT
} GenesisSignalNum;



typedef enum {
	GENESIS_DBUS_TEST_EMIT_SIGNAL
} GenesisDbusTestID;

#endif /* GENESIS_DBUS_COMMON_H */
