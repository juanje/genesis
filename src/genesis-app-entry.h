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

#ifndef GENESIS_APP_ENTRY_H
#define GENESIS_APP_ENTRY_H

G_BEGIN_DECLS

typedef struct _GenesisAppEntry GenesisAppEntry;
typedef struct _GenesisAppEntryClass GenesisAppEntryClass;
typedef struct _GenesisAppEntryPrivate GenesisAppEntryPrivate;

#define GENESIS_TYPE_APP_ENTRY                (genesis_app_entry_get_type ())
#define GENESIS_APP_ENTRY(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), GENESIS_TYPE_APP_ENTRY, GenesisAppEntry))
#define GENESIS_APP_ENTRY_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), GENESIS_TYPE_APP_ENTRY, GenesisAppEntryClass))
#define IS_GENESIS_APP_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GENESIS_TYPE_APP_ENTRY))
#define IS_GENESIS_APP_ENTRY_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), GENESIS_TYPE_APP_ENTRY))
#define GENESIS_APP_ENTRY_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), GENESIS_TYPE_APP_ENTRY, GenesisAppEntryClass))

#define KEY_FILE_DESKTOP_KEY_X_SPLASH "X-Splash"
#define DEFAULT_SPLASH_SCREEN_PATH "/usr/share/pixmaps/splash"

struct _GenesisAppEntry {
  GObject parent;

  GenesisAppEntryPrivate *priv;
};

struct _GenesisAppEntryClass {
  GObjectClass parent;
};

GType genesis_app_entry_get_type (void);

/* Public Functions */
gboolean genesis_app_entry_extract_info (GenesisAppEntry *entry, GHashTable *hashtable);
void genesis_app_entry_start (GenesisAppEntry *entry);
void genesis_app_entry_set_name (GenesisAppEntry *entry, const gchar *name);
void genesis_app_entry_set_exec (GenesisAppEntry *entry, const gchar *name);
gchar *genesis_app_entry_get_name (GenesisAppEntry *entry);
gchar *genesis_app_entry_get_icon (GenesisAppEntry *entry);
gchar *genesis_app_entry_get_exec (GenesisAppEntry *entry);
gboolean genesis_app_entry_is_showup (GenesisAppEntry *entry);
gchar *genesis_app_entry_get_category (GenesisAppEntry *entry);

G_END_DECLS

#endif /* GENESIS_APP_ENTRY_H */
