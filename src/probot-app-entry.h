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

#ifndef PROBOT_APP_ENTRY_H
#define PROBOT_APP_ENTRY_H

G_BEGIN_DECLS

typedef struct _ProbotAppEntry ProbotAppEntry;
typedef struct _ProbotAppEntryClass ProbotAppEntryClass;
typedef struct _ProbotAppEntryPrivate ProbotAppEntryPrivate;

#define PROBOT_TYPE_APP_ENTRY                (probot_app_entry_get_type ())
#define PROBOT_APP_ENTRY(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), PROBOT_TYPE_APP_ENTRY, ProbotAppEntry))
#define PROBOT_APP_ENTRY_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), PROBOT_TYPE_APP_ENTRY, ProbotAppEntryClass))
#define IS_PROBOT_APP_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PROBOT_TYPE_APP_ENTRY))
#define IS_PROBOT_APP_ENTRY_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), PROBOT_TYPE_APP_ENTRY))
#define PROBOT_APP_ENTRY_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), PROBOT_TYPE_APP_ENTRY, ProbotAppEntryClass))

struct _ProbotAppEntry {
  GObject parent;

  ProbotAppEntryPrivate *priv;
};

struct _ProbotAppEntryClass {
  GObjectClass parent;
};

GType probot_app_entry_get_type (void);

/* Public Functions */
gboolean probot_app_entry_extract_info (ProbotAppEntry *entry, GHashTable *hashtable);
void probot_app_entry_start (ProbotAppEntry *entry);
void probot_app_entry_set_name (ProbotAppEntry *entry, const gchar *name);
void probot_app_entry_set_exec (ProbotAppEntry *entry, const gchar *name);
gchar *probot_app_entry_get_name (ProbotAppEntry *entry);
gchar *probot_app_entry_get_icon (ProbotAppEntry *entry);
gchar *probot_app_entry_get_exec (ProbotAppEntry *entry);
gboolean probot_app_entry_is_showup (ProbotAppEntry *entry);
gchar *probot_app_entry_get_category (ProbotAppEntry *entry);

G_END_DECLS

#endif /* PROBOT_APP_ENTRY_H */
