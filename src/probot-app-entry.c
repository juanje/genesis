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

#define PROBOT_APP_ENTRY_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), PROBOT_TYPE_APP_ENTRY, ProbotAppEntryPrivate))

G_DEFINE_TYPE (ProbotAppEntry, probot_app_entry, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_DESKTOP_ENTRY
};

struct _ProbotAppEntryPrivate
{
  gchar *filename;

  gchar *name;
  gchar *icon;
  gchar *exec;
  gchar *service;
  gchar *id;
  gboolean showup;
  gchar *category;
  gchar *splash_screen;

  GtkWidget *splashscreen;
  WnckScreen *screen;

  GPid pid;
};

static void probot_app_entry_get_property (GObject *object, guint prop_id,
                                           GValue *value, GParamSpec *pspec);

static void probot_app_entry_set_property (GObject *object, guint prop_id,
                                           const GValue *value, GParamSpec *pspec);

static void probot_app_entry_class_init (ProbotAppEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ProbotAppEntryPrivate));

  object_class->set_property = probot_app_entry_set_property;
  object_class->get_property = probot_app_entry_get_property;

  g_object_class_install_property (object_class,
                                   PROP_DESKTOP_ENTRY,
                                   g_param_spec_string(
                                           "desktop_entry",
                                           "desktop_entry",
                                           "Desktop Entry Path",
                                           "",
                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void probot_app_entry_init (ProbotAppEntry *self)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (self);
  static WnckScreen *screen = NULL;
  //priv->pid = -1;
  priv->pid = 0;
  priv->splashscreen = NULL;
  if (!screen)
    screen = wnck_screen_get_default ();

  priv->screen = screen;
}

static void probot_app_entry_get_property (GObject *object, guint prop_id,
                                           GValue *value, GParamSpec *pspec)
{
  ProbotAppEntry *self = PROBOT_APP_ENTRY (object);
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (self);

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

static void probot_app_entry_set_property (GObject *object, guint prop_id,
                                           const GValue *value, GParamSpec *pspec)
{
  ProbotAppEntry *self = PROBOT_APP_ENTRY (object);
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (self);

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

static gchar *probot_app_entry_get_localized_name (GKeyFile *key_file)
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

static void probot_app_entry_pid_watch_callback (GPid pid, gint status, gpointer data)
{
  ProbotAppEntry *entry = PROBOT_APP_ENTRY (data);
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);

  priv->pid = 0;
}

static void probot_app_entry_window_opened (WnckScreen *screen, WnckWindow *window, ProbotAppEntry *entry)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);
  WnckWindowType type;
  int pid;

  type = wnck_window_get_window_type (window);
  pid = wnck_window_get_pid (window);

  // Check if a new normal window is opened with process id.
  if ((WNCK_WINDOW_NORMAL == type) && pid > 0)
    gtk_widget_hide_all (priv->splashscreen);
}

/* Public Functions */
gboolean probot_app_entry_extract_info (ProbotAppEntry *entry, GHashTable *hashtable)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);
  GKeyFile *key_file = NULL;
  gchar **categories = NULL, **onlyshowin = NULL;
  gchar *splash_abs_path = NULL;
  GtkWidget *image = NULL;

  if (!priv->filename)
  {
    g_warning ("No desktop entry patch recorded, invalid app entry.\n");
    return FALSE;
  }

  key_file = g_key_file_new();
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

  priv->name = probot_app_entry_get_localized_name (key_file);
  priv->icon = g_key_file_get_string (key_file,
                                      G_KEY_FILE_DESKTOP_GROUP,
                                      G_KEY_FILE_DESKTOP_KEY_ICON,
                                      NULL);

  if (priv->icon)
  {
    //if there is an extension, strip it.  simple logic will catch 90% (.jpg, .png)
    int pos = strlen(priv->icon)-4;  //extension length = 4
    if (pos > 0 && priv->icon[pos] == '.')
      priv->icon[pos] = '\0';
  }

  if (!priv->icon)
    priv->icon = g_strdup ("null");

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
    g_signal_connect (G_OBJECT (priv->screen), "window_opened", G_CALLBACK (probot_app_entry_window_opened), entry);

  priv->category = NULL;
  categories = g_key_file_get_string_list (key_file,
                                           G_KEY_FILE_DESKTOP_GROUP,
                                           G_KEY_FILE_DESKTOP_KEY_CATEGORIES,
                                           NULL,
                                           NULL);

  if (NULL != categories)
  {
    for (gint j = 0; categories[j]; j++) 
    {
      priv->category = (gchar*) g_hash_table_lookup (hashtable, categories[j]);
      if (priv->category)
        break;
    }
    g_strfreev (categories);
  }

  if (!priv->category)
    priv->category = g_strdup ("Extras");

  return TRUE;
}

void probot_app_entry_start (ProbotAppEntry *entry)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);
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

    g_child_watch_add (priv->pid, probot_app_entry_pid_watch_callback, entry);

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

void probot_app_entry_set_exec (ProbotAppEntry *entry, const gchar *name)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);

  priv->exec = g_strdup (name);
}

void probot_app_entry_set_name (ProbotAppEntry *entry, const gchar *name)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);

  priv->name = g_strdup (name);
}

gchar *probot_app_entry_get_name (ProbotAppEntry *entry)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);

  return priv->name;
}

gchar *probot_app_entry_get_icon (ProbotAppEntry *entry)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);

  return priv->icon;
}

gchar *probot_app_entry_get_exec (ProbotAppEntry *entry)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);

  return priv->exec;
}

gboolean probot_app_entry_is_showup (ProbotAppEntry *entry)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);

  return priv->showup;
}

gchar *probot_app_entry_get_category (ProbotAppEntry *entry)
{
  ProbotAppEntryPrivate *priv = PROBOT_APP_ENTRY_GET_PRIVATE (entry);

  return priv->category;
}
