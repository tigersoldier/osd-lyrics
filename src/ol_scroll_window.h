/**
 * @file   ol_scroll_window.h
 * @author Sarlmol Apple <sarlmolapple@gmail.com>
 * @date   Mon May 17 14:16:52 2010
 * 
 * @brief  The definition of an OlOsdWindow widget
 * 
 * Copyright (C) 2009  Sarlmol Apple
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
#ifndef __SCROLL_WINDOW_H_
#define __SCROLL_WINDOW_H_

#include <gtk/gtk.h>
#include <ol_debug.h>

#define OL_SCROLL_WINDOW(obj)                   GTK_CHECK_CAST (obj, ol_scroll_window_get_type (), OlScrollWindow)
#define OL_SCROLL_WINDOW_CLASS(klass)           GTK_CHECK_CLASS_CAST (klass, ol_scroll_window_get_type (), OlScrollWindowClass)
#define OL_IS_SCROLL_WINDOW(obj)                GTK_CHECK_TYPE (obj, ol_scroll_window_get_type ())
#define OL_SCROLL_WINDOW_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), ol_scroll_window_get_type (), OlScrollWindowClass))

typedef struct _OlScrollWindow                  OlScrollWindow;
typedef struct _OlScrollWindowClass             OlScrollWindowClass;

struct _OlScrollWindow
{
  /*basic*/
  GtkWindow widget;
  double percentage;
  GPtrArray *whole_lyrics;
  gint whole_lyrics_len;
  gint current_lyric_id;
};


struct _OlScrollWindowClass
{
  GtkWindowClass parent_class;
};

GtkType ol_scroll_window_get_type (void);

/** 
 * @brief create a new Scroll Window. 
 * To destroy the Scroll Window, use g_object_unref
 */

GtkWidget* ol_scroll_window_new (void);

/** 
 * @brief Set the lyric of certain line
 * If a line of lyric is set, it will changes to the lyric.
 * @param scroll An OlScrollWindow
 * @param lyric_id The lyric_line which is currenty being displayed. -1  means the line has no lyric currently.
 */
void ol_scroll_window_set_lyric (OlScrollWindow *scroll, const int lyric_id);
/** 
 * @brief Set the whole lyric of a song
 * If music changes,the whole lyrics of window will be changed.
 * @param scroll An OlScrollWindow
 * @param whole_lyrics The lyrics of a song. NULL means the line has no lyric currently.
 * @param whole_lyrics_len The lyrics number of a song
 */
void ol_scroll_window_set_whole_lyrics(OlScrollWindow *scroll, GPtrArray *whole_lyrics, gint whole_lyrics_len);
/** 
 * @brief Sets the progress of the current lyric line
 * @param scroll An OlScrollWindow
 * @param percentage The width percentage of the left part whose color is changed
 */
void ol_scroll_window_set_current_percentage (OlScrollWindow *scroll, double percentage);


/** 
 * @brief Gets the current line number
 * The current line is the lyric which is playing currently. 
 * @param scroll An OlScrollWindow
 * @param line The line number of the current lyric, can be 0 or 1. 0 is the upper line and 1 is the lower
 */
int ol_scroll_window_get_current_lyric_id (OlScrollWindow *scroll);
/** 
 * @brief Sets the font family for an SCROLL Window
 * 
 * @param scroll An OlScrollWindow;
 * @param font_name Font family, must not be NULL. The font_name contains style and
 *        size information. Should be able to pass the value to
 *        pango_font_description_from_string() 
 */
void ol_scroll_window_set_font_name (OlScrollWindow *scroll,
                                     const char *font_family);
/** 
 * @brief Gets the font family for an SCROLL Window
 * 
 * @param scroll An OlScrollWindow
 * @return The font name, see the comment of ol_scroll_window_set_font_name
 */
const char* ol_scroll_window_get_font_name (OlScrollWindow *scroll);

/** 
 * Sets the opacity of the background
 * 
 * @param scroll 
 * @param opacity The opacity of the background. 0 being fully transparent
 *                and 1 meansfully opaque.
 * 
 */
void ol_scroll_window_set_bg_opacity (OlScrollWindow *scroll,
                                      double opacity);

double ol_scroll_window_get_bg_opacity (OlScrollWindow *scroll);
#endif /* __OL_SCROLL_WINDOW_H__ */
