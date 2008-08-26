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

#define DESKTOP_FILE_SUFFIX         ".desktop"
#define DESKTOP_DIR                 "/usr/share/applications"

#define APPLICATIONS_MENU           "/etc/xdg/menus/applications.menu"
#define PREFERENCES_MENU            "/etc/xdg/menus/preferences.menu"

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
//  GenesisFSMonitor  *monitor;
};
#if 0
static gboolean applications_list_updated (GenesisFSMonitor *monitor, const gchar *path, 
                                           GenesisFSMonitorEventType type, gpointer data)
{
  GenesisController *self = GENESIS_CONTROLLER (data);
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (self);
  GenesisAppEntry *entry = NULL;
  gchar *desktop_entry = NULL;
  gchar *nth_desktop_entry = NULL;
  guint n = 0;

  if (!g_str_has_suffix (path, DESKTOP_FILE_SUFFIX))
    return FALSE;

  desktop_entry = g_strdup (path);
  switch (type)
  {
    case EVENT_MODIFIED:
    case EVENT_REMOVED:
      do
      {
        entry = genesis_controller_get_nth_entry (self, n);
        g_object_get (G_OBJECT(entry), "desktop_entry", &nth_desktop_entry, NULL);

        if (!g_ascii_strcasecmp (nth_desktop_entry, desktop_entry))
        {
          genesis_controller_remove_entry (self, entry);
          g_free (nth_desktop_entry);

          if (EVENT_REMOVED == type)
          {
            g_signal_emit_by_name (self, "app-entry-updated", type, desktop_entry);
            goto Exit;
          }
          else if (EVENT_MODIFIED == type)
            break;
        }

        n++;
      }while (entry);

      if (EVENT_REMOVED == type)
        break;
    case EVENT_CREATED:
      entry = g_object_new (GENESIS_TYPE_APP_ENTRY, "desktop_entry", desktop_entry, NULL);

      /* Extract the information for this entry from its desktop file and
       * populate the GenesisAppEntry fields, as well as adding to the
       * GenesisCategory hash table. */
      if (genesis_app_entry_extract_info(entry, priv->categories))
        priv->applications = g_list_append (priv->applications, entry);

      g_signal_emit_by_name (self, "app-entry-updated", type, desktop_entry);
      break;
    default:
      break;
  }

Exit:
  if (desktop_entry)
    g_free (desktop_entry);

  return TRUE;
}
#endif
static GHashTable *genesis_controller_parse_menu (GenesisController *self, xmlNode *list)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (self);
  xmlNode *node, *iter = NULL;
  gchar *name = NULL, *category = NULL;

  for (node = list->children; node != NULL; node = node->next )
  {
    if (!strcmp ((gchar *)node->name, "Menu"))
      priv->categories = genesis_controller_parse_menu (self, node);
    else if (!strcmp ((gchar *)node->name, "Name"))
      name = g_strdup ((gchar *)node->last->content);
    else if (!strcmp ((gchar *)node->name, "Include"))
    {
      for (iter = ((node->children)->next)->children; iter != NULL; iter = iter->next)
      {
        if (strcmp ((gchar *)iter->name, "Category"))
          continue;
        category = g_strdup ((gchar *)iter->last->content);
      }
    }
  }

  if (name && category)
    g_hash_table_replace (priv->categories, category, name);

  return priv->categories;
}

#if 0
static GHashTable *genesis_controller_append_categories (GenesisController *self, const gchar *menu)
{
  xmlDoc *doc = NULL;
  xmlNode *root = NULL;

  doc = xmlParseFile (menu);
  root = xmlDocGetRootElement (doc);

  if (NULL == root)
    return NULL;

  return genesis_controller_parse_menu (self, root);
}
#endif

static GList *genesis_controller_append_applications (GenesisController *self)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (self);
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
/*
  priv->categories = genesis_controller_append_categories (self, APPLICATIONS_MENU);
  priv->categories = genesis_controller_append_categories (self, PREFERENCES_MENU);
*/
  priv->applications = genesis_controller_append_applications (self);
#if 0
  priv->monitor = genesis_fs_monitor_get_singleton ();
  genesis_fs_monitor_add (priv->monitor, DESKTOP_DIR, IN_ALL_EVENTS, applications_list_updated, self);
#endif
}

/* Public Functions */

GenesisController *genesis_controller_get_singleton (void)
{
  static GenesisController *controller = NULL;

  if (!controller)
    controller = g_object_new (GENESIS_TYPE_CONTROLLER, NULL);

  return controller;
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
