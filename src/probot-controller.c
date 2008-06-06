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

#include "probot-common.h"

#define DESKTOP_FILE_SUFFIX         ".desktop"
#define DESKTOP_DIR                 "/usr/share/applications"

#define APPLICATIONS_MENU           "/etc/xdg/menus/applications.menu"
#define PREFERENCES_MENU            "/etc/xdg/menus/preferences.menu"

#define PROBOT_CONTROLLER_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), PROBOT_TYPE_CONTROLLER, ProbotControllerPrivate))

G_DEFINE_TYPE (ProbotController, probot_controller, G_TYPE_OBJECT)

enum
{
  APPLICATIONS_LIST_CHANGED,
  N_SIGNALS
};

struct _ProbotControllerPrivate
{
  GList *applications;
  GHashTable *categories;
  GConfClient *client;
  GnomeVFSMonitorHandle  *monitor;
};

static void applications_list_updated (GnomeVFSMonitorHandle *handle, const gchar *monitor_uri,
                                       const gchar *info_uri, GnomeVFSMonitorEventType event_type,
                                       ProbotController *self);

static GHashTable *probot_controller_parse_menu (ProbotController *self, xmlNode *list)
{
  ProbotControllerPrivate *priv = PROBOT_CONTROLLER_GET_PRIVATE (self);
  xmlNode *node, *iter = NULL;
  gchar *name = NULL, *category = NULL;

  for (node = list->children; node != NULL; node = node->next )
  {
    if (!strcmp ((gchar *)node->name, "Menu"))
      priv->categories = probot_controller_parse_menu (self, node);
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

static GHashTable *probot_controller_append_categories (ProbotController *self, const gchar *menu)
{
  xmlDoc *doc = NULL;
  xmlNode *root = NULL;

  doc = xmlParseFile (menu);
  root = xmlDocGetRootElement (doc);

  if (NULL == root)
    return NULL;

  return probot_controller_parse_menu (self, root);
}

static GList *probot_controller_append_applications (ProbotController *self)
{
  ProbotControllerPrivate *priv = PROBOT_CONTROLLER_GET_PRIVATE (self);
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
      ProbotAppEntry *entry = g_object_new (PROBOT_TYPE_APP_ENTRY, 
                                            "desktop_entry", 
                                            desktop_path, NULL);
      if (probot_app_entry_extract_info (entry, priv->categories))
        priv->applications = g_list_append (priv->applications, entry);
    }
  }

  closedir (dir_handle);
  g_free (d);

  return priv->applications;
}

static void probot_controller_finalize (GObject *object)
{
  ProbotController *self = PROBOT_CONTROLLER (object);
  ProbotControllerPrivate *priv = PROBOT_CONTROLLER_GET_PRIVATE (self);

  if (priv->monitor)
    gnome_vfs_monitor_cancel (priv->monitor);

  if (priv->categories)
    g_free (priv->categories);

  if (priv->applications)
    g_free (priv->applications);
}

static void probot_controller_class_init (ProbotControllerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ProbotControllerPrivate));

  object_class->finalize = probot_controller_finalize;

  g_signal_new ("app-entry-updated",
                G_OBJECT_CLASS_TYPE(object_class),
                G_SIGNAL_RUN_LAST, 0,
                NULL, NULL,
                g_cclosure_user_marshal_VOID__UINT_POINTER,
                G_TYPE_NONE,
                2,
                G_TYPE_UINT, G_TYPE_POINTER);
}

static void probot_controller_init (ProbotController *self)
{
  ProbotControllerPrivate *priv = PROBOT_CONTROLLER_GET_PRIVATE (self);

  gnome_vfs_init ();

  priv->client = gconf_client_get_default ();
  priv->categories = g_hash_table_new (g_str_hash, g_str_equal); 
  priv->categories = probot_controller_append_categories (self, APPLICATIONS_MENU);
  priv->categories = probot_controller_append_categories (self, PREFERENCES_MENU);

  priv->applications = probot_controller_append_applications (self);

  gnome_vfs_monitor_add  (&priv->monitor,
                          DESKTOP_DIR,
                          GNOME_VFS_MONITOR_DIRECTORY,
                          (GnomeVFSMonitorCallback) applications_list_updated,
                          self);
}

