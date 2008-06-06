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

#include "gtk/gtk.h"
#include "probot-common.h"

static void selection_changed_callback (GtkTreeSelection *selection, gpointer user_data)
{
  GtkWidget *textview = GTK_WIDGET (user_data);
  GtkTextBuffer *buffer;
  GtkWidget *treeview;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *entry_name = NULL;
  gchar *entry_info = NULL;
  ProbotController *controller = probot_controller_get_singleton ();
  ProbotAppEntry *entry = NULL;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    treeview = GTK_WIDGET (gtk_tree_selection_get_tree_view (selection));
    gtk_tree_model_get (model, &iter, 0, &entry_name, -1);

    entry = probot_controller_get_entry_by_name (controller, entry_name);
    if (entry)
      entry_info = g_strdup_printf ("-=-=-=-=-=-=-=-\nAppEntry Information \nname : %s \nicon : %s \nexec : %s \nshowup : %d \ncategory : %s \n-=-=-=-=-=-=-=-\n",
                                    probot_app_entry_get_name (entry),
                                    probot_app_entry_get_icon (entry),
                                    probot_app_entry_get_exec (entry),
                                    probot_app_entry_is_showup (entry),
                                    probot_app_entry_get_category (entry));
    else
      entry_info = g_strdup ("Invalid desktop entry!");

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    gtk_text_buffer_set_text (buffer, entry_info, -1);

    gtk_widget_show_all (textview);
  } 
}

static void start_button_clicked_callback (GtkButton *button, gpointer user_data)
{
  GtkWidget *treeview = GTK_WIDGET (user_data);
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *entry_name = NULL;
  ProbotController *controller = probot_controller_get_singleton ();

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, 0, &entry_name, -1);
    probot_controller_start_app_from_name (controller, entry_name);
  }
}

static void app_entry_updated_callback (ProbotController *controller, guint event_type, gchar *info_path, gpointer user_data)
{
  g_print ("app_entry_updated_callback: entered\n");
  if (info_path)
    g_print ("app_entry_updated_callback: event %d happened to %s\n", event_type, info_path);
  else
    g_print ("app_entry_updated_callback: event %d happened to NULL\n", event_type);
}

static void tree_view_append_entry (GtkTreeModel *model, gchar *entry_name)
{
  GtkTreeIter iter;
 
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, entry_name, -1);
}

static void home_window_construct (ProbotController *controller)
{
  GtkWidget *window;
  GtkWidget *hbox, *vbox;
  GtkWidget *treeview;
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkWidget *textview, *button;
  ProbotAppEntry *entry = NULL;
  guint n = 0;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Probots Face");
  g_signal_connect (G_OBJECT (window), "delete-event",  (GCallback) gtk_main_quit, NULL);

  hbox = gtk_hbox_new (FALSE,0);
  gtk_container_add (GTK_CONTAINER (window), hbox);

  vbox = gtk_vbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

  textview = gtk_text_view_new ();
  gtk_box_pack_start (GTK_BOX(vbox), textview, FALSE, FALSE, 0);

  treeview = gtk_tree_view_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview), 
                                               -1, "Name", 
                                               gtk_cell_renderer_text_new(), 
                                               "text", 0, NULL);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (selection), "changed",
                    G_CALLBACK (selection_changed_callback), textview);
  gtk_box_pack_start (GTK_BOX(hbox), treeview, FALSE, FALSE, 0);

  model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
  gtk_tree_view_set_model (GTK_TREE_VIEW(treeview), model);

  do
  {
    entry = probot_controller_get_nth_entry (controller, n);
    if (entry)
    {
      gchar *entry_name = probot_app_entry_get_name (entry);

      tree_view_append_entry (model, entry_name);
      n++;
    }
  }while (entry);

  button = gtk_button_new_with_label ("Start");

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (start_button_clicked_callback), treeview);

  gtk_box_pack_start (GTK_BOX(vbox), button, FALSE, FALSE, 0);

  gtk_widget_show_all (window);
}

int main(int argc, char *argv[])
{
  ProbotController *controller = NULL;

  gtk_init (&argc, &argv);

  controller = probot_controller_get_singleton ();

  if (!controller)
  {
    g_warning ("Failed to get probot controller singleton, returned.\n");
    return -1;
  }

  g_signal_connect (G_OBJECT(controller), "app-entry-updated",
                    G_CALLBACK(app_entry_updated_callback), NULL);

  home_window_construct (controller);

  gtk_main ();

  return 0;
}
