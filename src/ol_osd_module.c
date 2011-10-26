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
#include <string.h>
#include "ol_metadata.h"
#include "ol_config.h"
#include "ol_player.h"
#include "ol_osd_window.h"
#include "ol_osd_toolbar.h"
#include "ol_osd_module.h"
#include "ol_lrc.h"
#include "ol_stock.h"
#include "ol_menu.h"
#include "ol_app.h"
#include "ol_utils.h"
#include "ol_debug.h"

const int MESSAGE_DURATION_MS = 3000;
typedef struct _OlOsdModule OlOsdModule;

struct OlLrc;

struct _OlOsdModule
{
  OlPlayer *player;
  OlMetadata *metadata;
  gint lrc_id;
  gint lrc_next_id;
  gint current_line;
  gint line_count;
  OlLrc *lrc;
  gboolean display;
  OlOsdWindow *window;
  OlOsdToolbar *toolbar;
  guint message_source;
  gulong config_signal;
};

/** interfaces */
static OlOsdModule* ol_osd_module_new (struct OlDisplayModule *module,
                                       OlPlayer *player);
static void ol_osd_module_free (struct OlDisplayModule *module);
static void _metadata_changed_cb (OlPlayer *player,
                                  OlOsdModule *module);
static void _update_metadata (OlOsdModule *module);
static gboolean _advance_to_nonempty_lyric (OlLrcIter *iter);

static void ol_osd_module_set_played_time (struct OlDisplayModule *module,
                                           guint64 played_time);
static void ol_osd_module_set_lrc (struct OlDisplayModule *module,
                                   OlLrc *lrc_file);
static void ol_osd_module_set_message (struct OlDisplayModule *module,
                                       const char *message,
                                       int duration_ms);
static void ol_osd_module_search_message (struct OlDisplayModule *module,
                                          const char *message);
static void ol_osd_module_search_fail_message (struct OlDisplayModule *module,
                                               const char *message);
static void ol_osd_module_download_fail_message (struct OlDisplayModule *module,
                                                 const char *message);
static void ol_osd_module_clear_message (struct OlDisplayModule *module);

/** internal functions */

/** 
 * @brief Gets the real lyric of the given lyric
 * A REAL lyric is the nearest lyric to the given lyric, whose text is not empty
 * If the given lyric text is not empty, the given lyric is a real lyric
 * If not real lyric available, returns NULL
 * @param lrc An const struct OlLrcItem
 * 
 * @return The real lyric of the lrc. returns NULL if not available
 */
static void ol_osd_module_update_next_lyric (OlOsdModule *osd,
                                             OlLrcIter *iter);
static void ol_osd_module_init_osd (OlOsdModule *osd);
static void config_change_handler (OlConfig *config,
                                   gchar *group,
                                   gchar *name,
                                   gpointer userdata);
static void ol_osd_moved_handler (OlOsdWindow *osd, gpointer data);
static void ol_osd_resize_handler (OlOsdWindow *osd, gpointer data);
static gboolean ol_osd_button_release (OlOsdWindow *osd,
                                       GdkEventButton *event,
                                       gpointer data);
static void ol_osd_scroll (OlOsdWindow *osd,
                           GdkEventScroll *event,
                           gpointer data);
static gboolean hide_message (OlOsdModule *osd);
static void clear_lyrics (OlOsdModule *osd);

static void
ol_osd_moved_handler (OlOsdWindow *osd, gpointer data)
{
  ol_log_func ();
  OlConfig *config = ol_config_get_instance ();
  int x, y;
  ol_osd_window_get_pos (osd, &x, &y);
  ol_config_set_int_no_emit (config, "OSD", "x", x);
  ol_config_set_int_no_emit (config, "OSD", "y", y);
}

static void
ol_osd_resize_handler (OlOsdWindow *osd, gpointer data)
{
  ol_log_func ();
  OlConfig *config = ol_config_get_instance ();
  int width = ol_osd_window_get_width (osd);
  ol_config_set_int_no_emit (config, "OSD", "width", width);
}

static gboolean
ol_osd_button_release (OlOsdWindow *osd,
                       GdkEventButton *event,
                       gpointer data)
{
  if (event->button == 3)
  {
    gtk_menu_popup (GTK_MENU (ol_menu_get_popup ()),
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    event->button,
                    event->time);
    return TRUE;
  }
  return FALSE;
}

