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

#define GENESIS_APP_ENTRY_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), GENESIS_TYPE_APP_ENTRY, GenesisAppEntryPrivate))

G_DEFINE_TYPE (GenesisAppEntry, genesis_app_entry, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_DESKTOP_ENTRY
};

struct _GenesisAppEntryPrivate
{
  gchar *filename;

  gchar *name;
  gchar *icon;
  gchar *exec;
  gchar *service;
  gchar *id;
  gboolean showup;
  GList *categories; /* Do not free gpointer elements */
  gchar *splash_screen;

  GtkWidget *splashscreen;
  WnckScreen *screen;

  GPid pid;
};

static void genesis_app_entry_get_property (GObject *object, guint prop_id,
                                            GValue *value, GParamSpec *pspec);

static void genesis_app_entry_set_property (GObject *object, guint prop_id,
                                            const GValue *value, GParamSpec *pspec);

static void genesis_app_entry_class_init (GenesisAppEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GenesisAppEntryPrivate));

  object_class->set_property = genesis_app_entry_set_property;
  object_class->get_property = genesis_app_entry_get_property;

  g_object_class_install_property (object_class,
                                   PROP_DESKTOP_ENTRY,
                                   g_param_spec_string(
                                           "desktop_entry",
                                           "desktop_entry",
                                           "Desktop Entry Path",
                                           "",
                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void genesis_app_entry_init (GenesisAppEntry *self)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (self);
  static WnckScreen *screen = NULL;
  priv->pid = 0;
  priv->splashscreen = NULL;
  if (!screen)
    screen = wnck_screen_get_default ();

  priv->screen = screen;
}

static void genesis_app_entry_get_property (GObject *object, guint prop_id,
                                            GValue *value, GParamSpec *pspec)
{
  GenesisAppEntry *self = GENESIS_APP_ENTRY (object);
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (self);

  switch (prop_id)
  {
    case PROP_DESKTOP_ENTRY:
      g_value_set_string (value, priv->filename);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void genesis_app_entry_set_property (GObject *object, guint prop_id,
                                            const GValue *value, GParamSpec *pspec)
{
  GenesisAppEntry *self = GENESIS_APP_ENTRY (object);
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (self);

  switch (prop_id)
  {
    case PROP_DESKTOP_ENTRY:
      priv->filename = g_strdup (g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gchar *genesis_app_entry_get_localized_name (GKeyFile *key_file)
{
  gchar *lang, *namefield, *name = NULL;
  const gchar *mylang;

  /* retrieve the language from the env var */
  if ((mylang = g_getenv ("LANG")) != NULL) 
  {
    lang = g_strdup (mylang);

    /* if the value has .UTF-8 appended, we */
    /* have to remove that as it's unneeded */
    g_strdelimit (lang, ".", '\0');

    /* this is the nametag for our language */
    namefield = g_strdup_printf ("%s[%s]", G_KEY_FILE_DESKTOP_KEY_NAME, lang);

    /* try to get out Name value */
    name = g_key_file_get_string(key_file, G_KEY_FILE_DESKTOP_GROUP, namefield, NULL);

    g_free(lang);
    g_free(namefield);
  }

  /* if we found no localized name, get the default */
  if (name == NULL) 
  {
    name = g_key_file_get_string(key_file,
                                 G_KEY_FILE_DESKTOP_GROUP, 
                                 G_KEY_FILE_DESKTOP_KEY_NAME,
                                 NULL);
  }

  return name;
}

static void genesis_app_entry_pid_watch_callback (GPid pid, gint status, gpointer data)
{
  GenesisAppEntry *entry = GENESIS_APP_ENTRY (data);
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);

  priv->pid = 0;
}

static void genesis_app_entry_window_opened (WnckScreen *screen, WnckWindow *window, GenesisAppEntry *entry)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);
  WnckWindowType type;
  int pid;

  type = wnck_window_get_window_type (window);
  pid = wnck_window_get_pid (window);

  // Check if a new normal window is opened with process id.
  if ((WNCK_WINDOW_NORMAL == type) && pid > 0)
    gtk_widget_hide_all (priv->splashscreen);
}

/* Public Functions */
gboolean genesis_app_entry_extract_info (GenesisAppEntry *entry, GHashTable *hashtable)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);
  GKeyFile *key_file = NULL;
  gchar **categories = NULL, **onlyshowin = NULL;
  gchar *splash_abs_path = NULL;
  GtkWidget *image = NULL;
  gchar **tmp;

  if (!priv->filename)
  {
    g_warning ("No desktop entry patch recorded, invalid app entry.\n");
    return FALSE;
  }

  key_file = g_key_file_new ();
  if (g_key_file_load_from_file (key_file, priv->filename, G_KEY_FILE_NONE, NULL) == FALSE)
  {
    g_warning ("Error reading '%s', exit.\n", priv->filename);
    return FALSE;
  }

  priv->showup = FALSE;
  onlyshowin = g_key_file_get_string_list (key_file,
                                           G_KEY_FILE_DESKTOP_GROUP,
                                           G_KEY_FILE_DESKTOP_KEY_ONLY_SHOW_IN,
                                           NULL,
                                           NULL);

  if (onlyshowin) 
  {
    for (gint j = 0; onlyshowin[j]; j++) 
    {
      if (!g_ascii_strcasecmp (onlyshowin[j], "MOBILE")) 
      {
        priv->showup = TRUE;
        break;
      }
    }
    g_strfreev (onlyshowin);
  }

  priv->name = genesis_app_entry_get_localized_name (key_file);
  priv->icon = g_key_file_get_string (key_file,
                                      G_KEY_FILE_DESKTOP_GROUP,
                                      G_KEY_FILE_DESKTOP_KEY_ICON,
                                      NULL);

  if (!priv->icon)
    priv->icon = g_strdup ("default");

  priv->exec = g_key_file_get_string (key_file,
                                      G_KEY_FILE_DESKTOP_GROUP,
                                      G_KEY_FILE_DESKTOP_KEY_EXEC,
                                      NULL);

  priv->splash_screen = g_key_file_get_string (key_file,
                                               G_KEY_FILE_DESKTOP_GROUP,
                                               KEY_FILE_DESKTOP_KEY_X_SPLASH,
                                               NULL);

  splash_abs_path = g_build_filename (DEFAULT_SPLASH_SCREEN_PATH,
                                      priv->splash_screen, NULL);

  priv->splashscreen = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  
  gtk_window_set_decorated (GTK_WINDOW (priv->splashscreen), FALSE);
  gtk_window_set_type_hint (GTK_WINDOW (priv->splashscreen), GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
  gtk_window_set_position (GTK_WINDOW (priv->splashscreen), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_stick (GTK_WINDOW (priv->splashscreen));

  if (g_file_test (splash_abs_path, G_FILE_TEST_IS_REGULAR))
  {
    image = gtk_image_new_from_file (splash_abs_path);
    gtk_container_add (GTK_CONTAINER (priv->splashscreen), image);
  }
  else
  {
    gchar *banner = g_strdup_printf ("Starting %s...", priv->name);
    image = gtk_label_new (banner);
    gtk_container_add (GTK_CONTAINER (priv->splashscreen), image);
  }

  if (priv->screen)
    g_signal_connect(G_OBJECT (priv->screen), "window_opened", 
		     G_CALLBACK (genesis_app_entry_window_opened), entry);

  categories = g_key_file_get_string_list(
    key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_CATEGORIES,
    NULL, NULL);

  printf("%s categories: ", priv->name);
  tmp = categories;
  while (tmp && *tmp) {
    GenesisCategory *category;

    /* Look up this entry in the GenesisController's hashtable of categories
     * to find the GenesisCategory entry.  If it doesn't exist, create it
     * and initialize it. */
    category = g_hash_table_lookup(hashtable, *tmp);
    if (!category)
    {
      category = g_new0(GenesisCategory, 1);
      category->name = g_strdup(*tmp);
      g_hash_table_insert(hashtable, category->name, category);
    }

    /* Add this category to the list of categories this application 
     * references */
    priv->categories = g_list_append(priv->categories, category->name);

    if (tmp == categories)
      category->is_primary = 1;

    /* Add this application to the list of applications this category
     * is referenced by */
    g_object_ref(G_OBJECT(entry));
    category->applications = g_list_prepend(category->applications, 
					    G_OBJECT(entry));

    printf("%s ", *tmp);

    tmp++;
  }
  printf("\n");
  g_strfreev(categories);

  /* Free the storage for the key_file */
  if (key_file)
    g_key_file_free (key_file);

  return TRUE;
}

void genesis_app_entry_start (GenesisAppEntry *entry)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);
  gchar *program = NULL;
  GError *error = NULL;
  gint argc;
  gchar **argv;
  gchar *space;

  if (priv->pid > 0)
    return;

  // launch a new instance
  space = strchr (priv->exec, ' ');
  if (space) 
  {
    gchar *cmd = g_strdup (priv->exec);
    cmd[space - priv->exec] = 0;
    gchar *exc = g_find_program_in_path (cmd);

    program = g_strconcat (exc, space, NULL);

    g_free (exc);
    g_free (cmd);
  }
  else
    program = g_find_program_in_path (priv->exec);

  if (!program) 
  {
    g_warning ("Attempt to exec invalid entry: %s", priv->exec);
    return;
  }

  if (priv->splashscreen)
    gtk_widget_show_all (priv->splashscreen);

  if (g_shell_parse_argv (program, &argc, &argv, &error)) 
    g_spawn_async (NULL, argv, NULL, 
                   G_SPAWN_DO_NOT_REAP_CHILD, 
                   NULL, NULL, &priv->pid, &error);

  if (error) 
  {
    g_warning ("Failed to execute %s: %s.", program, error->message);
    g_clear_error (&error);
  }
  else
  {
    int priority;
    errno = 0;
    gchar *oom_filename;
    int fd;

    /* If the child process inherited desktop's high
     * priority, give child default priority */
    priority = getpriority (PRIO_PROCESS, priv->pid);

    if (!errno && priority < 0)
      setpriority (PRIO_PROCESS, priv->pid, 0);

    g_child_watch_add (priv->pid, genesis_app_entry_pid_watch_callback, entry);

    /* Unprotect from OOM */
    oom_filename = g_strdup_printf ("/proc/%i/oom_adj", priv->pid);
    fd = open (oom_filename, O_WRONLY);
    g_free (oom_filename);

    if (fd >= 0) 
    {
      write (fd, "0", sizeof (char));
      close (fd);
    }
  }
}

void genesis_app_entry_set_exec (GenesisAppEntry *entry, const gchar *name)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);

  priv->exec = g_strdup (name);
}

void genesis_app_entry_set_name (GenesisAppEntry *entry, const gchar *name)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);

  priv->name = g_strdup (name);
}

gchar *genesis_app_entry_get_name (GenesisAppEntry *entry)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);

  return priv->name;
}

gchar *genesis_app_entry_get_icon (GenesisAppEntry *entry)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);

  return priv->icon;
}

gchar *genesis_app_entry_get_exec (GenesisAppEntry *entry)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);

  return priv->exec;
}

gboolean genesis_app_entry_is_showup (GenesisAppEntry *entry)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);

  return priv->showup;
}

gchar *genesis_app_entry_get_category (GenesisAppEntry *entry)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);

  return priv->categories->data;
}

GList *genesis_app_entry_get_categories (GenesisAppEntry *entry)
{
  GenesisAppEntryPrivate *priv = GENESIS_APP_ENTRY_GET_PRIVATE (entry);
  return g_list_copy(priv->categories);
}
