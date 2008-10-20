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

#include "genesis-common.h"

#include "genesis-dbus-common.h"
#include "genesis-dbus-proxy-glue.h"

#include "genesis-proxy.h"


#define GENESIS_PROXY_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), GENESIS_TYPE_PROXY, GenesisProxyPrivate))

G_DEFINE_TYPE (GenesisProxy, genesis_proxy, G_TYPE_OBJECT)

/**
 * _GenesisProxyPrivate:
 */
struct _GenesisProxyPrivate
{
	DBusGProxy* dbusproxy;
};

static void genesis_proxy_class_init (GenesisProxyClass *klass)
{
	g_type_class_add_private (klass, sizeof (GenesisProxyPrivate));
}

static void genesis_proxy_init (GenesisProxy *self)
{

	DBusGConnection* bus;
	DBusGProxy* dbusproxy;
	GError* error = NULL;

	GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (self);

 
	bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);

	if (error != NULL) {
		g_error(error->message);
	}

	dbusproxy = dbus_g_proxy_new_for_name(bus,
                              GENESIS_DBUSOBJ_SERVICE_NAME, /* name */
                              GENESIS_DBUSOBJ_SERVICE_OBJECT_PATH, /* obj path */
                              GENESIS_DBUSOBJ_SERVICE_INTERFACE /* interface */);
	if (dbusproxy == NULL) {
		g_error("could not create dbus_proxy for %s \n", GENESIS_DBUSOBJ_SERVICE_NAME);
	}

	priv->dbusproxy = dbusproxy;
}


/* Public Functions */

GenesisProxy *genesis_proxy_get_singleton (void)
{
	static GenesisProxy *proxy = NULL;
	if (!proxy)
		proxy = g_object_new (GENESIS_TYPE_PROXY, NULL);
	return proxy;
}


void genesis_proxy_hello (GenesisProxy *proxy, gchar * name)
{
	GenesisProxyPrivate *priv;
	GError* error = NULL;
	
	g_return_if_fail(IS_GENESIS_PROXY(proxy));
	proxy = GENESIS_PROXY(proxy);

	priv = GENESIS_PROXY_GET_PRIVATE (proxy);

	org_moblin_genesis_hello(priv->dbusproxy, name, &error);
	if (error != NULL) {
		g_error("%s, seems proxy not working right", error->message);
	}
	
}


/* ------------------ the following is net yet used, just collect here -----------------------------*/
#ifdef NO_THIS_MACRO


enum
{
	APPLICATIONS_LIST_CHANGED,
	N_SIGNALS
};

static void genesis_proxy_finalize (GObject *object)
{
	GenesisProxy *self = GENESIS_PROXY (object);
	GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (self);
}

static GList *genesis_proxy_append_applications (GenesisProxy *self)
{
  GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (self);
  DIR *dir_handle = NULL;
  struct dirent *d = NULL;
  struct stat buf;
  gchar *desktop_path = NULL;

  if ((dir_handle = opendir(DESKTOP_DIR)) == NULL) {
    g_warning ("Error reading file '%s'\n", DESKTOP_DIR);
    return priv->applications;
  }

  while ((d = readdir (dir_handle)) != NULL) {
    desktop_path = g_build_filename (DESKTOP_DIR, d->d_name, NULL);

    if ((stat (desktop_path, &buf) == 0) &&
        S_ISREG (buf.st_mode) &&
        g_str_has_suffix (desktop_path, DESKTOP_FILE_SUFFIX)) 
    {
      GenesisAppEntry *entry = g_object_new (GENESIS_TYPE_APP_ENTRY, 
                                             "desktop_entry", 
                                             desktop_path, NULL);
      if (genesis_app_entry_extract_info (entry, priv->categories))
        priv->applications = g_list_append (priv->applications, entry);
    }
  }

  closedir (dir_handle);

  return priv->applications;
}


static void genesis_category_destroy(GenesisCategory *category)
{
  GList *tmp = category->applications;
  g_free(category->name);
  while (tmp) {
    g_object_unref(tmp->data);
    tmp = tmp->next;
  }
  g_list_free(category->applications);
  g_free(category);
  
}




