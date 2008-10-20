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

#include "genesis-daemon-dbus.h"

/* Utility macro to define the value_object GType structure. */
G_DEFINE_TYPE( GenesisDbusObj, genesis_dbusobj, G_TYPE_OBJECT)


gboolean genesis_dbusobj_hello(GenesisDbusObj* obj, char * who, GError** error);
gboolean genesis_dbusobj_getname(GenesisDbusObj* obj, char ** name_out, GError** error);

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



gboolean genesis_dbusobj_getname(GenesisDbusObj* obj, char ** name_out, GError** error)
{

  save_log("Called \n");

  g_assert(obj != NULL);

  *name_out =  g_strdup("genesis");
  return TRUE;

}

gboolean genesis_dbusobj_hello(GenesisDbusObj* obj, char * who, GError** error)
{

  save_log("hello %s", who);

  g_assert(obj != NULL);

  return TRUE;

}


static void handleError(const char* msg, const char* reason, gboolean fatal)
{
  save_log("ERROR: %s (%s)\n", msg, reason);
  if (fatal) {
    exit(EXIT_FAILURE);
  }
}

GenesisDbusObj *genesis_dbus_daemon_init()
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

  return dbusobj;
}


