/**
 * @file   ol_osd_window.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Mon May 11 14:16:52 2009
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
#ifndef __OSD_WINDOW_H_
#define __OSD_WINDOW_H_

#include <gtk/gtk.h>
#include "ol_osd_render.h"

#define OL_OSD_WINDOW(obj)                  GTK_CHECK_CAST (obj, ol_osd_window_get_type (), OlOsdWindow)
#define OL_OSD_WINDOW_CLASS(klass)          GTK_CHECK_CLASS_CAST (klass, ol_osd_window_get_type (), OlOsdWindowClass)
#define OL_IS_OSD_WINDOW(obj)               GTK_CHECK_TYPE (obj, ol_osd_window_get_type ())
#define OL_OSD_WINDOW_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), ol_osd_window_get_type (), OlOsdWindowClass))
#define OL_OSD_WINDOW_MAX_LINE_COUNT        2

typedef struct _OlOsdWindow                 OlOsdWindow;
typedef struct _OlOsdWindowClass            OlOsdWindowClass;

enum OlOsdWindowSingals {
  OSD_INVALID_SIGNAL = 0,
  OSD_MOVED,
  OSD_SINGAL_COUNT,
};

struct _OlOsdWindow
{
  GtkWindow parent;
  GdkWindow *event_window;
  GdkWindow *osd_window;
  GdkPixbuf *bg_pixbuf;
  GdkScreen *screen;
  gchar *lyrics[OL_OSD_WINDOW_MAX_LINE_COUNT];
  double line_alignment[OL_OSD_WINDOW_MAX_LINE_COUNT];
  guint current_line;           /* which line is playing currently */
  double percentage[OL_OSD_WINDOW_MAX_LINE_COUNT];
  GdkPixmap *active_lyric_pixmap[OL_OSD_WINDOW_MAX_LINE_COUNT];
  GdkPixmap *inactive_lyric_pixmap[OL_OSD_WINDOW_MAX_LINE_COUNT];
  OlColor active_colors[OL_LINEAR_COLOR_COUNT];
  OlColor inactive_colors[OL_LINEAR_COLOR_COUNT];
  GdkRectangle lyric_rects[OL_OSD_WINDOW_MAX_LINE_COUNT];
  GdkPixmap *shape_pixmap;
  OlOsdRenderContext *render_context;
  guint line_count;
  gboolean translucent_on_mouse_over;
  
};

struct _OlOsdWindowClass
{
  GtkWindowClass parent_class;
  guint signals[OSD_SINGAL_COUNT];
};

GtkType ol_osd_window_get_type (void);

/** 
 * @brief Creates a new OSD Window.
 * To destroy the OSD Window, use g_object_unref
 */
GtkWidget* ol_osd_window_new (void);

/** 
 * @brief Sets the alignment of the OSD Window, respect to the screen
 * 
 * @param osd an OlOsdWindow
 * @param xalign the horizontal position of the OSD Window. 0.0 is left aligned, 1.0 is right aligned.
 * @param yalign the vertical position of the OSD Window. 0.0 is top aligned, 1.0 is bottom aligned.
 */
void ol_osd_window_set_alignment (OlOsdWindow *osd, double xalign, double yalign);

/** 
 * @brief Gets the alignment of the OSD Window, respect to the screen
 * 
 * @param osd an OlOsdWindow
 * @param xalign return location of the horizontal position of the OSD Window, or NULL
 * @param yalign return location of the vertical position of the OSD Window, or NULL
 */
void ol_osd_window_get_alignment (OlOsdWindow *osd, double *xalign, double *yalign);

/** 
 * @brief Gets the alignment of the OSD Window, respect to the screen
 * 
 * @param osd An OlOsdWindow
 * @param xalign the horizontal position of the OSD Window. 0.0 is left aligned, 1.0 is right aligned.
 * @param yalign the vertical position of the OSD Window. 0.0 is top aligned, 1.0 is bottom aligned.
 */
void ol_osd_window_get_alignment (OlOsdWindow *osd, double *xalign, double *yalign);

