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

G_END_DECLS


/* the following is net yet used*/
#ifdef NO_THIS_MACRO

#define DESKTOP_FILE_SUFFIX         ".desktop"
#define DESKTOP_DIR                 "/usr/share/applications/"

/**
 * GenesisCategory:
 * @applications: GList of GenesisAppEntry entries in this category
 * @name: Locally allocated gchar name of category.
 * @is_primary: Set to True if any applications list this as their primary
 *              category.
 */
typedef struct _GenesisCategory {
  /*< public >*/
  GList *applications;
  gchar *name;
  gboolean is_primary;
} GenesisCategory;

gboolean genesis_proxy_start_app_from_path (GenesisProxy *proxy, gchar *path);
gboolean genesis_proxy_start_app_from_name (GenesisProxy *proxy, gchar* name);
GenesisAppEntry *genesis_proxy_get_nth_entry (GenesisProxy *proxy, guint n);
GenesisAppEntry *genesis_proxy_get_entry_by_name (GenesisProxy *proxy, gchar* name);
void genesis_proxy_remove_entry (GenesisProxy *proxy, GenesisAppEntry *entry);
void genesis_proxy_add_entry (GenesisProxy *proxy, GenesisAppEntry *entry);
void genesis_proxy_add_entry_by_path (GenesisProxy *proxy, const gchar *path);
GList* genesis_proxy_get_categories (GenesisProxy *proxy);
GList* genesis_proxy_get_all_applications (GenesisProxy *proxy);
GList* genesis_proxy_get_applications_by_category (GenesisProxy *proxy, const gchar *name);
GList* genesis_proxy_get_all_categories (GenesisProxy *proxy);

#endif

#endif /* GENESIS_PROXY_H */
