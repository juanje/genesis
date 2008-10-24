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

#ifndef GENESIS_PROXY_H
#define GENESIS_PROXY_H

#include "genesis-dbus-common.h"

G_BEGIN_DECLS

typedef struct _GenesisProxy GenesisProxy;
typedef struct _GenesisProxyClass GenesisProxyClass;
typedef struct _GenesisProxyPrivate GenesisProxyPrivate;

#define GENESIS_TYPE_PROXY                (genesis_proxy_get_type ())
#define GENESIS_PROXY(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), GENESIS_TYPE_PROXY, GenesisProxy))
#define GENESIS_PROXY_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), GENESIS_TYPE_PROXY, GenesisProxyClass))
#define IS_GENESIS_PROXY(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GENESIS_TYPE_PROXY))
#define IS_GENESIS_PROXY_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), GENESIS_TYPE_PROXY))
#define GENESIS_PROXY_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), GENESIS_TYPE_PROXY, GenesisProxyClass))

struct _GenesisProxy {
  GObject parent;

  GenesisProxyPrivate *priv;
};

struct _GenesisProxyClass {
  GObjectClass parent;
};

GType genesis_proxy_get_type (void);

/* Public functions */

GenesisProxy *genesis_proxy_get_singleton (void);
void genesis_proxy_hello (GenesisProxy *proxy, gchar * name);
gboolean genesis_proxy_start_app_by_name (GenesisProxy *proxy, gchar* name);
gchar *genesis_proxy_get_nth_entry_name (GenesisProxy *proxy, gint index);
gchar **genesis_proxy_get_category_names (GenesisProxy *proxy);
gchar **genesis_proxy_get_entry_names_by_category(GenesisProxy *proxy, gchar *category);
gchar *genesis_proxy_get_app_icon(GenesisProxy *proxy, gchar *name);
gchar *genesis_proxy_get_app_exec(GenesisProxy *proxy, gchar *name);
gchar **genesis_proxy_get_app_category_names(GenesisProxy *proxy, gchar *name);
gboolean genesis_proxy_get_app_showup(GenesisProxy *proxy, gchar *name);


typedef gboolean (*GenesisDbusSignalSCallback) (GenesisProxy *proxy, const char *data);

// dbus signal related handling functions
gboolean genesis_proxy_connect_signal_s_callback
	(DBusGProxy *proxy, const char* data, gpointer userData);
void genesis_proxy_connect_signal_type_string
	(GenesisProxy *proxy, const gchar* signame, GenesisDbusSignalSCallback handler);

G_END_DECLS

#endif /* GENESIS_PROXY_H */
