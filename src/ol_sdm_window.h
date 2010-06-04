/**
 * @file   ol_classic_window.h
 * @author Sarlmol Apple <sarlmolapple@gmail.com>
 * @date   Mon May 17 14:16:52 2010
 * 
 * @brief  The definition of an OlOsdWindow widget
 * 
 * Copyright (C) 2009  Tiger Soldier
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 */
#ifndef __CLASSIC_WINDOW_H_
#define __CLASSIC_WINDOW_H_

#include <gtk/gtk.h>
#include <ol_debug.h>

#define OL_CLASSIC_WINDOW(obj)                   GTK_CHECK_CAST (obj, ol_classic_window_get_type (), OlClassicWindow)
#define OL_CLASSIC_WINDOW_CLASS(klass)           GTK_CHECK_CLASS_CAST (klass, ol_classic_window_get_type (), OlClassicWindowClass)
#define OL_IS_CLASSIC_WINDOW(obj)                GTK_CHECK_TYPE (obj, ol_classic_window_get_type ())
#define OL_CLASSIC_WINDOW_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), ol_classic_window_get_type (), OlClassicWindowClass))

typedef struct _OlClassicWindow                  OlClassicWindow;
typedef struct _OlClassicWindowClass             OlClassicWindowClass;

struct _OlClassicWindow
{
  /*basic*/
  GtkWindow widget;
  GPtrArray *paint_lyrics;
  double percentage;
  GPtrArray *whole_lyrics;
  gint whole_lyrics_len;
  gint current_lyric_id;
};


struct _OlClassicWindowClass
{
  GtkWindowClass parent_class;
};

GtkType ol_classic_window_get_type (void);

/** 
 * @brief create a new Classic Window. 
 * To destroy the Classic Window, use g_object_unref
 */

GtkWidget* ol_classic_window_new (void);

/** 
 * @brief Set the lyric of certain line
 * If a line of lyric is set, it will changes to the lyric.
 * @param classic An OlClassicWindow
 * @param line The line whose lyric will be set. Can be 0 or 1.
 * @param lyric The lyric of the line. NULL means the line has no lyric currently.
 */
void ol_classic_window_set_lyric (OlClassicWindow *classic, const int lyric_id);
/** 
 * @brief Set the whole lyric of a song
 * If music changes,the whole lyrics of window will be changed.
 * @param classic An OlClassicWindow
 * @param whole_lyric The lyrics of a song. NULL means the line has no lyric currently.
 */
void ol_classic_window_set_whole_lyrics(OlClassicWindow *classic, GPtrArray *whole_lyrics, gint whole_lyrics_len);
/** 
 * @brief Sets the progress of the current lyric line
 * The color of the left part of the current lyric line will be changed, which makes the lyric KaraOK-like.
 * @param classic An OlClassicWindow
 * @param percentage The width percentage of the left part whose color is changed
 */
void ol_classic_window_set_current_percentage (OlClassicWindow *classic, double percentage);

/** 
 * @brief Resizes an CLASSIC window
 * 
 * @param classic An OlClassicWindow
 * @param width The width of the window
 * @param height The height of the window
 */
void ol_classic_window_resize (OlClassicWindow *classic, gint width, gint height);
/** 
 * @brief Sets the current line number
 * The current line is the lyric which is playing currently. The current lyric's color will be affected by
 * the current percentage set by ol_osd_window_set_current_percentage
 * @param osd An OlOsdWindow
 * @param line The line number of the current lyric, can be 0 or 1. 0 is the upper line and 1 is the lower
 */
void ol_osd_window_get_current_lyric_id (OlClassicWindow *classic);


#endif /* __OL_OSD_WINDOW_H__ */