static void
ol_osd_scroll (OlOsdWindow *osd,
               GdkEventScroll *event,
               gpointer data)
{
  int doffset = 0;
  if (event->direction == GDK_SCROLL_DOWN ||
      event->direction == GDK_SCROLL_RIGHT)
    doffset = -200;
  else if (event->direction == GDK_SCROLL_UP ||
           event->direction == GDK_SCROLL_LEFT)
    doffset = 200;
  ol_app_adjust_lyric_offset (doffset);
}

static void
ol_osd_module_update_next_lyric (OlOsdModule *osd, OlLrcIter *iter)
{
  if (osd->line_count == 1)
  {
    osd->lrc_next_id = -1;
    return;
  }
  if (ol_lrc_iter_next (iter))
    _advance_to_nonempty_lyric (iter);
  gint id;
  const char *text = NULL;
  if (ol_lrc_iter_is_valid (iter))
  {
    id = ol_lrc_iter_get_id (iter);
    text = ol_lrc_iter_get_text (iter);
  }
  else
  {
    id = -1;
    text = "";
  }
  if (osd->lrc_next_id != id)
  {
    int next_line = 1 - osd->current_line;
    osd->lrc_next_id = id;
    ol_osd_window_set_lyric (osd->window, next_line, text);
    ol_osd_window_set_percentage (osd->window, next_line, 0.0);
  }
}

static void
config_change_handler (OlConfig *config,
                       gchar *group,
                       gchar *name,
                       gpointer userdata)
{
  ol_debugf ("%s:[%s]%s\n", __FUNCTION__, group, name);
  OlOsdModule *osd = (OlOsdModule*) userdata;
  if (osd == NULL)
    return;
  OlOsdWindow *window = osd->window;
  /* OlOsdWindow *osd = OL_OSD_WINDOW (userdata); */
  if (window == NULL || !OL_IS_OSD_WINDOW (window))
    return;
  if (strcmp (group, "OSD") != 0)
    return;
  if (strcmp (name, "locked") == 0)
  {
    ol_debugf ("  locked: %d\n", ol_config_get_bool (config, group, name));
    ol_osd_window_set_locked (window,
                              ol_config_get_bool (config, group, name));
  }
  else if (strcmp (name, "x") == 0 || strcmp (name, "y") == 0)
  {
    ol_osd_window_move (window,
                        ol_config_get_int (config, group, "x"),
                        ol_config_get_int (config, group, "y"));
  }
  else if (strcmp (name, "font-name") == 0)
  {
    gchar *font = ol_config_get_string (config, group, name);
    ol_assert (font != NULL);
    ol_osd_window_set_font_name (window, font);
    g_free (font);
  }
  else if (strcmp (name, "width") == 0)
  {
    ol_osd_window_set_width (window,
                             ol_config_get_int (config, group, name));
  }
  else if (strcmp (name, "lrc-align-0") == 0)
  {
    ol_osd_window_set_line_alignment (window, 0,
                                      ol_config_get_double (config, group, name));
  }
  else if (strcmp (name, "lrc-align-1") == 0)
  {
    ol_osd_window_set_line_alignment (window, 1,
                                      ol_config_get_double (config, group, name));
  }
  else if (strcmp (name, "active-lrc-color") == 0)
  {
    gsize len;
    char **color_str = ol_config_get_str_list (config, group, name, &len);
    ol_debugf ("len = %d\n", (int)len);
    if (len != OL_LINEAR_COLOR_COUNT) return;
    if (color_str != NULL)
    {
      OlColor *colors = ol_color_from_str_list ((const char**)color_str, NULL);
      ol_osd_window_set_active_colors (window, colors[0], colors[1], colors[2]);
      g_free (colors);
      g_strfreev (color_str);
    }
  }
  else if (strcmp (name, "inactive-lrc-color") == 0)
  {
    gsize len;
    char **color_str = ol_config_get_str_list (config, group, name, &len);
    ol_debugf ("len = %d\n", (int)len);
    if (len != OL_LINEAR_COLOR_COUNT) return;
    if (color_str != NULL)
    {
      OlColor *colors = ol_color_from_str_list ((const char**)color_str, NULL);
      ol_osd_window_set_inactive_colors (window, colors[0], colors[1], colors[2]);
      g_free (colors);
      g_strfreev (color_str);
    }
  }
  else if (strcmp (name, "line-count") == 0)
  {
    osd->line_count = ol_config_get_int (config, group, name);
    ol_osd_window_set_line_count (window, osd->line_count);
  }
  else if (strcmp (name, "visible") == 0)
  {
    gboolean visible = ol_config_get_bool (config, group, name);
    if (visible)
    {
      gtk_widget_show (GTK_WIDGET (osd->window));
    }
    else
    {
      gtk_widget_hide (GTK_WIDGET (osd->window));
    }
  }
  else if (strcmp (name, "translucent-on-mouse-over") == 0)
  {
    ol_osd_window_set_translucent_on_mouse_over (window, ol_config_get_bool (config, "OSD", name));
  }
  else if (strcmp (name, "outline-width") == 0)
  {
    ol_osd_window_set_outline_width (window, ol_config_get_int (config, group, name));
  }
  else if (strcmp (name, "osd-window-mode") == 0)
  {
    gchar *mode = ol_config_get_string (config, group, name);
    if (strcmp (mode, "dock") == 0)
      ol_osd_window_set_mode (window, OL_OSD_WINDOW_DOCK);
    else
      ol_osd_window_set_mode (window, OL_OSD_WINDOW_NORMAL);
    g_free (mode);
  }
  else if (strcmp (name, "blur-radius") == 0)
  {
    ol_osd_window_set_blur_radius (window, ol_config_get_double (config, group, name));
  }
}

