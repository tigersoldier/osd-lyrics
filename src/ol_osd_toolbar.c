/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009-2011  Tiger Soldier <tigersoldi@gmail.com>
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
#include "ol_osd_toolbar.h"
#include "ol_image_button.h"
#include "ol_stock.h"
#include "ol_debug.h"

#define OL_OSD_TOOLBAR_GET_PRIVATE(obj)   (G_TYPE_INSTANCE_GET_PRIVATE \
                                            ((obj),                     \
                                             ol_osd_toolbar_get_type (),  \
                                             OlOsdToolbarPriv))
typedef struct _OlOsdToolbarPriv OlOsdToolbarPriv;
struct _OlOsdToolbarPriv
{
  struct OlPlayer *player;
  enum OlPlayerStatus status;
};

G_DEFINE_TYPE (OlOsdToolbar, ol_osd_toolbar, GTK_TYPE_ALIGNMENT);

enum {
  BTN_PLAY = 0,
  BTN_PAUSE,
  BTN_STOP,
  BTN_PREV,
  BTN_NEXT,
};

struct ButtonSpec
{
  const char *stock;
  void (*handler) (GtkButton *button, OlOsdToolbar *toolbar);
};

static void _player_control (struct OlPlayer *player,
                             enum OlPlayerCapacity capacity,
                             gboolean (*cmd) (struct OlPlayer *player));
static void _play_clicked (GtkButton *button, OlOsdToolbar *toolbar);
static void _pause_clicked (GtkButton *button, OlOsdToolbar *toolbar);
static void _stop_clicked (GtkButton *button, OlOsdToolbar *toolbar);
static void _prev_clicked (GtkButton *button, OlOsdToolbar *toolbar);
static void _next_clicked (GtkButton *button, OlOsdToolbar *toolbar);
static GtkButton *_add_button (OlOsdToolbar *toolbar,
                               const struct ButtonSpec *btn_spec);
static void _update_capacity (OlOsdToolbar *toolbar,
                              int capacity);

const static struct ButtonSpec btn_spec[] = {
  {OL_STOCK_OSD_PLAY, _play_clicked},
  {OL_STOCK_OSD_PAUSE, _pause_clicked},
  {OL_STOCK_OSD_STOP, _stop_clicked},
  {OL_STOCK_OSD_PREV, _prev_clicked},
  {OL_STOCK_OSD_NEXT, _next_clicked},
};

static void
_player_control (struct OlPlayer *player,
                 enum OlPlayerCapacity capacity,
                 gboolean (*cmd) (struct OlPlayer *player))
{
  if (player != NULL &&
      (ol_player_get_capacity (player) & capacity))
  {
    cmd (player);
  }
}

static void
_play_clicked (GtkButton *button, OlOsdToolbar *toolbar)
{
  ol_assert (toolbar != NULL);
  OlOsdToolbarPriv *priv = OL_OSD_TOOLBAR_GET_PRIVATE (toolbar);
  _player_control (priv->player,
                   OL_PLAYER_PLAY,
                   ol_player_play);
}

static void
_pause_clicked (GtkButton *button, OlOsdToolbar *toolbar)
{
  ol_assert (toolbar != NULL);
  OlOsdToolbarPriv *priv = OL_OSD_TOOLBAR_GET_PRIVATE (toolbar);
  _player_control (priv->player,
                   OL_PLAYER_PAUSE,
                   ol_player_pause);
}

static void
_stop_clicked (GtkButton *button, OlOsdToolbar *toolbar)
{
  ol_assert (toolbar != NULL);
  OlOsdToolbarPriv *priv = OL_OSD_TOOLBAR_GET_PRIVATE (toolbar);
  _player_control (priv->player,
                   OL_PLAYER_STOP,
                   ol_player_stop);
}

static void
_prev_clicked (GtkButton *button, OlOsdToolbar *toolbar)
{
  ol_assert (toolbar != NULL);
  OlOsdToolbarPriv *priv = OL_OSD_TOOLBAR_GET_PRIVATE (toolbar);
  _player_control (priv->player,
                   OL_PLAYER_PREV,
                   ol_player_prev);
}

static void
_next_clicked (GtkButton *button, OlOsdToolbar *toolbar)
{
  ol_assert (toolbar != NULL);
  OlOsdToolbarPriv *priv = OL_OSD_TOOLBAR_GET_PRIVATE (toolbar);
  _player_control (priv->player,
                   OL_PLAYER_NEXT,
                   ol_player_next);
}

static GtkButton *
_add_button (OlOsdToolbar *toolbar,
             const struct ButtonSpec *btn_spec)
{
  OlImageButton *btn = OL_IMAGE_BUTTON (ol_image_button_new ());
  GtkIconTheme *icontheme = gtk_icon_theme_get_default ();
  GtkIconInfo *info = gtk_icon_theme_lookup_icon (icontheme,
                                                  btn_spec->stock,
                                                  16,
                                                  0);
  
  GdkPixbuf *image = gdk_pixbuf_new_from_file (gtk_icon_info_get_filename (info),
                                               NULL);
  gtk_icon_info_free (info);
  ol_image_button_set_pixbuf (btn, image);
  g_signal_connect (btn,
                    "clicked",
                    G_CALLBACK (btn_spec->handler),
                    toolbar);
  gtk_box_pack_start (GTK_BOX (toolbar->center_box), GTK_WIDGET (btn),
                      FALSE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (btn));
  return GTK_BUTTON (btn);
}

