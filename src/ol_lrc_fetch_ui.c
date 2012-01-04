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
#include <stdlib.h>
#include "ol_lrc_fetch_ui.h"
#include "ol_lrc_fetch.h"
#include "ol_gui.h"
#include "ol_config_proxy.h"
#include "ol_lrc_fetch_module.h"
#include "ol_lrc_candidate_list.h"
#include "ol_debug.h"

static GtkWidget *window = NULL;
static GtkTreeView *list = NULL;
static GtkButton *download_button = NULL;
static OlLrcFetchEngine *engine = NULL;
static OlMetadata *metadata = NULL;
static char *filepath = NULL;

static void ol_lrc_fetch_select_changed (GtkTreeSelection *selection, gpointer data);
static gboolean internal_init ();
gboolean ol_lrc_fetch_cancel (GtkWidget *widget, gpointer data);

gboolean
ol_lrc_fetch_ui_cancel (GtkWidget *widget, gpointer data)
{
  ol_log_func ();
  if (window != NULL)
    gtk_widget_hide (window);
  return TRUE;
}

gboolean
ol_lrc_fetch_ui_download (GtkWidget *widget, gpointer data)
{
  ol_log_func ();
  OlLrcCandidate candidate = {{0}};
  if (ol_lrc_candidate_list_get_selected (list, &candidate))
  {
    ol_lrc_fetch_begin_download (engine, &candidate, metadata, NULL);
  }
  OlConfigProxy *config = ol_config_proxy_get_instance ();
  GtkToggleButton *prompt_btn = GTK_TOGGLE_BUTTON (ol_gui_get_widget ("choose-do-not-prompt"));
  if (prompt_btn != NULL && config != NULL)
  {
    if (gtk_toggle_button_get_active (prompt_btn))
    {
      ol_config_proxy_set_bool (config, 
                                "Download/download-first-lyric", 
                                TRUE);
    }
  }
  gtk_widget_hide (window);
  return TRUE;
}

static void
ol_lrc_fetch_select_changed (GtkTreeSelection *selection, gpointer data)
{
  ol_log_func ();
  if (download_button != NULL)
    gtk_widget_set_sensitive (GTK_WIDGET (download_button),
                              gtk_tree_selection_get_selected (selection, NULL, NULL));
}

static gboolean
internal_init ()
{
  ol_log_func ();
  if (window == NULL)
  {
    window = ol_gui_get_widget ("downloaddialog");
    if (window == NULL)
      return FALSE;
    g_signal_connect (G_OBJECT (window),
                      "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete),
                      NULL);
  }
  if (download_button == NULL)
  {
    download_button = GTK_BUTTON (ol_gui_get_widget ("lrc-download"));
  }
  if (list == NULL)
  {
    list = GTK_TREE_VIEW (ol_gui_get_widget ("candidate-list"));
    ol_lrc_candidate_list_init (list, 
                                G_CALLBACK (ol_lrc_fetch_select_changed));
  }
  return TRUE;
}

void
ol_lrc_fetch_ui_show (OlLrcFetchEngine *lrcengine,
                      const OlLrcCandidate *candidates,
                      int count,
                      const OlMetadata *_metadata)
{
  ol_log_func ();
  if (window == NULL && !internal_init ())
    return;
  if (lrcengine == NULL || candidates == NULL || count <= 0)
  {
    gtk_widget_hide (window);
    return;
  }
  if (filepath != NULL)
    g_free (filepath);
  engine = lrcengine;
  ol_lrc_candidate_list_set_list (list, candidates, count);
  if (metadata == NULL)
    metadata = ol_metadata_new ();
  ol_metadata_copy (metadata, _metadata);
  gboolean prompt = TRUE;
  OlConfigProxy *config = ol_config_proxy_get_instance ();
  if (config != NULL)
    prompt = ol_config_proxy_get_bool (config, "Download/download-first-lyric");
  if (prompt || count == 1)
    ol_lrc_fetch_ui_download (GTK_WIDGET (download_button), NULL);
  else
    gtk_widget_show (window);
}