gboolean genesis_proxy_start_app_from_path (GenesisProxy *proxy, gchar *path)
{
  GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (proxy);
  GenesisAppEntry *entry = g_object_new (GENESIS_TYPE_APP_ENTRY, NULL);

  genesis_app_entry_set_exec (entry, path);
  genesis_app_entry_set_name (entry, path);

  priv->applications = g_list_append (priv->applications, G_OBJECT(entry));

  genesis_app_entry_start (entry);
  return TRUE;
}

gboolean genesis_proxy_start_app_from_name (GenesisProxy *proxy, 
						 gchar* name)
{
  GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (proxy);
  GList *tmp = priv->applications;

  while (tmp) {
    GenesisAppEntry *entry = GENESIS_APP_ENTRY(tmp->data);

    /* compare name passed in with app name in list */
    if (!g_ascii_strcasecmp (genesis_app_entry_get_name (entry), name)) 
    {
      genesis_app_entry_start(entry);
      return TRUE;
    }

    tmp = tmp->next;
  }

  g_warning ("Couldn't find app with name = %s\n", name);
  return FALSE;
}

GenesisAppEntry *genesis_proxy_get_nth_entry (GenesisProxy *proxy, guint n)
{
  GenesisProxy *proxy;
  GenesisProxyPrivate *priv;
  GenesisAppEntry *entry = NULL;  

  g_return_if_fail(IS_GENESIS_PROXY(proxy));
  proxy= GENESIS_PROXY(proxy);

  priv = GENESIS_PROXY_GET_PRIVATE (proxy);
  
  if (priv->applications)
    entry = GENESIS_APP_ENTRY (g_list_nth_data (priv->applications, n));

  return entry;
}

GenesisAppEntry *genesis_proxy_get_entry_by_name (GenesisProxy *proxy, gchar* name)
{
  GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (proxy);
  GList *tmp = priv->applications;

  while (tmp)
  {
    GenesisAppEntry *entry = GENESIS_APP_ENTRY (tmp->data);

    // compare name passed in with app name in list
    if (!g_ascii_strcasecmp (genesis_app_entry_get_name (entry), name))
      return entry;

    tmp = tmp->next;
  }

  return NULL;
}

void genesis_proxy_remove_entry (GenesisProxy *proxy, GenesisAppEntry *entry)
{
  GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (proxy);

  if (priv->applications)
    priv->applications = g_list_remove (priv->applications, entry);
}

void genesis_proxy_add_entry (GenesisProxy *proxy, GenesisAppEntry *entry)
{
  GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (proxy);

  if (genesis_app_entry_extract_info (entry, priv->categories))
    priv->applications = g_list_append (priv->applications, entry);
}

void genesis_proxy_add_entry_by_path (GenesisProxy *proxy, const gchar *path)
{
  GenesisAppEntry *entry = NULL;
  gchar *desktop_entry = NULL;

  desktop_entry = g_strdup (path);
  entry = g_object_new (GENESIS_TYPE_APP_ENTRY, "desktop_entry", desktop_entry, NULL);

  genesis_proxy_add_entry (proxy, entry);
}

GList* genesis_proxy_get_categories (GenesisProxy *proxy)
{
  GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (proxy);
  GList *categories;
  GList *tmp;
  GList *results = NULL;

  if (!priv->categories)
    return NULL;

  categories = g_hash_table_get_values (priv->categories);
  tmp = categories;

  while (tmp)
  {
    GenesisCategory *category = tmp->data;
    if (category->is_primary)
      results = g_list_append(results, category);
    tmp = tmp->next;
  }

  g_list_free(categories);

  return results;
}


GList* genesis_proxy_get_all_categories (GenesisProxy *proxy)
{
  GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (proxy);

  if (!priv->categories)
    return NULL;

  return g_hash_table_get_values (priv->categories);
}


GList* genesis_proxy_get_applications_by_category (GenesisProxy *proxy, const gchar *name)
{
  GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (proxy);
  GenesisCategory *category = g_hash_table_lookup (priv->categories, name);
  if (!category)
    return NULL;
  return g_list_copy(category->applications);
}

GList* genesis_proxy_get_all_applications (GenesisProxy *proxy)
{
  GenesisProxyPrivate *priv = GENESIS_PROXY_GET_PRIVATE (proxy);
  return g_list_copy(priv->applications);
}


#endif


