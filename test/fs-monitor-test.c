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

#include "genesis-common.h"

static gboolean monitor_callback (GenesisFSMonitor *monitor, const gchar *path, 
                                  GenesisFSMonitorEventType type, gpointer data)
{
  g_print ("event %d happened to %s\n", type, path);
  return TRUE;
}

int main(int argc, char *argv[])
{
	GenesisFSMonitor *monitor = NULL;
	GMainLoop* mainloop;
//	GError* error = NULL;

	g_type_init ();

	if (!g_thread_supported ())
		g_thread_init (NULL);
	
	monitor = genesis_fs_monitor_get_singleton ();


	if (!monitor)
	{
		g_warning ("Failed to get probot monitor singleton, returned.\n");
		return -1;
	}
	
	genesis_fs_monitor_add (monitor, "/usr/share/applications/", IN_ALL_EVENTS, monitor_callback, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);

	if (mainloop == NULL) {
	 	g_error("Failed to create the mainloop\n");
	}
	
	g_main_loop_run(mainloop);


  return 0;
}