static void
ol_osd_module_init_osd (OlOsdModule *osd)
{
  osd->window = OL_OSD_WINDOW (ol_osd_window_new ());
  if (osd->window == NULL)
    return;
  GtkIconTheme *icontheme = gtk_icon_theme_get_default ();
  GdkPixbuf *bg = gtk_icon_theme_load_icon (icontheme,
                                            OL_STOCK_OSD_BG,
                                            32,
                                            0,
                                            NULL);
  ol_osd_window_set_bg (osd->window, bg);
  g_object_unref (bg);
  osd->toolbar = OL_OSD_TOOLBAR (ol_osd_toolbar_new ());
  if (osd->toolbar != NULL)
  {
    gtk_container_add (GTK_CONTAINER (osd->window),
                       GTK_WIDGET (osd->toolbar));
    gtk_widget_show_all (GTK_WIDGET (osd->toolbar));
    g_object_ref (osd->toolbar);
    ol_osd_toolbar_set_player (osd->toolbar, osd->player);
  }
  osd->display = FALSE;
  OlConfig *config = ol_config_get_instance ();
  ol_assert (config != NULL);
  ol_config_set_bool (config, "OSD", "visible", TRUE);
  config_change_handler (config, "OSD", "width", osd);
  config_change_handler (config, "OSD", "osd-window-mode", osd);
  config_change_handler (config, "OSD", "visible", osd);
  config_change_handler (config, "OSD", "locked", osd);
  config_change_handler (config, "OSD", "line-count", osd);
  config_change_handler (config, "OSD", "font-name", osd);
  config_change_handler (config, "OSD", "x", osd);
  config_change_handler (config, "OSD", "lrc-align-0", osd);
  config_change_handler (config, "OSD", "lrc-align-1", osd);
  config_change_handler (config, "OSD", "active-lrc-color", osd);
  config_change_handler (config, "OSD", "inactive-lrc-color", osd);
  config_change_handler (config, "OSD", "translucent-on-mouse-over", osd);
  config_change_handler (config, "OSD", "outline-width", osd);
  config_change_handler (config, "OSD", "blur-radius", osd);
  g_signal_connect (osd->window, "moved",
                    G_CALLBACK (ol_osd_moved_handler),
                    NULL);
  g_signal_connect (osd->window, "resize",
                    G_CALLBACK (ol_osd_resize_handler),
                    NULL);
  g_signal_connect (osd->window, "button-release-event",
                    G_CALLBACK (ol_osd_button_release),
                    NULL);
  g_signal_connect (osd->window, "scroll-event",
                    G_CALLBACK (ol_osd_scroll),
                    NULL);
  osd->config_signal = g_signal_connect (config, "changed",
                                         G_CALLBACK (config_change_handler),
                                         osd);
}

