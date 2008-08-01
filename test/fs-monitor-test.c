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

#include "gtk/gtk.h"
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

  gtk_init (&argc, &argv);

  monitor = genesis_fs_monitor_get_singleton ();

  if (!monitor)
  {
    g_warning ("Failed to get probot monitor singleton, returned.\n");
    return -1;
  }

  genesis_fs_monitor_add (monitor, "/usr/share/applications/", IN_ALL_EVENTS, monitor_callback, NULL);

  gtk_main ();

  return 0;
}
