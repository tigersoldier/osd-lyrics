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
  GtkTable *supported_table;
  GtkTable *all_table;
  GtkExpander *supported_expander;
  GtkExpander *all_expander;
  GtkWidget *remember_button;
  GtkEntry *cmd_entry;
  GtkWidget *launch_button;
} OlPlayerChooserPrivate;

static void _destroy (GtkObject *object);
static void _set_app_table (OlPlayerChooser *window,
                            GtkTable *table,
                            GList *app_list,
                            gboolean (*filter) (GAppInfo *));
static void _set_supported_players (OlPlayerChooser *chooser,
                                    GList *supported_players);
static void _player_button_launch (GtkButton *button,
                                   GAppInfo *app_info);
static void _launch_button_clicked_cb (GtkButton *button,
                                       OlPlayerChooser *window);
static gboolean _app_command_exists (GAppInfo *app_info);
static GtkWidget *_image_from_app_info (GAppInfo *app_info);
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
  GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (button));
  if (OL_IS_PLAYER_CHOOSER (toplevel))
	{
    _remember_cmd_if_needed (OL_PLAYER_CHOOSER (toplevel),
                             g_app_info_get_commandline (app_info));
    gtk_dialog_response (GTK_DIALOG (toplevel), OL_PLAYER_CHOOSER_RESPONSE_LAUNCH);
  }
}

static gboolean
_app_command_exists (GAppInfo *app_info)
{
  ol_assert_ret (G_IS_APP_INFO (app_info), FALSE);
  const char *exec = g_app_info_get_executable (app_info);
  char *path = g_find_program_in_path (exec);
  if (path != NULL)
  {
    g_free (path);
    return TRUE;
  }
  return FALSE;
}

static GtkWidget
*_image_from_app_info (GAppInfo *app_info)
{
  GIcon *icon = g_app_info_get_icon (app_info);
  GtkWidget *image = NULL;
  if (icon)
  {
    image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
  }
  else
  {
    const char *icon_name = g_app_info_get_executable (app_info);
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
    if (!gtk_icon_theme_has_icon (icon_theme, icon_name))
      icon_name = "media-playback-start";
    image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_DIALOG);
  }
  gtk_image_set_pixel_size (GTK_IMAGE (image), 64);
  return image;
}

static void
_set_app_table (OlPlayerChooser *window,
                GtkTable *table,
                GList *app_list,
                gboolean (*filter) (GAppInfo *))
{
  ol_assert (GTK_IS_TABLE (table));
  int count = 0;
  GList *app = NULL;
  OlPlayerChooserPrivate *priv = OL_PLAYER_CHOOSER_GET_PRIVATE (window);
  for (app = app_list; app != NULL;)
  {
    if (filter == NULL || filter (app->data))
    {
      count++;
      app = g_list_next (app);
    }
    else
    {
      g_object_unref (G_OBJECT (app->data));
      if (app_list == app)
        app_list = g_list_next (app);
      app = g_list_delete_link (app, app);
    }
  }
  if (count == 0) return;
  app_list = g_list_sort (app_list, (GCompareFunc)ol_app_info_cmp);
  guint n_rows, n_columns;
  n_columns = priv->n_columns;
  n_rows = (count - 1) / n_columns + 1;
  gtk_table_resize (table, n_rows, n_columns);
  guint row = 0, col = 0;
  for (app = app_list; app != NULL; app = g_list_next (app))
  {
    GAppInfo *app_info = app->data;
    GtkWidget *button, *frame, *vbox, *image, *label;
    image = _image_from_app_info (app_info);

    label = gtk_label_new (g_app_info_get_display_name (app_info));
    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
    gtk_widget_set_size_request (label, 80, -1);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), image, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

    frame = gtk_aspect_frame_new (NULL, 0.5, 0.5, 1.0, FALSE);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

    button = gtk_button_new ();
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text (button, g_app_info_get_display_name (app_info));
    gtk_container_add (GTK_CONTAINER (button), frame);
    g_signal_connect (button,
                      "clicked",
                      G_CALLBACK (_player_button_launch),
                      app_info);

    gtk_table_attach_defaults (table, button, col, col + 1, row, row + 1);
    row += (col + 1) / n_columns;
    col = (col + 1) % n_columns;
  }
  gtk_widget_show_all (GTK_WIDGET (table));
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
    _set_app_table (window,
                    priv->supported_table,
                    supported_players,
                    _app_command_exists);
    gtk_expander_set_expanded (priv->supported_expander, TRUE);
    gtk_expander_set_expanded (priv->all_expander, FALSE);
  }
  _set_app_table (window,
                  priv->all_table,
                  g_app_info_get_all_for_type ("audio/mp3"),
                  g_app_info_should_show);
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
