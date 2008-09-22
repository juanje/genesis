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

#define MAXIMUM_NUMBER 4096

#define GENESIS_FS_MONITOR_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), GENESIS_TYPE_FS_MONITOR, GenesisFSMonitorPrivate))

G_DEFINE_TYPE (GenesisFSMonitor, genesis_fs_monitor, G_TYPE_OBJECT)

struct _GenesisWatchNode
{
  GenesisFSMonitorCallback callback;
  gpointer data;
  gchar *path;
  gint watch_descriptor;
};

struct _GenesisFSMonitorPrivate
{
  GThread *thread; 
  gint inotify_handle;
  struct inotify_event event[MAXIMUM_NUMBER];
};

static GList *monitored_list;

static void genesis_fs_monitor_updated_callback (GenesisFSMonitor *monitor, gchar *path, GenesisWatchNode *node, guint event_type, gpointer user_data)
{
  node->callback (monitor, path, event_type, node->data);
}

static GenesisWatchNode *genesis_fs_monitor_get_node_by_wd (gint watch_descriptor)
{
  GList *iter;

  for (iter = monitored_list; iter; iter = g_list_next (iter))
  {
    GenesisWatchNode *node = (GenesisWatchNode *) iter->data;
    
    if (node->watch_descriptor == watch_descriptor)
        return node;
  }

  return NULL;
}

static struct inotify_event* genesis_fs_monitor_get_next_event (GenesisFSMonitor *monitor)
{
  GenesisFSMonitorPrivate *priv = GENESIS_FS_MONITOR_GET_PRIVATE (monitor);
  static fd_set set;
  guint read_out_bytes;
  gint ret;

  FD_ZERO(&set);
  FD_SET(priv->inotify_handle, &set);
  ret = select (priv->inotify_handle + 1, &set, NULL, NULL, NULL);
  if (ret <= 0)
    return NULL;

  do
  {
    ret = ioctl (priv->inotify_handle, FIONREAD, &read_out_bytes);
    if (ret == 0)
      break;
  } while (read_out_bytes < sizeof (struct inotify_event));

  if (ret <= 0)
    return NULL;

  read_out_bytes = read (priv->inotify_handle, &priv->event[0], sizeof(struct inotify_event)*MAXIMUM_NUMBER);

  if (read_out_bytes <= 0)
    return NULL;

  return &priv->event[0];
}

static gpointer genesis_fs_monitor_main_thread (gpointer data)
{
  GenesisFSMonitor *monitor = GENESIS_FS_MONITOR (data);
  struct inotify_event *ret_event;
  GenesisWatchNode *node = NULL;

  while (1)
  {
    ret_event = genesis_fs_monitor_get_next_event (monitor);

    if (!ret_event)
      continue;

    node = genesis_fs_monitor_get_node_by_wd (ret_event->wd);

    if (!node)
      continue;

    switch (ret_event->mask)
    {
      case IN_MODIFY:
      case IN_CREATE:
      case IN_MOVED_TO:
        save_log ("IN_MODIFY | IN_CREATE | IN_MOVED_TO :: ret_event->mask is %d\n", ret_event->mask);
        g_signal_emit_by_name (monitor, "directory-updated", g_build_filename (node->path, ret_event->name, NULL), node, EVENT_CREATED);
        break;
      case IN_MOVED_FROM:
      case IN_DELETE:
        save_log ("IN_MOVED_FROM | IN_DELETE :: ret_event->mask is %d\n", ret_event->mask);
        g_signal_emit_by_name (monitor, "directory-updated", g_build_filename (node->path, ret_event->name, NULL), node, EVENT_REMOVED);
        break;
      case IN_CLOSE_WRITE:
        save_log ("IN_CLOSE_WRITE :: ret_event->mask is %d\n", ret_event->mask);
        g_signal_emit_by_name (monitor, "directory-updated", g_build_filename (node->path, ret_event->name, NULL), node, EVENT_MODIFIED);
        break;
      default:
        break;
    }
  }

  return NULL;
}

static void genesis_fs_monitor_finalize (GObject *object)
{
  GenesisFSMonitor *self = GENESIS_FS_MONITOR (object);
  GenesisFSMonitorPrivate *priv = GENESIS_FS_MONITOR_GET_PRIVATE (self);

  if (priv->inotify_handle)
    close (priv->inotify_handle);
}

static void genesis_fs_monitor_class_init (GenesisFSMonitorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GenesisFSMonitorPrivate));

  object_class->finalize = genesis_fs_monitor_finalize;

  g_signal_new ("directory-updated",
                G_OBJECT_CLASS_TYPE(object_class),
                G_SIGNAL_RUN_LAST, 0,
                NULL, NULL,
                g_cclosure_user_marshal_VOID__POINTER_POINTER_UINT,
                G_TYPE_NONE,
                3,
                G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_UINT);
}

static void genesis_fs_monitor_init (GenesisFSMonitor *self)
{
  GenesisFSMonitorPrivate *priv = GENESIS_FS_MONITOR_GET_PRIVATE (self);

  priv->inotify_handle = inotify_init ();
  monitored_list = NULL;

  priv->thread = g_thread_create (genesis_fs_monitor_main_thread,
                                  self,
                                  FALSE,
                                  NULL);
}

static GList* genesis_fs_monitor_add_recursively (GenesisFSMonitor *monitor, 
                                                  const gchar *path, gint events,
                                                  GenesisFSMonitorCallback callback, gpointer data)
{
  GenesisFSMonitorPrivate *priv = GENESIS_FS_MONITOR_GET_PRIVATE (monitor);
  GenesisWatchNode *node = NULL;

  if (!priv->inotify_handle)
    return NULL;

  node = g_new0 (GenesisWatchNode, 1);

  if (g_file_test (path, G_FILE_TEST_IS_DIR))
  {
    DIR *dir_handle = NULL;
    struct dirent *d = NULL;
    gchar *filename = NULL;

    if ((dir_handle = opendir (path)) == NULL)
      g_warning ("Error reading file '%s'\n", path);

    while ((d = readdir (dir_handle)) != NULL)
    {
      if (!g_ascii_strcasecmp (d->d_name, ".") ||
         (!g_ascii_strcasecmp (d->d_name, "..")))
        continue;

      filename = g_build_filename (path, d->d_name, NULL);
      if (g_file_test (filename, G_FILE_TEST_IS_DIR))
        monitored_list = genesis_fs_monitor_add_recursively (monitor, filename, events, callback, data);
    }
  }

  node->watch_descriptor = inotify_add_watch (priv->inotify_handle, path, events);
  node->path = g_strdup (path);
  node->callback = callback;
  node->data = data;

  monitored_list = g_list_append (monitored_list, node);

  return monitored_list;
}

/* Public Functions */

GenesisFSMonitor *genesis_fs_monitor_get_singleton (void)
{
  static GenesisFSMonitor *monitor = NULL;

  if (!monitor)
    monitor = g_object_new (GENESIS_TYPE_FS_MONITOR, NULL);

  return monitor;
}

void genesis_fs_monitor_add (GenesisFSMonitor *monitor, const gchar *path, gint events,
                             GenesisFSMonitorCallback callback, gpointer data)
{
  monitored_list = genesis_fs_monitor_add_recursively (monitor, path, events, callback, data);

  g_signal_connect (G_OBJECT (monitor), "directory-updated", G_CALLBACK (genesis_fs_monitor_updated_callback), NULL);
}