static OlOsdModule*
ol_osd_module_new (struct OlDisplayModule *module,
                   OlPlayer *player)
{
  ol_log_func ();
  OlOsdModule *data = g_new (OlOsdModule, 1);
  g_object_ref (player);
  data->player = player;
  data->window = NULL;
  data->lrc = NULL;
  data->lrc_id = -1;
  data->lrc_next_id = -1;
  data->current_line = 0;
  data->message_source = 0;
  data->metadata = ol_metadata_new ();
  ol_osd_module_init_osd (data);
  g_signal_connect (player,
                    "track-changed",
                    G_CALLBACK (_metadata_changed_cb),
                    data);
  _update_metadata (data);
  return data;
}

static void
ol_osd_module_free (struct OlDisplayModule *module)
{
  ol_log_func ();
  ol_assert (module != NULL);
  OlOsdModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (priv->lrc)
  {
    g_object_unref (priv->lrc);
    priv->lrc = NULL;
  }
  if (priv->toolbar)
  {
    g_object_unref (priv->toolbar);
    priv->toolbar = NULL;
  }
  if (priv->window != NULL)
  {
    gtk_widget_destroy (GTK_WIDGET (priv->window));
    priv->window = NULL;
  }
  if (priv->message_source > 0)
  {
    g_source_remove (priv->message_source);
    priv->message_source = 0;
  }
  if (priv->metadata != NULL)
  {
    ol_metadata_free (priv->metadata);
    priv->metadata = NULL;
  }
  g_signal_handlers_disconnect_by_func (priv->player,
                                        _metadata_changed_cb,
                                        priv);
  g_object_unref (priv->player);
  priv->player = NULL;
  OlConfig *config = ol_config_get_instance ();
  g_signal_handler_disconnect (config, priv->config_signal);
  g_free (priv);
}

static void
_metadata_changed_cb (OlPlayer *player,
                      OlOsdModule *module)
{
  ol_log_func ();
  _update_metadata (module);
  
}

static void
_update_metadata (OlOsdModule *module)
{
  ol_log_func ();
  ol_assert (module != NULL);
  ol_player_get_metadata (module->player, module->metadata);
  clear_lyrics (module);
  hide_message (module);
}

static void
ol_osd_module_set_played_time (struct OlDisplayModule *module,
                               guint64 played_time)
{
  ol_assert (module != NULL);
  OlOsdModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (priv->lrc != NULL && priv->window != NULL)
  {
    OlLrcIter *iter = ol_lrc_iter_from_timestamp (priv->lrc,
                                                  played_time);
    if (_advance_to_nonempty_lyric (iter))
    {
      gint id = ol_lrc_iter_get_id (iter);
      if (id != priv->lrc_id)
      {
        if (id == priv->lrc_next_id)
        {
          /* advance to the next line */
          ol_osd_window_set_percentage (priv->window, priv->current_line, 1.0);
          priv->current_line = 1 - priv->current_line;
          priv->lrc_id = priv->lrc_next_id;
          priv->lrc_next_id = -1;
        }
        else
        {
          /* The user seeks the position or there is only 1 line in OSD window.
             Reset the lyrics. */
          priv->lrc_id = id;
          priv->current_line = 0;
          ol_osd_window_set_current_line (priv->window, 0);
          ol_osd_window_set_lyric (priv->window, priv->current_line,
                                   ol_lrc_iter_get_text (iter));
          ol_osd_module_update_next_lyric (priv, iter);
        }
      }
      gdouble percentage = ol_lrc_iter_compute_percentage (iter, played_time);
      ol_osd_window_set_current_percentage (priv->window, percentage);
      if (percentage > 0.5 && priv->lrc_next_id == -1)
        ol_osd_module_update_next_lyric (priv, iter);
    }
    else if (priv->lrc_id != -1)
    {
      clear_lyrics (priv);
    }
    ol_lrc_iter_free (iter);
  }
}

