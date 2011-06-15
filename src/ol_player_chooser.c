/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2011  Tiger Soldier <tigersoldier@gmail.com>
 *
 * This file is part of OSD Lyrics.
 * 
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
#include "ol_player_chooser.h"
#include "ol_intl.h"
#include "ol_debug.h"

#define OL_PLAYER_CHOOSER_GET_PRIVATE(obj)   (G_TYPE_INSTANCE_GET_PRIVATE  \
                                              ((obj),                   \
                                               ol_player_chooser_get_type (), \
                                               OlPlayerChooserPrivate))
G_DEFINE_TYPE(OlPlayerChooser, ol_player_chooser, GTK_TYPE_DIALOG)

static const int DEFAULT_N_COLUMN = 4;

typedef struct _OlPlayerChooserPrivate
{
  int n_columns;
  GtkTable *supported_table;
  GtkTable *all_table;
  GtkExpander *supported_expander;
  GtkExpander *all_expander;
  GList *all_app_info;
} OlPlayerChooserPrivate;

static void _destroy (GtkObject *object);
static void _set_app_table (OlPlayerChooser *window,
                            GtkTable *table,
                            GList *app_list);
static void _set_supported_players (OlPlayerChooser *chooser,
                                    GList *supported_players);
static void _player_button_launch (GtkButton *button,
                                   GAppInfo *app_info);
static void _player_button_response (GtkButton *button,
                                     GtkDialog *dialog);

static void
ol_player_chooser_class_init (OlPlayerChooserClass *klass)
{
  ol_log_func ();
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;

  gobject_class = G_OBJECT_CLASS (klass);
  object_class = (GtkObjectClass*)klass;
  ol_player_chooser_parent_class = g_type_class_peek_parent (klass);

  object_class->destroy = _destroy;
  g_type_class_add_private (gobject_class, sizeof (OlPlayerChooserPrivate));
}

static void
ol_player_chooser_init (OlPlayerChooser *window)
{
  OlPlayerChooserPrivate *priv = OL_PLAYER_CHOOSER_GET_PRIVATE (window);
  priv->n_columns = DEFAULT_N_COLUMN;

  priv->supported_expander = GTK_EXPANDER (gtk_expander_new (_("Supported players")));
  priv->supported_table = GTK_TABLE (gtk_table_new (1, priv->n_columns, TRUE));
  gtk_container_add (GTK_CONTAINER (priv->supported_expander),
                     GTK_WIDGET (priv->supported_table));

  priv->all_expander = GTK_EXPANDER (gtk_expander_new (_("All players")));
  priv->all_table = GTK_TABLE (gtk_table_new (1, priv->n_columns, TRUE));
  gtk_container_add (GTK_CONTAINER (priv->all_expander),
                     GTK_WIDGET (priv->all_table));
  
  GtkWidget *cmd_hbox = gtk_hbox_new (FALSE, 0);
  GtkWidget *cmd_label = gtk_label_new (_("Use command:"));
  gtk_misc_set_padding (GTK_MISC (cmd_label), 5, 0);
  GtkWidget *cmd_entry = gtk_entry_new ();
  GtkWidget *cmd_button = gtk_button_new_with_label (_("Launch"));
  gtk_box_pack_start (GTK_BOX (cmd_hbox), cmd_label, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (cmd_hbox), cmd_entry, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (cmd_hbox), cmd_button, FALSE, TRUE, 0);

  GtkWidget *final_hbox = gtk_hbox_new (FALSE, 0);
  GtkWidget *remember_button = gtk_check_button_new_with_label (_("Remember my choice"));
  gtk_box_pack_start (GTK_BOX (final_hbox), remember_button, FALSE, TRUE, 0);

  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (window))),
                     vbox);
  gtk_box_pack_start (GTK_BOX (vbox),
                      GTK_WIDGET (priv->supported_expander),
                      FALSE,
                      TRUE,
                      0);
  gtk_box_pack_start (GTK_BOX (vbox),
                      GTK_WIDGET (priv->all_expander),
                      FALSE,
                      TRUE,
                      0);
  gtk_box_pack_start (GTK_BOX (vbox),
                      cmd_hbox,
                      FALSE,
                      TRUE,
                      0);
  gtk_box_pack_end (GTK_BOX (vbox),
                    final_hbox,
                    FALSE,
                    TRUE,
                    0);
  gtk_widget_show_all (vbox);

  gtk_dialog_add_button (GTK_DIALOG (window),
                         GTK_STOCK_QUIT,
                         GTK_RESPONSE_CLOSE);
  gtk_window_set_title (GTK_WINDOW (window), _("Choose a player to launch"));
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
}

