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

#include "genesis-dbus-proxy-glue.h"

#include "genesis-proxy.h"


#define GENESIS_PROXY_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), GENESIS_TYPE_PROXY, GenesisProxyPrivate))

G_DEFINE_TYPE (GenesisProxy, genesis_proxy, G_TYPE_OBJECT);

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

gboolean genesis_proxy_connect_signal_s_callback
		(DBusGProxy* proxy, const char* data, gpointer userData)
{
	GenesisDbusSignalSCallback real_cb_func = ( GenesisDbusSignalSCallback )userData;
	return real_cb_func(genesis_proxy_get_singleton(), data);
}


void genesis_proxy_connect_signal_type_string
	(GenesisProxy *proxy, const gchar* signame, GenesisDbusSignalSCallback handler)
{

	GenesisProxyPrivate *priv;
	
	g_return_if_fail(IS_GENESIS_PROXY(proxy));
	proxy = GENESIS_PROXY(proxy);

	priv = GENESIS_PROXY_GET_PRIVATE (proxy);
	
	dbus_g_proxy_add_signal(priv->dbusproxy,
				signame,
				G_TYPE_STRING,
				G_TYPE_INVALID);

	dbus_g_proxy_connect_signal(priv->dbusproxy,
				signame,
				/* the common callback function for 1 type String */
				G_CALLBACK (genesis_proxy_connect_signal_s_callback),
				/* handler as user data for using by common callback function */
				handler,	
				NULL);

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

gboolean genesis_proxy_start_app_by_name (GenesisProxy *proxy, 
						 gchar* name)
{
	GenesisProxyPrivate *priv;
	GError* error = NULL;
	
	g_return_val_if_fail(IS_GENESIS_PROXY(proxy), FALSE);
	proxy = GENESIS_PROXY(proxy);

	priv = GENESIS_PROXY_GET_PRIVATE (proxy);

	org_moblin_genesis_start_app_by_name(priv->dbusproxy, name, &error);
	if (error != NULL) {
		g_warning("%s", error->message);
		return FALSE;
	}

	return TRUE;
}


gchar *genesis_proxy_get_nth_entry_name (GenesisProxy *proxy, 
			gint index)
{
	GenesisProxyPrivate *priv;
	GError* error = NULL;
	gchar* name = NULL;
	
	g_return_val_if_fail(IS_GENESIS_PROXY(proxy), FALSE);
	proxy = GENESIS_PROXY(proxy);

	priv = GENESIS_PROXY_GET_PRIVATE (proxy);

	org_moblin_genesis_get_nth_entry_name(priv->dbusproxy, index, &name, &error);
	if (error != NULL) {
		g_warning("%s", error->message);
		return NULL;
	}

	return name;
}


/* the return gchar** should be freed with g_strfreev by the caller */
gchar **genesis_proxy_get_category_names (GenesisProxy *proxy)
{
	GenesisProxyPrivate *priv;
	GError* error = NULL;
	gchar** names = NULL;
	
	g_return_val_if_fail(IS_GENESIS_PROXY(proxy), FALSE);
	proxy = GENESIS_PROXY(proxy);

	priv = GENESIS_PROXY_GET_PRIVATE (proxy);

	org_moblin_genesis_get_category_names(priv->dbusproxy, &names, &error);
	if (error != NULL) {
		g_warning("%s", error->message);
		return NULL;
	}

	return names;
}

/* the return gchar** should be freed with g_strfreev by the caller */
gchar **genesis_proxy_get_entry_names_by_category
		(GenesisProxy *proxy, gchar *category)
{
	GenesisProxyPrivate *priv;
	GError* error = NULL;
	gchar** names = NULL;
	
	g_return_val_if_fail(IS_GENESIS_PROXY(proxy), FALSE);
	proxy = GENESIS_PROXY(proxy);

	priv = GENESIS_PROXY_GET_PRIVATE (proxy);

	org_moblin_genesis_get_entry_names_by_category(priv->dbusproxy, category, &names, &error);
	if (error != NULL) {
		g_warning("%s", error->message);
		return NULL;
	}

	return names;
}

gchar *genesis_proxy_get_app_icon
		(GenesisProxy *proxy, gchar *name)
{
	GenesisProxyPrivate *priv;
	GError* error = NULL;
	gchar* icon = NULL;
	
	g_return_val_if_fail(IS_GENESIS_PROXY(proxy), FALSE);
	proxy = GENESIS_PROXY(proxy);

	priv = GENESIS_PROXY_GET_PRIVATE (proxy);

	org_moblin_genesis_get_app_icon(priv->dbusproxy, name, &icon, &error);
	if (error != NULL) {
		g_warning("%s", error->message);
		return NULL;
	}

	return icon;
}

gchar *genesis_proxy_get_app_exec
		(GenesisProxy *proxy, gchar *name)
{
	GenesisProxyPrivate *priv;
	GError* error = NULL;
	gchar* exec = NULL;
	
	g_return_val_if_fail(IS_GENESIS_PROXY(proxy), FALSE);
	proxy = GENESIS_PROXY(proxy);

	priv = GENESIS_PROXY_GET_PRIVATE (proxy);

	org_moblin_genesis_get_app_exec(priv->dbusproxy, name, &exec, &error);
	if (error != NULL) {
		g_warning("%s", error->message);
		return NULL;
	}

	return exec;
}

gchar **genesis_proxy_get_app_category_names
		(GenesisProxy *proxy, gchar *name)
{
	GenesisProxyPrivate *priv;
	GError* error = NULL;
	gchar** cat_names = NULL;
	
	g_return_val_if_fail(IS_GENESIS_PROXY(proxy), FALSE);
	proxy = GENESIS_PROXY(proxy);

	priv = GENESIS_PROXY_GET_PRIVATE (proxy);

	org_moblin_genesis_get_app_category_names(priv->dbusproxy, name, &cat_names, &error);
	if (error != NULL) {
		g_warning("%s", error->message);
		return NULL;
	}

	return cat_names;
}

gboolean genesis_proxy_get_app_showup
		(GenesisProxy *proxy, gchar *name)
{
	GenesisProxyPrivate *priv;
	GError* error = NULL;
	gboolean showup = FALSE;
	
	g_return_val_if_fail(IS_GENESIS_PROXY(proxy), FALSE);
	proxy = GENESIS_PROXY(proxy);

	priv = GENESIS_PROXY_GET_PRIVATE (proxy);

	org_moblin_genesis_get_app_showup(priv->dbusproxy, name, &showup, &error);
	if (error != NULL) {
		g_warning("%s", error->message);
		return FALSE;
	}

	return showup;
}