static gboolean
_advance_to_nonempty_lyric (OlLrcIter *iter)
{
  for (; ol_lrc_iter_is_valid (iter); ol_lrc_iter_next (iter))
  {
    if (!ol_is_string_empty (ol_lrc_iter_get_text (iter)))
      return TRUE;
  }
  return FALSE;
}

static void
ol_osd_module_set_lrc (struct OlDisplayModule *module, OlLrc *lrc_file)
{
  ol_log_func ();
  ol_assert (module != NULL);
  OlOsdModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (priv->lrc)
    g_object_unref (priv->lrc);
  priv->lrc = lrc_file;
  if (lrc_file)
    g_object_ref (lrc_file);
  if (lrc_file != NULL && priv->message_source != 0)
  {
    ol_osd_module_clear_message (module);
  }
  if (lrc_file == NULL && priv->message_source == 0)
  {
    clear_lyrics (priv);
  }
  /* if (lrc_file != NULL) */
  /*   module->display = TRUE; */
}

static void
ol_osd_module_set_message (struct OlDisplayModule *module,
                           const char *message,
                           int duration_ms)
{
  ol_log_func ();
  ol_assert (module != NULL);
  OlOsdModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  ol_assert (message != NULL);
  ol_assert (priv->window != NULL);
  if (priv->lrc != NULL)
    return;
  ol_debugf ("  message:%s\n", message);
  ol_osd_window_set_current_line (priv->window, 0);
  ol_osd_window_set_current_percentage (priv->window, 1.0);
  ol_osd_window_set_lyric (priv->window, 0, message);
  ol_osd_window_set_lyric (priv->window, 1, NULL);
  if (priv->message_source != 0)
    g_source_remove (priv->message_source);
  priv->message_source = g_timeout_add (duration_ms,
                                        (GSourceFunc) hide_message,
                                        (gpointer) priv);
}

static void
ol_osd_module_search_message (struct OlDisplayModule *module, const char *message)
{
  ol_osd_module_set_message (module, message, -1);
}

static void
ol_osd_module_search_fail_message (struct OlDisplayModule *module, const char *message)
{
  ol_osd_module_set_message (module, message, MESSAGE_DURATION_MS);
}

static void
ol_osd_module_download_fail_message (struct OlDisplayModule *module, const char *message)
{
  ol_osd_module_set_message (module, message, MESSAGE_DURATION_MS);
}

static gboolean
hide_message (OlOsdModule *osd)
{
  ol_log_func ();
  ol_assert_ret (osd != NULL, FALSE);
  if (osd->lrc != NULL)
    return FALSE;
  ol_osd_window_set_lyric (osd->window, 0, NULL);
  /* gtk_widget_hide (GTK_WIDGET (module->window)); */
  osd->message_source = 0;
  return FALSE;
}

static void
clear_lyrics (OlOsdModule *osd)
{
  ol_log_func ();
  if (osd->window != NULL && osd->message_source == 0)
  {
    osd->display = FALSE;
    /* gtk_widget_hide (GTK_WIDGET (module->window)); */
    ol_osd_window_set_lyric (osd->window, 0, NULL);
    ol_osd_window_set_lyric (osd->window, 1, NULL);
  }
  osd->current_line = 0;
  osd->lrc_id = -1;
  osd->lrc_next_id = -1;
}

static void
ol_osd_module_clear_message (struct OlDisplayModule *module)
{
  ol_log_func ();
  ol_assert (module != NULL);
  OlOsdModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (priv->message_source != 0)
  {
    g_source_remove (priv->message_source);
    hide_message (priv);
  }
  ol_debug ("  clear message done");
}

struct OlDisplayClass*
ol_osd_module_get_class ()
{
  struct OlDisplayClass *klass = ol_display_class_new ("OSD",
                                                       (OlDisplayInitFunc) ol_osd_module_new,
                                                       ol_osd_module_free);
  klass->clear_message = ol_osd_module_clear_message;
  klass->download_fail_message = ol_osd_module_download_fail_message;
  klass->search_fail_message = ol_osd_module_search_fail_message;
  klass->search_message = ol_osd_module_search_message;
  klass->set_lrc = ol_osd_module_set_lrc;
  klass->set_message = ol_osd_module_set_message;
  klass->set_played_time = ol_osd_module_set_played_time;
  return klass;
}
