/*
 * Copyright (C) 2008 Intel Corporation
 *
 * Author:  Raymond Liu <raymond.li@intel.com>
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

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h> /* exit, EXIT_FAILURE */
#include "genesis-dbus-common.h"
#include "genesis-utils.h"

#include "genesis-daemon.h"
#include "genesis-daemon-dbus.h"

/* Utility macro to define the value_object GType structure. */
G_DEFINE_TYPE( GenesisDbusObj, genesis_dbusobj, G_TYPE_OBJECT)

gboolean genesis_dbusobj_hello
	(GenesisDbusObj* obj, char * who, GError** error);

gboolean genesis_dbusobj_start_app_by_name
	(GenesisDbusObj* obj, char * name, GError** error);

gboolean genesis_dbusobj_get_nth_entry_name
	(GenesisDbusObj* obj, gint index, char ** name, GError** error);

gboolean genesis_dbusobj_get_category_names
	(GenesisDbusObj* obj, char *** names, GError** error);

gboolean genesis_dbusobj_get_entry_names_by_category
	(GenesisDbusObj* obj, char *category, char *** names, GError** error);

gboolean genesis_dbusobj_get_app_icon
	(GenesisDbusObj* obj, char *name, char ** icon, GError** error);

gboolean genesis_dbusobj_get_app_exec
	(GenesisDbusObj* obj, char *name, char ** exec, GError** error);


gboolean genesis_dbusobj_get_app_showup
	(GenesisDbusObj* obj, char *name, gboolean* showup, GError** error);

gboolean genesis_dbusobj_get_app_category_names
	(GenesisDbusObj* obj, char *name, char *** names, GError** error);

#include "genesis-daemon-dbus-glue.h"


static void genesis_dbusobj_init(GenesisDbusObj* obj) {
  save_log("Called\n");

  g_assert(obj != NULL);

}

/**
 * Per class initializer
 *
 * Registers the type into the GLib/D-Bus wrapper so that it may add
 * its own magic.
 */
static void genesis_dbusobj_class_init(GenesisDbusObjClass* klass) {

  save_log("Called\n");

  g_assert(klass != NULL);

  save_log("Binding to GLib/D-Bus\n");

  /* Time to bind this GType into the GLib/D-Bus wrappers.
     NOTE: This is not yet "publishing" the object on the D-Bus, but
           since it is only allowed to do this once per class
           creation, the safest place to put it is in the class
           initializer.
           Specifically, this function adds "method introspection
           data" to the class so that methods can be called over
           the D-Bus. */
  dbus_g_object_type_install_info(GENESIS_TYPE_DBUSOBJ,
                                 &dbus_glib_genesis_dbusobj_object_info);

  save_log("Done\n");
  /* All done. Class is ready to be used for instantiating objects */
}

gboolean genesis_dbusobj_hello(GenesisDbusObj* obj, char * who, GError** error)
{

	g_return_val_if_fail(GENESIS_IS_DBUSOBJ(obj), FALSE);
	obj = GENESIS_DBUSOBJ(obj);
	
	save_log("hello %s", who);

	return TRUE;

}

gboolean genesis_dbusobj_start_app_by_name
	(GenesisDbusObj* obj, char * name, GError** error)
{
	GenesisController* controller;
	
	g_return_val_if_fail(GENESIS_IS_DBUSOBJ(obj), FALSE);
	obj = GENESIS_DBUSOBJ(obj);

	controller = obj->genesis_daemon->controller;

	return genesis_controller_start_app_from_name(controller, name);

}


gboolean genesis_dbusobj_get_nth_entry_name
	(GenesisDbusObj* obj, gint index, char ** name, GError** error)
{
	GenesisController* controller;
	GenesisAppEntry* entry;
	
	g_return_val_if_fail(GENESIS_IS_DBUSOBJ(obj), FALSE);
	obj = GENESIS_DBUSOBJ(obj);
	
	controller = obj->genesis_daemon->controller;
	
	entry = genesis_controller_get_nth_entry(controller, index);
	if ( entry ){
		*name = g_strdup( genesis_app_entry_get_name(entry) );
		return TRUE;
	}else{
	//FIXME: need to find out the suitable error domain and code
		g_set_error(error,1,1,"out of entry index");
		return FALSE;
	}

}

gboolean genesis_dbusobj_get_category_names
	(GenesisDbusObj* obj, char *** names, GError** error)
{
	GenesisController* controller;
	
	g_return_val_if_fail(GENESIS_IS_DBUSOBJ(obj), FALSE);
	obj = GENESIS_DBUSOBJ(obj);
	
	controller = obj->genesis_daemon->controller;
	
	*names =  genesis_controller_get_category_names(controller);

	return TRUE;

}

gboolean genesis_dbusobj_get_entry_names_by_category
	(GenesisDbusObj* obj, char *category, char *** names, GError** error)
{
	GenesisController* controller;
	
	g_return_val_if_fail(GENESIS_IS_DBUSOBJ(obj), FALSE);
	obj = GENESIS_DBUSOBJ(obj);
	
	controller = obj->genesis_daemon->controller;
	
	*names =  genesis_controller_get_entry_names_by_category(controller,category);

	return TRUE;
}