static void applications_list_updated (GnomeVFSMonitorHandle *handle, const gchar *monitor_uri,
                                       const gchar *info_uri, GnomeVFSMonitorEventType event_type,
                                       ProbotController *self)
{
  ProbotControllerPrivate *priv = PROBOT_CONTROLLER_GET_PRIVATE (self);
  ProbotAppEntry *entry = NULL;
  gchar *desktop_entry = NULL;
  gchar *nth_desktop_entry = NULL;
  guint n = 0;

  if (!g_str_has_suffix (info_uri, DESKTOP_FILE_SUFFIX))
    return;

  desktop_entry = uri_to_path (info_uri);
  switch (event_type)
  {
    case GNOME_VFS_MONITOR_EVENT_CHANGED:
    case GNOME_VFS_MONITOR_EVENT_DELETED:
      do
      {
        entry = probot_controller_get_nth_entry (self, n);
        g_object_get (G_OBJECT(entry), "desktop_entry", &nth_desktop_entry, NULL);

        if (!g_ascii_strcasecmp (nth_desktop_entry, desktop_entry))
        {
          probot_controller_remove_entry (self, entry);
          g_free (nth_desktop_entry);

          if (GNOME_VFS_MONITOR_EVENT_DELETED == event_type)
          {
            g_signal_emit_by_name (self, "app-entry-updated", event_type, desktop_entry);
            goto Exit;
          }
          else if (GNOME_VFS_MONITOR_EVENT_CHANGED == event_type)
            break;
        }

        n++;
      }while (entry);

      if (GNOME_VFS_MONITOR_EVENT_DELETED == event_type)
        break;
    case GNOME_VFS_MONITOR_EVENT_CREATED:
      entry = g_object_new (PROBOT_TYPE_APP_ENTRY, "desktop_entry", desktop_entry, NULL);

      if (probot_app_entry_extract_info (entry, priv->categories))
        priv->applications = g_list_append (priv->applications, entry);
 
      g_signal_emit_by_name (self, "app-entry-updated", event_type, desktop_entry);
      break;
    default:
      break;
  }

Exit:
  if (desktop_entry)
    g_free (desktop_entry);
}

/* Public Functions */

ProbotController *probot_controller_get_singleton (void)
{
  static ProbotController *controller = NULL;

  if (!controller)
    controller = g_object_new (PROBOT_TYPE_CONTROLLER, NULL);

  return controller;
}

gboolean probot_controller_start_app_from_path (ProbotController *controller, gchar *path)
{
  ProbotControllerPrivate *priv = PROBOT_CONTROLLER_GET_PRIVATE (controller);
  ProbotAppEntry *entry = g_object_new (PROBOT_TYPE_APP_ENTRY, NULL);

  probot_app_entry_set_exec (entry, path);
  probot_app_entry_set_name (entry, path);

  priv->applications = g_list_append (priv->applications, entry);

  probot_app_entry_start (entry);
  return TRUE;
}

gboolean probot_controller_start_app_from_name (ProbotController *controller, gchar* name)
{
  ProbotControllerPrivate *priv = PROBOT_CONTROLLER_GET_PRIVATE (controller);
  ProbotAppEntry *entry = NULL;

  guint len = g_list_length (priv->applications);
  gboolean foundApp = FALSE;

  for (int i = 0; i < len; i++) 
  {
    entry = PROBOT_APP_ENTRY (g_list_nth_data (priv->applications, i));

    // compare name passed in with app name in list
    if (!g_ascii_strcasecmp (probot_app_entry_get_name (entry), name)) 
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

  probot_app_entry_start (entry);
  return TRUE;
}

ProbotAppEntry *probot_controller_get_nth_entry (ProbotController *controller, guint n)
{
  ProbotControllerPrivate *priv = PROBOT_CONTROLLER_GET_PRIVATE (controller);
  ProbotAppEntry *entry = NULL;  

  if (priv->applications)
    entry = PROBOT_APP_ENTRY (g_list_nth_data (priv->applications, n));

  return entry;
}

ProbotAppEntry *probot_controller_get_entry_by_name (ProbotController *controller, gchar* name)
{
  ProbotControllerPrivate *priv = PROBOT_CONTROLLER_GET_PRIVATE (controller);
  ProbotAppEntry *entry = NULL;

  guint len = g_list_length (priv->applications);

  for (int i = 0; i < len; i++)
  {
    entry = PROBOT_APP_ENTRY (g_list_nth_data (priv->applications, i));

    // compare name passed in with app name in list
    if (!g_ascii_strcasecmp (probot_app_entry_get_name (entry), name))
      return entry;
  }

  return entry;
}

void probot_controller_remove_entry (ProbotController *controller, ProbotAppEntry *entry)
{
  ProbotControllerPrivate *priv = PROBOT_CONTROLLER_GET_PRIVATE (controller);

  if (priv->applications)
    priv->applications = g_list_remove (priv->applications, entry);
}
