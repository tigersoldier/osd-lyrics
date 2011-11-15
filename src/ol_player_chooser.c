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
#include "ol_app_chooser_widget.h"
#include "ol_intl.h"
#include "ol_utils.h"
#include "ol_config.h"
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
  OlAppChooserWidget *supported_chooser;
  OlAppChooserWidget *all_chooser;
  GtkExpander *supported_expander;
  GtkExpander *all_expander;
  GtkWidget *remember_button;
  GtkEntry *cmd_entry;
  GtkWidget *launch_button;
} OlPlayerChooserPrivate;

static void _destroy (GtkObject *object);
static void _set_supported_players (OlPlayerChooser *chooser,
                                    GList *supported_players);
static void _app_activate_cb (OlAppChooserWidget *chooser,
                              GAppInfo *info,
                              OlPlayerChooser *window);
static void _launch_button_clicked_cb (GtkButton *button,
                                       OlPlayerChooser *window);
static void _remember_cmd_if_needed (OlPlayerChooser *window,
                                     const char *cmd);

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
  priv->supported_chooser = OL_APP_CHOOSER_WIDGET (ol_app_chooser_widget_new ());
  g_signal_connect (priv->supported_chooser,
                    "app-activate",
                    G_CALLBACK (_app_activate_cb),
                    window);
  gtk_container_add (GTK_CONTAINER (priv->supported_expander),
                     GTK_WIDGET (priv->supported_chooser));

  priv->all_expander = GTK_EXPANDER (gtk_expander_new (_("All players")));
  priv->all_chooser = OL_APP_CHOOSER_WIDGET (ol_app_chooser_widget_new ());
  g_signal_connect (priv->all_chooser,
                    "app-activate",
                    G_CALLBACK (_app_activate_cb),
                    window);
  gtk_container_add (GTK_CONTAINER (priv->all_expander),
                     GTK_WIDGET (priv->all_chooser));
  
  GtkWidget *cmd_hbox = gtk_hbox_new (FALSE, 0);
  GtkWidget *cmd_label = gtk_label_new (_("Use command:"));
  gtk_misc_set_padding (GTK_MISC (cmd_label), 5, 0);
  GtkWidget *cmd_entry = gtk_entry_new ();
  priv->cmd_entry = GTK_ENTRY (cmd_entry);
  gtk_entry_set_activates_default (priv->cmd_entry, TRUE);
  GtkWidget *launch_button = gtk_button_new_with_label (_("Launch"));
  gtk_widget_set_can_default (launch_button, TRUE);
  gtk_window_set_default (GTK_WINDOW (window), launch_button);
  priv->launch_button = launch_button;
  g_signal_connect (launch_button,
                    "clicked",
                    G_CALLBACK (_launch_button_clicked_cb),
                    window);
  gtk_box_pack_start (GTK_BOX (cmd_hbox), cmd_label, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (cmd_hbox), cmd_entry, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (cmd_hbox), launch_button, FALSE, TRUE, 0);

  GtkWidget *final_hbox = gtk_hbox_new (FALSE, 0);
  GtkWidget *remember_button = gtk_check_button_new_with_label (_("Remember my choice"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (remember_button), TRUE);
  priv->remember_button = remember_button;
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
_set_supported_players (OlPlayerChooser *window,
                        GList *supported_players)
{
  ol_assert (OL_IS_PLAYER_CHOOSER (window));
  OlPlayerChooserPrivate *priv = OL_PLAYER_CHOOSER_GET_PRIVATE (window);
  gtk_expander_set_expanded (priv->all_expander, TRUE);
  if (supported_players != NULL)
  {
    ol_app_chooser_widget_set_app_list (priv->supported_chooser,
                                        supported_players,
                                        0);
    gtk_expander_set_expanded (priv->supported_expander, TRUE);
    gtk_expander_set_expanded (priv->all_expander, FALSE);
  }
  ol_app_chooser_widget_set_app_list (priv->all_chooser,
                                      g_app_info_get_all_for_type ("audio/mp3"),
                                      ol_app_chooser_widget_get_columns (priv->supported_chooser));
}

static void
_app_activate_cb (OlAppChooserWidget *chooser,
                  GAppInfo *app_info,
                  OlPlayerChooser *window)
{
  ol_assert (G_IS_APP_INFO (app_info));
  GError *err = NULL;
  if (!g_app_info_launch (app_info, NULL, NULL, &err))
  {
    ol_errorf ("Cannot launch %s: %s", g_app_info_get_commandline (app_info));
    g_error_free (err);
  }
  if (OL_IS_PLAYER_CHOOSER (window))
	{
    _remember_cmd_if_needed (window, g_app_info_get_commandline (app_info));
    gtk_dialog_response (GTK_DIALOG (window), OL_PLAYER_CHOOSER_RESPONSE_LAUNCH);
  }
}

static void
_launch_button_clicked_cb (GtkButton *button,
                           OlPlayerChooser *window)
{
  ol_assert (OL_IS_PLAYER_CHOOSER (window));
  OlPlayerChooserPrivate *priv = OL_PLAYER_CHOOSER_GET_PRIVATE (window);
  const char *cmd = gtk_entry_get_text (priv->cmd_entry);
  if (ol_is_string_empty (cmd))
    return;
  g_spawn_command_line_async (cmd, NULL);
  _remember_cmd_if_needed (window, cmd);
  gtk_dialog_response (GTK_DIALOG (window), OL_PLAYER_CHOOSER_RESPONSE_LAUNCH);
}

static void
_remember_cmd_if_needed (OlPlayerChooser *window,
                         const char *cmd)
{
  ol_assert (OL_IS_PLAYER_CHOOSER (window));
  OlPlayerChooserPrivate *priv = OL_PLAYER_CHOOSER_GET_PRIVATE (window);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->remember_button)))
  {
    OlConfig *config = ol_config_get_instance ();
    ol_config_set_string (config,
                          "General",
                          "startup-player",
                          cmd);
  }
}