/** 
 * @brief Resizes an OSD window
 * 
 * @param osd An OlOsdWindow
 * @param width The width of the window
 * @param height The height of the window
 */
void ol_osd_window_resize (OlOsdWindow *osd, gint width, gint height);
/** 
 * @brief Sets witdh of an OSD window
 * 
 * @param osd An OlOsdWindow
 * @param width The width of the window
 */
void ol_osd_window_set_width (OlOsdWindow *osd, gint width);
/** 
 * @brief Gets the size of an OSD window
 * 
 * @param osd An OlOsdWindow
 * @param width The width of the window, can be NULL
 * @param height The height of the window, can be NULL
 */
void ol_osd_window_get_osd_size (OlOsdWindow *osd, gint *width, gint *height);

/** 
 * @brief Sets whether an OSD window is locked
 * If an OSD window is locked, it can neither be moved by mouse, nor receive the mouse event.
 * Mouse events on it will be forwarded to the window below it.
 * If an OSD window is unlocked, it can be moved by mouse, just move your pointer to the rectangle
 * of the OSD window and drag it.
 * @param osd An OlOsdWindow
 * @param locked Whether the OSD window is locked or not
 */
void ol_osd_window_set_locked (OlOsdWindow *osd, gboolean locked);
/** 
 * @brief Gets whether an OSD window is locked
 * 
 * @param osd An OldOsdWindow
 * 
 * @return Whether the OSD window is locked or not
 */
gboolean ol_osd_window_get_locked (OlOsdWindow *osd);

/** 
 * @brief Sets the progress of the given lyric line
 * The color of the left part of the given lyric line will be changed, which makes the lyric KaraOK-like.
 * @param osd An OlOsdWindow
 * @param line The line of lyric
 * @param percentage The width percentage of the left part whose color is changed
 */
void ol_osd_window_set_percentage (OlOsdWindow *osd, gint line, double percentage);
/** 
 * @brief Sets the progress of the current lyric line
 * The color of the left part of the current lyric line will be changed, which makes the lyric KaraOK-like.
 * @param osd An OlOsdWindow
 * @param percentage The width percentage of the left part whose color is changed
 */
void ol_osd_window_set_current_percentage (OlOsdWindow *osd, double percentage);
/** 
 * @brief Gets the progress of the current lyric line
 * 
 * @param osd An OlOsdWindow
 * 
 * @return The width percentage of the left part whose color is changed
 */
double ol_osd_window_get_current_percentage (OlOsdWindow *osd);

/** 
 * @brief Sets the current line number
 * The current line is the lyric which is playing currently. The current lyric's color will be affected by
 * the current percentage set by ol_osd_window_set_current_percentage
 * @param osd An OlOsdWindow
 * @param line The line number of the current lyric, can be 0 or 1. 0 is the upper line and 1 is the lower
 */
void ol_osd_window_set_current_line (OlOsdWindow *osd, gint line);
/** 
 * @brief Gets the current line number
 * 
 * @param osd An OlOsdWindow
 * 
 * @return The line number of the current lyric.
 */
gint ol_osd_window_get_current_line (OlOsdWindow *osd);

/** 
 * @brief Set the lyric of certain line
 * If a line of lyric is set, it will changes to the lyric.
 * @param osd An OlOsdWindow
 * @param line The line whose lyric will be set. Can be 0 or 1.
 * @param lyric The lyric of the line. NULL means the line has no lyric currently.
 */
void ol_osd_window_set_lyric (OlOsdWindow *osd, gint line, const char *lyric);

/** 
 * @brief Set the horizontal alignment of a line
 * 
 * @param osd An OlOsdWindow
 * @param line The line that will be set. Can be 0 or 1.
 * @param alignment The alignment of the line, in the range of [0, 1].
 *                  0 means left aligned, 0.5 means center aligned, 1.0 means right aligned.
 */
void ol_osd_window_set_line_alignment (OlOsdWindow *osd, gint line, double alignment);

