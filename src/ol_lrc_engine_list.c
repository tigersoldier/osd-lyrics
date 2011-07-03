/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009-2011  Tiger Soldier <tigersoldi@gmail.com>
 *
 * This file is part of OSD Lyrics.
 * OSD Lyrics is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OSD Lyrics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OSD Lyrics.  If not, see <http://www.gnu.org/licenses/>. 
 */
#include "ol_lrc_engine_list.h"
#include "ol_intl.h"
#include "ol_utils.h"
#include "ol_debug.h"

const int COLUMN_ORDER_INF = INT_MAX;

static void _cell_toggled_cb (GtkCellRendererToggle *cell_renderer,
                              gchar *path,
                              gpointer user_data);

static void
_cell_toggled_cb (GtkCellRendererToggle *cell_renderer,
                  gchar *path,
                  gpointer user_data)
{
  GtkListStore *liststore = GTK_LIST_STORE (user_data);
  gboolean active = gtk_cell_renderer_toggle_get_active (cell_renderer);
  GtkTreeIter iter;
  gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (liststore), &iter, path);
  gtk_list_store_set (liststore, &iter, 0, !active, -1);
}

void
ol_lrc_engine_list_init (GtkTreeView *list)
{
  ol_assert (list != NULL);
  ol_assert (GTK_IS_TREE_VIEW (list));
  GtkListStore *liststore = gtk_list_store_new (3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);
  int i, nengine;
  char **download_engine = (char **)ol_lrc_fetch_get_engine_list (&nengine);
  for (i = 0; i < nengine; i++)
  {
    GtkTreeIter iter;
    gtk_list_store_append (liststore, &iter);
    gtk_list_store_set (liststore, &iter,
                        0, TRUE,
                        1, _(download_engine[i]),
                        2, download_engine[i],
                        -1);
  }
  gtk_tree_view_set_model (list, GTK_TREE_MODEL (liststore));
  GtkCellRenderer *check_renderer = gtk_cell_renderer_toggle_new ();
  gtk_cell_renderer_toggle_set_radio (GTK_CELL_RENDERER_TOGGLE (check_renderer),
                                      FALSE);
  g_signal_connect (G_OBJECT (check_renderer),
                    "toggled",
                    (GCallback) _cell_toggled_cb,
                    liststore);
  gtk_tree_view_insert_column_with_attributes (list,
                                               0,
                                               NULL,
                                               check_renderer,
                                               "active", 0,
                                               NULL);
  gtk_tree_view_insert_column_with_attributes (list,
                                               1,
                                               NULL,
                                               gtk_cell_renderer_text_new (),
                                               "text", 1,
                                               NULL);
}

char **
ol_lrc_engine_list_get_engine_names (GtkTreeView *list)
{
  ol_assert_ret (list != NULL, NULL);
  ol_assert_ret (GTK_IS_TREE_VIEW (list), NULL);
  GPtrArray *engine_array = g_ptr_array_new ();
  GtkTreeModel *liststore = gtk_tree_view_get_model (list);
  GtkTreeIter iter;
  gboolean valid;
  for (valid = gtk_tree_model_get_iter_first (liststore, &iter);
       valid;
       valid = gtk_tree_model_iter_next (liststore, &iter))
  {
    GValue checked_value = {0};
    gtk_tree_model_get_value (liststore, &iter, 0, &checked_value);
    if (!G_VALUE_HOLDS_BOOLEAN (&checked_value))
    {
      ol_error ("The first column of engine list is not gboolean");
      g_value_unset (&checked_value);
      continue;
    }
    gboolean checked = g_value_get_boolean (&checked_value);
    g_value_unset (&checked_value);
    if (checked)
    {
      GValue name_value = {0};
      gtk_tree_model_get_value (liststore, &iter, 2, &name_value);
      if (!G_VALUE_HOLDS_STRING (&name_value))
      {
        ol_error ("The third column of engine list is not a string");
      }
      else
      {
        g_ptr_array_add (engine_array, g_value_dup_string (&name_value));
      }
      g_value_unset (&name_value);
    }
  }
  g_ptr_array_add (engine_array, NULL);
  return (char**)g_ptr_array_free (engine_array, FALSE);
}

void
ol_lrc_engine_list_set_engine_names (GtkTreeView *list,
                                     char **name_list)
{
  ol_assert (GTK_IS_TREE_VIEW (list));
  ol_assert (name_list != NULL);
  ol_log_func ();
  GtkTreeModel *liststore = GTK_TREE_MODEL (gtk_tree_view_get_model (list));
  GtkTreeIter iter;
  gboolean valid;
  int count = 0;
  int enable_count = 0;
  int i;
  for (valid = gtk_tree_model_get_iter_first (liststore, &iter);
       valid;
       valid = gtk_tree_model_iter_next (liststore, &iter))
  {
    gtk_list_store_set (GTK_LIST_STORE (liststore),
                        &iter,
                        0, FALSE,
                        -1);
    count++;
  }
  gint *new_order = g_new (gint, count);
  for (; *name_list != NULL; name_list++)
  {
    ol_debugf ("setting %s\n", *name_list);
    gboolean found = FALSE;
    for (valid = gtk_tree_model_get_iter_first (liststore, &iter), i = 0;
         valid && !found;
         valid = gtk_tree_model_iter_next (liststore, &iter), i++)
    {
      GValue name_value = {0};
      gtk_tree_model_get_value (liststore, &iter, 2, &name_value);
      ol_debugf ("value is %s\n", g_value_get_string (&name_value));
      if (strcasecmp (*name_list, g_value_get_string (&name_value)) == 0)
      {
        gtk_list_store_set (GTK_LIST_STORE (liststore), &iter,
                            0, TRUE,
                            -1);
        new_order[enable_count] = i;
        found = TRUE;
        enable_count++;
      }
      g_value_unset (&name_value);
    }
  }
  for (valid = gtk_tree_model_get_iter_first (liststore, &iter), i = 0;
       valid;
       valid = gtk_tree_model_iter_next (liststore, &iter), i++)
  {
    GValue check_value = {0};
    gtk_tree_model_get_value (liststore, &iter, 0, &check_value);
    if (g_value_get_boolean (&check_value) == 0)
    {
      new_order[enable_count] = i;
      enable_count++;
    }
    g_value_unset (&check_value);
  }
  gtk_list_store_reorder (GTK_LIST_STORE (liststore), new_order);
  g_free (new_order);
}

