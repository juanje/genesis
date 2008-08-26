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
#include "genesis-common.h"

static void tree_view_append_entry (GtkTreeModel *model, gchar *entry_name);

static void app_selection_changed_callback (GtkTreeSelection *selection, gpointer user_data)
{
  GtkWidget *textview = GTK_WIDGET (user_data);
  GtkTextBuffer *buffer;
  GtkWidget *treeview;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *entry_name = NULL;
  gchar *entry_info = NULL;
  GenesisController *controller = genesis_controller_get_singleton ();
  GenesisAppEntry *entry = NULL;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    treeview = GTK_WIDGET (gtk_tree_selection_get_tree_view (selection));
    gtk_tree_model_get (model, &iter, 0, &entry_name, -1);

    entry = genesis_controller_get_entry_by_name (controller, entry_name);
    if (entry)
    {
      GList *categories = genesis_app_entry_get_categories(entry);

      if (categories)
      {
	GList *tmp;
	gchar *tmp_entry;
	
	tmp = categories;
	entry_info = g_strdup_printf (
	  "-=-=-=-=-=-=-=-\n"
	  "AppEntry Information \n"
	  "name : %s \n"
	  "icon : %s \n"
	  "exec : %s \n"
	  "showup : %d \n"
	  "category : %s",
	  genesis_app_entry_get_name (entry),
	  genesis_app_entry_get_icon (entry),
	  genesis_app_entry_get_exec (entry),
	  genesis_app_entry_is_showup (entry),
	  (gchar *)tmp->data);
	tmp = tmp->next;
	while (tmp)
	{
	  tmp_entry = entry_info;
	  entry_info = g_strdup_printf("%s, %s", tmp_entry, (gchar *)tmp->data);
	  g_free(tmp_entry);
	  tmp = tmp->next;
	}
	
	tmp_entry = entry_info;
	entry_info = g_strdup_printf("%s\n"
				     "-=-=-=-=-=-=-=-\n", tmp_entry);
	g_free(tmp_entry);
      } 
    }

    if (!entry_info)
      entry_info = g_strdup ("Invalid desktop entry!");

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    gtk_text_buffer_set_text (buffer, entry_info, -1);
    g_free(entry_info);
    gtk_widget_show_all (textview);
  } 
}

static void cat_selection_changed_callback (GtkTreeSelection *selection, gpointer user_data)
{
  GtkWidget *app_treeview = GTK_WIDGET (user_data);
  GtkTreeModel *cat_model, *app_model;
  GtkTreeIter iter;
  gchar *selected_category;
  GenesisController *controller = genesis_controller_get_singleton ();
  GList *applications;
  GList *tmp;

  if (!gtk_tree_selection_get_selected (selection, &cat_model, &iter))
    return;

  gtk_tree_model_get (cat_model, &iter, 0, &selected_category, -1);

  app_model = gtk_tree_view_get_model (GTK_TREE_VIEW (app_treeview));
  gtk_list_store_clear (GTK_LIST_STORE (app_model));

  applications = genesis_controller_get_applications_by_category (
    controller, selected_category);
  tmp = applications;
  while (tmp)
  {
    GenesisAppEntry *entry = tmp->data;

    tree_view_append_entry (app_model, genesis_app_entry_get_name (entry));
    tmp = tmp->next;
  }
}

static void start_button_clicked_callback (GtkButton *button, gpointer user_data)
{
  GtkWidget *treeview = GTK_WIDGET (user_data);
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *entry_name = NULL;
  GenesisController *controller = genesis_controller_get_singleton ();

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, 0, &entry_name, -1);
    genesis_controller_start_app_from_name (controller, entry_name);
  }
}

static void app_entry_updated_callback (GenesisController *controller, guint event_type, gchar *info_path, gpointer user_data)
{
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

static void home_window_construct (GenesisController *controller)
{
  GtkWidget *window;
  GtkWidget *hbox, *vbox;
  GtkWidget *app_treeview, *cat_treeview;
  GtkWidget *scrolledwindow;
  GtkTreeModel *app_model, *cat_model;
  GtkTreeSelection *app_selection, *cat_selection;
  GtkWidget *textview, *button;
  GenesisAppEntry *entry = NULL;
  guint n = 0;
  GList *cat_list;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Genesis Face");
  g_signal_connect (G_OBJECT (window), "delete-event",  (GCallback) gtk_main_quit, NULL);

  hbox = gtk_hbox_new (FALSE,0);
  gtk_container_add (GTK_CONTAINER (window), hbox);

  vbox = gtk_vbox_new (FALSE,0);
  gtk_box_pack_end (GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

  textview = gtk_text_view_new ();
  gtk_box_pack_start (GTK_BOX(vbox), textview, FALSE, FALSE, 0);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), 
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_end (GTK_BOX(hbox), scrolledwindow, FALSE, FALSE, 0);

  app_treeview = gtk_tree_view_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (app_treeview),
                                               -1, "Applications",
                                               gtk_cell_renderer_text_new(),
                                               "text", 0, NULL);
  app_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (app_treeview));

  gtk_tree_selection_set_mode (app_selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (app_selection), "changed",
                    G_CALLBACK (app_selection_changed_callback), textview);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), app_treeview);

  app_model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
  gtk_tree_view_set_model (GTK_TREE_VIEW(app_treeview), app_model);

  do
  {
    entry = genesis_controller_get_nth_entry (controller, n++);
    if (entry)
    {
      gchar *entry_name = genesis_app_entry_get_name (entry);

      tree_view_append_entry (app_model, entry_name);
    }
  }while (entry);

  cat_list = genesis_controller_get_categories (controller);

  if (!cat_list)
    g_print ("failed to get categories list.\n");
  else
  {
    GList *tmp;

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), 
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_end (GTK_BOX(hbox), scrolledwindow, FALSE, FALSE, 0);

    cat_treeview = gtk_tree_view_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (cat_treeview),
                                                 -1, "Categories",
                                                 gtk_cell_renderer_text_new(),
                                                 "text", 0, NULL);
    cat_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (cat_treeview));

    gtk_tree_selection_set_mode (cat_selection, GTK_SELECTION_SINGLE);
    g_signal_connect (G_OBJECT (cat_selection), "changed",
                      G_CALLBACK (cat_selection_changed_callback), app_treeview);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), cat_treeview);

    cat_model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
    gtk_tree_view_set_model (GTK_TREE_VIEW(cat_treeview), cat_model);

    tmp = cat_list;
    while (tmp) 
    {
      GenesisCategory *category = tmp->data;
      tree_view_append_entry (cat_model, category->name);
      tmp = tmp->next;
    }
    g_list_free(cat_list);
  }

  button = gtk_button_new_with_label ("Start");

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (start_button_clicked_callback), app_treeview);

  gtk_box_pack_start (GTK_BOX(vbox), button, FALSE, FALSE, 0);

  gtk_widget_show_all (window);
}

int main(int argc, char *argv[])
{
  GenesisController *controller = NULL;

  gtk_init (&argc, &argv);

  controller = genesis_controller_get_singleton ();

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
