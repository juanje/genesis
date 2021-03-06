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

#ifndef GENESIS_DAEMON_H
#define GENESIS_DAEMON_H

#define DEFAULT_DESKTOP_DIR	"/usr/share/applications/"

struct  _GenesisDbusObj;

typedef struct _GenesisDaemon
{
	GenesisController *controller;
	GenesisFSMonitor *monitor;
	struct _GenesisDbusObj *dbusobj;
} GenesisDaemon;


typedef struct _genesisd_opts {
	gboolean no_daemon;
	gchar *desktop_files_dir;
}genesisd_opts;

extern genesisd_opts genesisd;

#endif /* GENESIS_DAEMON_H */
