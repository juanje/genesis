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

#define GENESIS_CONTROLLER_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), GENESIS_TYPE_CONTROLLER, GenesisControllerPrivate))

G_DEFINE_TYPE (GenesisController, genesis_controller, G_TYPE_OBJECT)

enum
{
  APPLICATIONS_LIST_CHANGED,
  N_SIGNALS
};

/**
 * _GenesisControllerPrivate:
 * @categories: Hash table of GenesisCategory elements
 * @applications: GList of GenesisAppEntry elements
 */
struct _GenesisControllerPrivate
{
  GHashTable *categories;
  GList *applications;
};

static GList *genesis_controller_append_applications (GenesisController *self, gchar *path)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (self);
  DIR *dir_handle = NULL;
  struct dirent *d = NULL;
  struct stat buf;
  gchar *desktop_path = NULL;

  if ((dir_handle = opendir(path)) == NULL) {
    g_error ("Error reading file '%s'\n", path);
    return priv->applications;
  }

  while ((d = readdir (dir_handle)) != NULL) {
    desktop_path = g_build_filename (path, d->d_name, NULL);

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

static void genesis_controller_finalize (GObject *object)
{
  GenesisController *self = GENESIS_CONTROLLER (object);
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (self);

  if (priv->categories)
    g_free (priv->categories);

  if (priv->applications)
    g_free (priv->applications);
}

static void genesis_controller_class_init (GenesisControllerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GenesisControllerPrivate));

  object_class->finalize = genesis_controller_finalize;

  g_signal_new ("app-entry-updated",
                G_OBJECT_CLASS_TYPE(object_class),
                G_SIGNAL_RUN_LAST, 0,
                NULL, NULL,
                g_cclosure_user_marshal_VOID__UINT_POINTER,
                G_TYPE_NONE,
                2,
                G_TYPE_UINT, G_TYPE_POINTER);
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

static void genesis_controller_init (GenesisController *self)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (self);

  priv->categories = g_hash_table_new_full(
    g_str_hash, g_str_equal, NULL, (GDestroyNotify)genesis_category_destroy); 

}

/* Public Functions */

GenesisController *genesis_controller_get_singleton (void)
{
  static GenesisController *controller = NULL;
  if (!controller)
    controller = g_object_new (GENESIS_TYPE_CONTROLLER, NULL);
  return controller;
}

/* this function must be called after genesis_controller_get_singleton is been called for the first time */
void genesis_controller_init_application_lists (GenesisController *controller, gchar *path)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);

  priv->applications = genesis_controller_append_applications (controller, path);
}


gboolean genesis_controller_start_app_from_path (GenesisController *controller, gchar *path)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);
  GenesisAppEntry *entry = g_object_new (GENESIS_TYPE_APP_ENTRY, NULL);

  genesis_app_entry_set_exec (entry, path);
  genesis_app_entry_set_name (entry, path);

  priv->applications = g_list_append (priv->applications, G_OBJECT(entry));

  genesis_app_entry_start (entry);
  return TRUE;
}

gboolean genesis_controller_start_app_from_name (GenesisController *controller, 
						 gchar* name)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);
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

GenesisAppEntry *genesis_controller_get_nth_entry (GenesisController *controller, guint n)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);
  GenesisAppEntry *entry = NULL;  

  if (priv->applications)
    entry = GENESIS_APP_ENTRY (g_list_nth_data (priv->applications, n));

  return entry;
}

GenesisAppEntry *genesis_controller_get_entry_by_name (GenesisController *controller, gchar* name)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);
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

void genesis_controller_remove_entry (GenesisController *controller, GenesisAppEntry *entry)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);

  if (priv->applications)
    priv->applications = g_list_remove (priv->applications, entry);
}

void genesis_controller_add_entry (GenesisController *controller, GenesisAppEntry *entry)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);

  if (genesis_app_entry_extract_info (entry, priv->categories))
    priv->applications = g_list_append (priv->applications, entry);
}

void genesis_controller_add_entry_by_path (GenesisController *controller, const gchar *path)
{
  GenesisAppEntry *entry = NULL;
  gchar *desktop_entry = NULL;

  desktop_entry = g_strdup (path);
  entry = g_object_new (GENESIS_TYPE_APP_ENTRY, "desktop_entry", desktop_entry, NULL);

  genesis_controller_add_entry (controller, entry);
}

GList* genesis_controller_get_categories (GenesisController *controller)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);
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

gchar** genesis_controller_get_category_names (GenesisController *controller)
{
	GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);
	GList *categories;
	GList *tmp;
	gint length;
	gint index = 0;
	gchar **strs_names;

	if (!priv->categories)
		return NULL;

	categories = g_hash_table_get_values (priv->categories);
	length = g_list_length(categories);
	strs_names = g_new (gchar *, (length + 1));

	tmp = categories;

	while (tmp){
		GenesisCategory *category = tmp->data;
		if (category->is_primary)
		strs_names[index++] = g_strdup(category->name);
		tmp = tmp->next;
	}
	strs_names[index] = NULL;
	g_list_free(categories);

	return strs_names;
}


gchar** genesis_controller_get_entry_names_by_category 
			(GenesisController *controller, const gchar *category)
{
	GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);
	GenesisCategory *_category = g_hash_table_lookup (priv->categories, category);
	GenesisAppEntry *entry;
	GList *tmp;
	gint length;
	gint index = 0;
	gchar **strs_names;

	if ( (!_category) || (!_category->applications))
		return NULL;

	length = g_list_length(_category->applications);
	strs_names = g_new (gchar *, (length + 1));

	tmp = _category->applications;

	while (tmp){
		entry = tmp->data;
		strs_names[index++] = g_strdup( genesis_app_entry_get_name (entry) );
		tmp = tmp->next;
	}
	strs_names[index] = NULL;
	
	return strs_names;
}

GList* genesis_controller_get_all_categories (GenesisController *controller)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);

  if (!priv->categories)
    return NULL;

  return g_hash_table_get_values (priv->categories);
}

GList* genesis_controller_get_applications_by_category (GenesisController *controller, const gchar *name)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);
  GenesisCategory *category = g_hash_table_lookup (priv->categories, name);
  if (!category)
    return NULL;
  return g_list_copy(category->applications);
}

GList* genesis_controller_get_all_applications (GenesisController *controller)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);
  return g_list_copy(priv->applications);
}
