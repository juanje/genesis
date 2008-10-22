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
#include "genesis-proxy.h"

void print_entry_names_by_category(GenesisProxy *proxy, gchar *category);
void print_entry_info_by_name(GenesisProxy *proxy, gchar *name);


void print_entry_names_by_category(GenesisProxy *proxy, gchar *category)
{
	gchar **entry_names = NULL;
	gchar **entry_names_p;
	
	entry_names = genesis_proxy_get_entry_names_by_category(proxy, category);
	if (entry_names){
		entry_names_p = entry_names;
		while(*entry_names_p){
			g_print("%s\n", *entry_names_p);
			entry_names_p++;
		}
		g_strfreev(entry_names);
	}
}

void print_entry_info_by_name(GenesisProxy *proxy, gchar *name)
{
	gchar *icon, *exec;
	gboolean showup;
	gchar **category_names;
	gchar **category_names_p;
	
	if (!name)
		return;

	icon = genesis_proxy_get_app_icon(proxy, name);
	exec = genesis_proxy_get_app_exec(proxy, name);
	showup = genesis_proxy_get_app_showup(proxy, name);

	g_print("icon : %s \nexec : %s \nshowup : %d \n",
		icon,	 exec, showup
	);

	g_free(icon);
	g_free(exec);

	category_names = genesis_proxy_get_app_category_names(proxy, name);
	if (category_names){
		category_names_p = category_names;
		g_print("Categories: [\n");
		while(*category_names_p){
			g_print("\t%s\n", *category_names_p);
			category_names_p++;
		}
		g_print("]\n");
		g_strfreev(category_names);
	}
}

int main(int argc, char *argv[])
{
	GenesisProxy *proxy = NULL;
//	GMainLoop* mainloop;
//	GError* error = NULL;
	guint index = 0;
	gchar *name = NULL;
	gchar **category_names = NULL;
	gchar **category_names_p;
	
	g_type_init ();

	proxy = genesis_proxy_get_singleton();

	if (!proxy)
	{
		g_error ("Failed to get genesis proxy singleton.\n");
	}

	g_print("say helleo to genesis_daemon with our name\n");
	genesis_proxy_hello(proxy,"Daemon-dbus-test");

	category_names = genesis_proxy_get_category_names(proxy);
	if (category_names){
		category_names_p = category_names;
		g_print(" \n------ Category Lists : ------\n");
		while(*category_names_p){
			g_print("\n ---- %s : ----\n", *category_names_p);
			print_entry_names_by_category(proxy, *category_names_p);
			category_names_p++;
		}
		g_print("\n ------ End of Lists ------\n");
		g_strfreev(category_names);
	}

	g_print(" \n--- Entry Lists : ---\n");
	do{
		name = genesis_proxy_get_nth_entry_name (proxy, index);
		if (name != NULL){
			g_print("entry %d , name= %s\n", index, name);
			index++;
			print_entry_info_by_name(proxy,name);
			g_free(name);
		}else{
			g_print("--- End of list ---\n");
		}
	}while(name);

	g_print("now, start a terminal by name\n");
	genesis_proxy_start_app_by_name(proxy, "terminal");

#if 0
	mainloop = g_main_loop_new(NULL, FALSE);

	if (mainloop == NULL) {
	 	g_error("Failed to create the mainloop\n");
	}
	
	g_main_loop_run(mainloop);
#endif

  return 0;
}