static void
_destroy (GtkObject *object)
{
  GTK_OBJECT_CLASS (ol_player_chooser_parent_class)->destroy (object);
}

GtkWidget*
ol_player_chooser_new (GList *supported_players)
{
  ol_log_func ();
  GtkWidget *window = g_object_new (ol_player_chooser_get_type (), NULL);
  _set_supported_players (OL_PLAYER_CHOOSER (window), supported_players);
  return window;
}

static void
_player_button_launch (GtkButton *button,
                       GAppInfo *app_info)
{
  ol_assert (app_info != NULL);
  GError *err = NULL;
  if (!g_app_info_launch (app_info, NULL, NULL, &err))
  {
    ol_errorf ("Cannot launch %s: %s", g_app_info_get_commandline (app_info));
    g_error_free (err);
  }
}

static void
_player_button_response (GtkButton *button,
                         GtkDialog *dialog)
{
  gtk_dialog_response (dialog, OL_PLAYER_CHOOSER_RESPONSE_LAUNCH);
}

static void
_set_app_table (OlPlayerChooser *window, GtkTable *table, GList *app_list)
{
  ol_assert (GTK_IS_TABLE (table));
  int count = 0;
  GList *app = NULL;
  for (app = app_list; app != NULL; app = g_list_next (app))
  {
    if (g_app_info_should_show (app->data))
      count++;
  }
  if (count == 0) return;
  guint n_rows, n_columns;
  gtk_table_get_size (table, &n_rows, &n_columns);
  n_rows = (count - 1) / n_columns + 1;
  gtk_table_resize (table, n_rows, n_columns);
  guint row = 0, col = 0;
  for (app = app_list; app != NULL; app = g_list_next (app))
  {
    GAppInfo *app_info = app->data;
    if (g_app_info_should_show (app_info))
    {
      GtkWidget *button, *frame, *vbox, *image, *label;
      image = gtk_image_new_from_gicon (g_app_info_get_icon (app_info),
                                        GTK_ICON_SIZE_BUTTON);
      gtk_image_set_pixel_size (GTK_IMAGE (image), 64);

      label = gtk_label_new (g_app_info_get_display_name (app_info));
      gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
      gtk_widget_set_size_request (label, 80, -1);

      vbox = gtk_vbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

      frame = gtk_aspect_frame_new (NULL, 0.5, 0.5, 1.0, FALSE);
      gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
      gtk_container_add (GTK_CONTAINER (frame), vbox);

      button = gtk_button_new ();
      gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
      gtk_container_add (GTK_CONTAINER (button), frame);
      g_signal_connect (button,
                        "clicked",
                        G_CALLBACK (_player_button_launch),
                        app_info);
      g_signal_connect (button,
                        "clicked",
                        G_CALLBACK (_player_button_response),
                        window);

      gtk_table_attach_defaults (table, button, col, col + 1, row, row + 1);
      row += (col + 1) / n_columns;
      col = (col + 1) % n_columns;
    }
  }
  gtk_widget_show_all (GTK_WIDGET (table));
}

static void
_set_supported_players (OlPlayerChooser *window,
                        GList *supported_players)
{
  ol_assert (OL_IS_PLAYER_CHOOSER (window));
  OlPlayerChooserPrivate *priv = OL_PLAYER_CHOOSER_GET_PRIVATE (window);
  window->supported_players = supported_players;
  priv->all_app_info = g_app_info_get_all_for_type ("audio/mp3");
  _set_app_table (window, priv->all_table, priv->all_app_info);
  gtk_expander_set_expanded (priv->all_expander, TRUE);
}