static void
_update_capacity (OlOsdToolbar *toolbar,
                  int capacity)
{
  ol_assert (OL_IS_OSD_TOOLBAR (toolbar));
  gtk_widget_set_sensitive (GTK_WIDGET (toolbar->play_button),
                            capacity & OL_PLAYER_PLAY);
  gtk_widget_set_sensitive (GTK_WIDGET (toolbar->pause_button),
                            capacity & OL_PLAYER_PAUSE);
  gtk_widget_set_sensitive (GTK_WIDGET (toolbar->stop_button),
                            capacity & OL_PLAYER_STOP);
  gtk_widget_set_sensitive (GTK_WIDGET (toolbar->prev_button),
                            capacity & OL_PLAYER_PREV);
  gtk_widget_set_sensitive (GTK_WIDGET (toolbar->next_button),
                            capacity & OL_PLAYER_NEXT);
}

static void
ol_osd_toolbar_class_init (OlOsdToolbarClass *klass)
{
  g_type_class_add_private (klass, sizeof (OlOsdToolbarPriv));
}

static void
ol_osd_toolbar_init (OlOsdToolbar *toolbar)
{
  OlOsdToolbarPriv *priv = OL_OSD_TOOLBAR_GET_PRIVATE (toolbar);
  gtk_alignment_set (GTK_ALIGNMENT (toolbar), 0.5, 0.5, 0.0, 0.0);
  toolbar->center_box = GTK_HBOX (gtk_hbox_new (FALSE, 0));
  gtk_container_add (GTK_CONTAINER (toolbar), GTK_WIDGET (toolbar->center_box));
  
  toolbar->prev_button = _add_button (toolbar, &btn_spec[BTN_PREV]);
  toolbar->play_button = _add_button (toolbar, &btn_spec[BTN_PLAY]);
  toolbar->pause_button = _add_button (toolbar, &btn_spec[BTN_PAUSE]);
  toolbar->stop_button = _add_button (toolbar, &btn_spec[BTN_STOP]);
  toolbar->next_button = _add_button (toolbar, &btn_spec[BTN_NEXT]);

  priv->player = NULL;
  ol_osd_toolbar_set_status (toolbar, OL_PLAYER_UNKNOWN);
  _update_capacity (toolbar, 0);
}

GtkWidget *
ol_osd_toolbar_new (void)
{
  OlOsdToolbar *toolbar;
  toolbar = g_object_new (ol_osd_toolbar_get_type (), NULL);
  return GTK_WIDGET (toolbar);
}

void
ol_osd_toolbar_set_player (OlOsdToolbar *toolbar,
                           struct OlPlayer *player)
{
  ol_assert (OL_IS_OSD_TOOLBAR (toolbar));
  OlOsdToolbarPriv *priv = OL_OSD_TOOLBAR_GET_PRIVATE (toolbar);
  priv->player = player;
  int capacity = 0;
  if (player != NULL)
    capacity = ol_player_get_capacity (player);
  _update_capacity (toolbar, capacity);
  enum OlPlayerStatus status = OL_PLAYER_UNKNOWN;
  if (capacity & OL_PLAYER_STATUS)
    status= ol_player_get_status (player);
  ol_osd_toolbar_set_status (toolbar, status);
}

void
ol_osd_toolbar_set_status (OlOsdToolbar *toolbar,
                           enum OlPlayerStatus status)
{
  ol_log_func ();
  OlOsdToolbarPriv *priv = OL_OSD_TOOLBAR_GET_PRIVATE (toolbar);
  priv->status = status;
  /* if (status == OL_PLAYER_PLAYING) */
  /* { */
  /*   gtk_widget_show (GTK_WIDGET (toolbar->pause_button)); */
  /*   gtk_widget_hide (GTK_WIDGET (toolbar->play_button)); */
  /* } */
  /* else if (status == OL_PLAYER_PAUSED || status == OL_PLAYER_STOPPED) */
  /* { */
  /*   gtk_widget_hide (GTK_WIDGET (toolbar->pause_button)); */
  /*   gtk_widget_show (GTK_WIDGET (toolbar->play_button)); */
  /* } */
  /* else */
  /* { */
  /*   gtk_widget_show (GTK_WIDGET (toolbar->pause_button)); */
  /*   gtk_widget_show (GTK_WIDGET (toolbar->play_button)); */
  /* } */
  switch (status)
  {
  case OL_PLAYER_PLAYING:
    gtk_widget_show (GTK_WIDGET (toolbar->pause_button));
    gtk_widget_hide (GTK_WIDGET (toolbar->play_button));
    break;
  case OL_PLAYER_PAUSED:
  case OL_PLAYER_STOPPED:
    gtk_widget_hide (GTK_WIDGET (toolbar->pause_button));
    gtk_widget_show (GTK_WIDGET (toolbar->play_button));
    break;
  default:
    gtk_widget_show (GTK_WIDGET (toolbar->pause_button));
    gtk_widget_show (GTK_WIDGET (toolbar->play_button));
    break;
  }
  gtk_widget_queue_draw (GTK_WIDGET (toolbar));
}