gboolean genesis_dbusobj_get_app_icon
	(GenesisDbusObj* obj, char *name, char ** icon, GError** error)
{
	GenesisController* controller;
	GenesisAppEntry* entry;
	
	g_return_val_if_fail(GENESIS_IS_DBUSOBJ(obj), FALSE);
	obj = GENESIS_DBUSOBJ(obj);
	
	controller = obj->genesis_daemon->controller;
	
	entry = genesis_controller_get_entry_by_name(controller, name);
	if ( entry ){
		*icon = g_strdup( genesis_app_entry_get_icon(entry) );
		return TRUE;
	}else{
	//FIXME: need to find out the suitable error domain and code
		g_set_error(error,1,1,"can not find %s",name);
		return FALSE;
	}
}

gboolean genesis_dbusobj_get_app_exec
	(GenesisDbusObj* obj, char *name, char ** exec, GError** error)
{
	GenesisController* controller;
	GenesisAppEntry* entry;
	
	g_return_val_if_fail(GENESIS_IS_DBUSOBJ(obj), FALSE);
	obj = GENESIS_DBUSOBJ(obj);
	
	controller = obj->genesis_daemon->controller;
	
	entry = genesis_controller_get_entry_by_name(controller, name);
	if ( entry ){
		*exec = g_strdup( genesis_app_entry_get_exec(entry) );
		return TRUE;
	}else{
	//FIXME: need to find out the suitable error domain and code
		g_set_error(error,1,1,"can not find %s",name);
		return FALSE;
	}
}


gboolean genesis_dbusobj_get_app_showup
	(GenesisDbusObj* obj, char *name, gboolean* showup, GError** error)
{
	GenesisController* controller;
	GenesisAppEntry* entry;
	
	g_return_val_if_fail(GENESIS_IS_DBUSOBJ(obj), FALSE);
	obj = GENESIS_DBUSOBJ(obj);
	
	controller = obj->genesis_daemon->controller;
	
	entry = genesis_controller_get_entry_by_name(controller, name);
	if ( entry ){
		*showup = genesis_app_entry_is_showup(entry);
		return TRUE;
	}else{
	//FIXME: need to find out the suitable error domain and code
		g_set_error(error,1,1,"can not find %s",name);
		return FALSE;
	}
}

gboolean genesis_dbusobj_get_app_category_names
	(GenesisDbusObj* obj, char *name, char *** names, GError** error)
{
	GenesisController* controller;
	GenesisAppEntry* entry;
	
	g_return_val_if_fail(GENESIS_IS_DBUSOBJ(obj), FALSE);
	obj = GENESIS_DBUSOBJ(obj);
	
	controller = obj->genesis_daemon->controller;
	
	entry = genesis_controller_get_entry_by_name(controller, name);
	if ( entry ){
		*names = genesis_app_entry_get_category_names(entry);
		return TRUE;
	}else{
	//FIXME: need to find out the suitable error domain and code
		g_set_error(error,1,1,"can not find %s",name);
		return FALSE;
	}
}


static void handleError(const char* msg, const char* reason, gboolean fatal)
{
  save_log("ERROR: %s (%s)\n", msg, reason);
  if (fatal) {
    exit(EXIT_FAILURE);
  }
}

GenesisDbusObj *genesis_dbus_daemon_init(GenesisDaemon *daemon)
{

  DBusGConnection* bus = NULL;
  DBusGProxy* busProxy = NULL;

  GenesisDbusObj* dbusobj = NULL;

  guint result;
  
  GError* error = NULL;

  bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
  if (error != NULL) {
    save_log("Couldn't connect to session bus\n");
    exit(-1);
  }

  save_log("Registering the well-known name (%s)\n", GENESIS_DBUSOBJ_SERVICE_NAME);

  busProxy = dbus_g_proxy_new_for_name(bus,
                                       DBUS_SERVICE_DBUS,
                                       DBUS_PATH_DBUS,
                                       DBUS_INTERFACE_DBUS);
  if (busProxy == NULL) {
    save_log("Failed to get a proxy for D-Bus\n");
  }

  /* Attempt to register the well-known name.*/
  if (!dbus_g_proxy_call(busProxy,
                         "RequestName",
                         &error,
                         G_TYPE_STRING,
                         GENESIS_DBUSOBJ_SERVICE_NAME,
                         G_TYPE_UINT,
                         0,
                         G_TYPE_INVALID,
                         G_TYPE_UINT,
                         &result,
                         G_TYPE_INVALID)) {
    handleError("D-Bus.RequestName RPC failed", error->message,
                                                TRUE);
  }

  save_log("RequestName returned %d.\n", result);
  if (result != 1) {
    handleError("Failed to get the primary well-known name.",
                "RequestName result != 1", TRUE);
  }

  save_log("Creating one dbus object.\n");
  
  dbusobj = g_object_new(GENESIS_TYPE_DBUSOBJ, NULL);
  
  if (dbusobj == NULL) {
    handleError("Failed to create dbus_obj.",
                "Unknown(OOM?)", TRUE);
  }

  //dbus_g_connection_register_g_object(bus, GENESIS_DBUSOBJ_SERVICE_OBJECT_PATH, G_OBJECT(GenesisDbusObj));

    dbus_g_connection_register_g_object(bus,
                                      GENESIS_DBUSOBJ_SERVICE_OBJECT_PATH,
                                      G_OBJECT(dbusobj));

  save_log("dbusobj register to DBUS done\n");
  
  dbusobj->genesis_daemon = daemon;

  return dbusobj;
}


