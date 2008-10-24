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
#include "genesis-daemon.h"
#include "genesis-dbus-common.h"
#include "genesis-daemon-dbus.h"

#if 1
static gboolean applications_list_updated (GenesisFSMonitor *monitor, const gchar *path,
                                           GenesisFSMonitorEventType type, gpointer data)
{
  gchar *desktop_entry = NULL;
  GenesisController *controller = genesis_controller_get_singleton ();
  GenesisAppEntry *entry = NULL;
  gchar *nth_desktop_entry = NULL;
  guint n = 0;

  save_log ("event %d happened to %s\n", type, path);

  if (!path && !g_str_has_suffix (path, DESKTOP_FILE_SUFFIX))
    return FALSE;

//FIXME: just a test code for sending out event here. need to come back and refine it.
  genesis_dbusobj_emit_signal(  ( (GenesisDaemon*)data)->dbusobj,
  			E_SIGNAL_GENESIS_ENTRY_UPDATED, "Entry Updated");

  desktop_entry = g_strdup (path);
  switch (type)
  {
    case EVENT_MODIFIED:
      do
      {
        entry = genesis_controller_get_nth_entry (controller, n);

        if (!entry)
          break;

        g_object_get (G_OBJECT(entry), "desktop_entry", &nth_desktop_entry, NULL);

        if (!g_ascii_strcasecmp (nth_desktop_entry, desktop_entry))
        {
          genesis_controller_remove_entry (controller, entry);
          break;
        }

        n++;
      }while (1);

      entry = g_object_new (GENESIS_TYPE_APP_ENTRY, "desktop_entry", desktop_entry, NULL);

      genesis_controller_add_entry (controller, entry);
      g_signal_emit_by_name (controller, "app-entry-updated", type, desktop_entry);
      break;
    case EVENT_REMOVED:
      do
      {
        entry = genesis_controller_get_nth_entry (controller, n);

        if (!entry)
          break;

        g_object_get (G_OBJECT(entry), "desktop_entry", &nth_desktop_entry, NULL);

        if (!g_ascii_strcasecmp (nth_desktop_entry, desktop_entry))
        {
          genesis_controller_remove_entry (controller, entry);
          g_signal_emit_by_name (controller, "app-entry-updated", type, desktop_entry);
          break;
        }

        n++;
      }while (1);
      break;
    case EVENT_CREATED:
      entry = g_object_new (GENESIS_TYPE_APP_ENTRY, "desktop_entry", desktop_entry, NULL);

      genesis_controller_add_entry (controller, entry);
      g_signal_emit_by_name (controller, "app-entry-updated", type, desktop_entry);
      break;
    default:
      break;
  }

  return TRUE;
}

#endif

static void genesis_daemon_init (GenesisDaemon *daemon)
{
  GenesisController *controller = NULL;
  GenesisFSMonitor *monitor = NULL;
  GenesisDbusObj *dbusobj = NULL;


  save_log("enter genesis_daemon_init \n");
  controller = genesis_controller_get_singleton ();

  if (!controller)
  {
    save_log ("no controller singleton returned\n");
    exit (EXIT_FAILURE);
  }
  save_log ("controller singleton geted\n");

  monitor = genesis_fs_monitor_get_singleton ();

  if (!monitor)
  {
    save_log ("no monitor singleton returned\n");
    exit (EXIT_FAILURE);
  }
  save_log ("monitor singleton geted\n");

  daemon->controller = controller;
  daemon->monitor = monitor;

  dbusobj = genesis_dbus_daemon_init(daemon);

  daemon->dbusobj = dbusobj;

  genesis_fs_monitor_add (monitor, DESKTOP_DIR, IN_ALL_EVENTS, applications_list_updated, daemon);
  
}

int main (int argc, char **argv)
{
  GenesisDaemon *daemon = NULL;
  GMainLoop *loop;

#ifndef NODAEMON
  pid_t pid, sid;
  
  pid = fork ();
  if (pid > 0)
    exit (EXIT_SUCCESS);
  else if (pid < 0)
    exit (EXIT_FAILURE);

  save_log ("now in the child process\n");
  umask (0);

  sid = setsid ();
  if (sid < 0)
  {
    save_log ("setsid : exit (EXIT_FAILURE)\n");
    exit (EXIT_FAILURE);
  }

  if (chdir ("/") < 0)
  {
    save_log ("chdir : exit (EXIT_FAILURE)\n");
    exit (EXIT_FAILURE);
  }

  close (STDIN_FILENO);
  close (STDOUT_FILENO);
  close (STDERR_FILENO);
#endif

#ifndef ENABLE_SPLASH
  	g_type_init ();
#else
  gtk_init(&argc, &argv);
#endif

  if (!g_thread_supported ())
    g_thread_init (NULL);
  
  daemon = g_new0 (GenesisDaemon, 1);

  printf("in main, start to call genesis_daemon_init\n");

  save_log("in main, start to call genesis_daemon_init\n");

  genesis_daemon_init (daemon);

  save_log ("in main, after call genesis_daemon_init\n");
  loop = g_main_loop_new (NULL, FALSE);

  g_main_loop_run (loop);

  save_log ("quit from main loop\n");
  
  if (daemon)
    g_free (daemon);

  return 0;
}
