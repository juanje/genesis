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

#ifndef GENESIS_UTILS_H
#define GENESIS_UTILS_H

// FIXEME: just define NODAEMON for using g_print to print out the debug message
// instead of save it to log file while we are not in daemon mode

#define NODAEMON

G_BEGIN_DECLS

gchar* uri_to_path (const gchar *uri);
gchar* path_to_uri (gchar *path);

//void save_log (const gchar *format, ...);

#ifndef NODAEMON
void save_log (const gchar *format, ...);
#else

#define save_log(fmtstr, args...) (g_print("%s:" fmtstr "\n", __func__, ##args))

#endif

G_END_DECLS

#endif /* GENESIS_UTILS_H */
