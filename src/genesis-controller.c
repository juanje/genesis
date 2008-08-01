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

struct _GenesisControllerPrivate
{
  GList *applications;
  GHashTable *categories;
  GConfClient *client;
  GenesisFSMonitor  *monitor;
};

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

      if (genesis_app_entry_extract_info (entry, priv->categories))
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

static GList *genesis_controller_append_applications (GenesisController *self)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (self);
  DIR *dir_handle = NULL;
  struct dirent *d = NULL;
  struct stat buf;
  gchar *desktop_path = NULL;

  if ((dir_handle = opendir(DESKTOP_DIR)) == NULL)
    g_warning ("Error reading file '%s'\n", DESKTOP_DIR);

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
  g_free (d);

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

static void genesis_controller_init (GenesisController *self)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (self);

  priv->client = gconf_client_get_default ();
  priv->categories = g_hash_table_new (g_str_hash, g_str_equal); 
  priv->categories = genesis_controller_append_categories (self, APPLICATIONS_MENU);
  priv->categories = genesis_controller_append_categories (self, PREFERENCES_MENU);

  priv->applications = genesis_controller_append_applications (self);
  priv->monitor = genesis_fs_monitor_get_singleton ();
  genesis_fs_monitor_add (priv->monitor, DESKTOP_DIR, IN_ALL_EVENTS, applications_list_updated, self);
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

  priv->applications = g_list_append (priv->applications, entry);

  genesis_app_entry_start (entry);
  return TRUE;
}

gboolean genesis_controller_start_app_from_name (GenesisController *controller, gchar* name)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);
  GenesisAppEntry *entry = NULL;

  guint len = g_list_length (priv->applications);
  gboolean foundApp = FALSE;

  for (int i = 0; i < len; i++) 
  {
    entry = GENESIS_APP_ENTRY (g_list_nth_data (priv->applications, i));

    // compare name passed in with app name in list
    if (!g_ascii_strcasecmp (genesis_app_entry_get_name (entry), name)) 
    {
      foundApp = TRUE;
      break;
    }
  }

  if (!foundApp) 
  {
    g_warning ("Couldn't find app with name = %s\n", name);
    return FALSE;
  }

  genesis_app_entry_start (entry);
  return TRUE;
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
  GenesisAppEntry *entry = NULL;

  guint len = g_list_length (priv->applications);

  for (int i = 0; i < len; i++)
  {
    entry = GENESIS_APP_ENTRY (g_list_nth_data (priv->applications, i));

    // compare name passed in with app name in list
    if (!g_ascii_strcasecmp (genesis_app_entry_get_name (entry), name))
      return entry;
  }

  return entry;
}

void genesis_controller_remove_entry (GenesisController *controller, GenesisAppEntry *entry)
{
  GenesisControllerPrivate *priv = GENESIS_CONTROLLER_GET_PRIVATE (controller);

  if (priv->applications)
    priv->applications = g_list_remove (priv->applications, entry);
}
