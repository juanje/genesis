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

#ifndef GENESIS_FS_MONITOR_H
#define GENESIS_FS_MONITOR_H

G_BEGIN_DECLS

typedef enum
{
  EVENT_CREATED,
  EVENT_REMOVED,
  EVENT_MODIFIED,
  N_EVENT_TYPE
} GenesisFSMonitorEventType;

typedef struct _GenesisFSMonitor GenesisFSMonitor;
typedef struct _GenesisFSMonitorClass GenesisFSMonitorClass;
typedef struct _GenesisFSMonitorPrivate GenesisFSMonitorPrivate;
typedef struct _GenesisWatchNode GenesisWatchNode;
typedef gboolean (*GenesisFSMonitorCallback) (GenesisFSMonitor *monitor, const gchar *path, GenesisFSMonitorEventType type, gpointer data);

#define GENESIS_TYPE_FS_MONITOR                (genesis_fs_monitor_get_type ())
#define GENESIS_FS_MONITOR(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), GENESIS_TYPE_FS_MONITOR, GenesisFSMonitor))
#define GENESIS_FS_MONITOR_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), GENESIS_TYPE_FS_MONITOR, GenesisFSMonitorClass))
#define IS_GENESIS_FS_MONITOR(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GENESIS_TYPE_FS_MONITOR))
#define IS_GENESIS_FS_MONITOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), GENESIS_TYPE_FS_MONITOR))
#define GENESIS_FS_MONITOR_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), GENESIS_TYPE_FS_MONITOR, GenesisFSMonitorClass))

struct _GenesisFSMonitor {
  GObject parent;

  GenesisFSMonitorPrivate *priv;
};

struct _GenesisFSMonitorClass {
  GObjectClass parent;
};

GType genesis_fs_monitor_get_type (void);

/* Public functions */

GenesisFSMonitor *genesis_fs_monitor_get_singleton (void);
void genesis_fs_monitor_add (GenesisFSMonitor *monitor, const gchar *path, gint events,
                             GenesisFSMonitorCallback callback, gpointer data);

G_END_DECLS

#endif /* GENESIS_FS_MONITOR_H */