/** 
 * @brief Sets the font family for an OSD Window
 * 
 * @param osd An OlOsdWindow;
 * @param font_family Font family, must not be NULL
 */
void ol_osd_window_set_font_family (OlOsdWindow *osd,
                                    const char *font_family);
/** 
 * @brief Gets the font family for an OSD Window
 * 
 * @param osd An OlOsdWindow
 * @return The font family of the context, must be freed by g_free
 */
char* ol_osd_window_get_font_family (OlOsdWindow *osd);

/** 
 * @brief Sets the font size for an OSD Window
 * 
 * @param osd An OlOsdWindow;
 * @param font_size Font size, must be positive
 */
void ol_osd_window_set_font_size (OlOsdWindow *osd,
                                  const double font_size);

/** 
 * @brief Gets the font size for an OSD Window
 * 
 * @param osd An OlOsdWindow;
 * 
 * @return The font size for the context
 */
double ol_osd_window_get_font_size (OlOsdWindow *osd);


/** 
 * @brief Sets the outline width
 * 
 * @param osd An OSD window
 * @param width Outline width, must be positive
 */
void ol_osd_window_set_outline_width (OlOsdWindow *osd,
                                      const int width);

/** 
 * @brief Gets the outline width for an OSD Window
 * 
 * @param osd An OSD Window;
 * 
 * @return The outline width for the context
 */
int ol_osd_window_get_outline_width (OlOsdWindow *osd);

/** 
 * @brief Sets the color of active lyrics
 * Active lyric is the played part of the lyric
 * @param osd An OlOsdWindow
 * @param top_color The color of the top part of the lyrics
 * @param middle_color The color of the middle part of the lyrics
 * @param bottom_color The color of the bottom part of the lyrics
 */
void ol_osd_window_set_active_colors (OlOsdWindow *osd,
                                      OlColor top_color,
                                      OlColor middle_color,
                                      OlColor bottom_color);

/** 
 * @brief Sets the color of inactive lyrics
 * Inactive lyric is the lyric to be played
 * @param osd An OlOsdWindow
 * @param top_color The color of the top part of the lyrics
 * @param middle_color The color of the middle part of the lyrics
 * @param bottom_color The color of the bottom part of the lyrics
 */
void ol_osd_window_set_inactive_colors (OlOsdWindow *osd,
                                        OlColor top_color,
                                        OlColor middle_color,
                                        OlColor bottom_color);

/** 
 * @brief Sets the number of lyric lines to be displayed
 * 
 * @param osd An OlOsdWindow
 * @param line_count number of lines, in the range of [1,2]
 */
void ol_osd_window_set_line_count (OlOsdWindow *osd,
                                   guint line_count);
/** 
 * @brief Sets the number of lyric lines to be displayed
 * 
 * @param osd An OlOsdWindow
 * @return number of lines, in the range of [1,2]
 */
guint ol_osd_window_get_line_count (OlOsdWindow *osd);

/** 
 * @brief Sets whether the OSD window will be translucent when pointer is over it
 * 
 * @param osd An OlOsdWindow
 * @param value whether the osd will be translucent
 */
void ol_osd_window_set_translucent_on_mouse_over (OlOsdWindow *osd,
                                                  gboolean value);
/** 
 * @brief Gets whether the OSD window will be translucent when pointer is over it
 * 
 * @param osd An OlOsdWindow
 * @return whether the osd will be translucent
 */
gboolean ol_osd_window_get_translucent_on_mouse_over (OlOsdWindow *osd);

/** 
 * @brief Sets the background of the OSD window
 *
 * The background will display when the OSD Window is unlocked and
 * with user's mouse on it.
 *
 * @param osd 
 * @param bg The new background. The OSD window won't increase its ref
 *           count, but unref the old one. So if you want to keep the
 *           copy of the pixbuf, you need to ref it manually.
 * 
 */
void ol_osd_window_set_bg (OlOsdWindow *osd, GdkPixbuf *bg);
#endif // __OSD_WINDOW_H__
