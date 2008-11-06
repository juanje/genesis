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

#include <glib.h>
#include <glib/gstdio.h>

#include "genesis-utils.h"

gchar* uri_to_path (const gchar *uri)
{
  gchar **splitted_uri = NULL;
 
  splitted_uri = g_strsplit (uri, "file://", 2);
 
  return splitted_uri[1];
}

gchar* path_to_uri (gchar *path)
{
  return g_strconcat ("file://", path, NULL);
}

#if 0
void save_log (const gchar *format, ...)
{
  FILE* log_file = NULL;
  va_list args;

  if (!log_file)
    log_file = fopen (g_build_filename (g_get_home_dir (), ".genesisd.log", NULL), "a+");

  va_start (args, format);
  fprintf (log_file, format, args);
  va_end (args);

  fclose (log_file);
}
#endif
