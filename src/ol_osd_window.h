/**
 * @file   ol_osd_window.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Mon May 11 14:16:52 2009
 * 
 * @brief  The definition of an OlOsdWindow widget
 * 
 * 
 */
#ifndef __OSD_WINDOW_H_
#define __OSD_WINDOW_H_

#include <gtk/gtk.h>
#include "ol_osd_render.h"

#define OL_OSD_WINDOW(obj)                  GTK_CHECK_CAST (obj, ol_osd_window_get_type (), OlOsdWindow)
#define OL_OSD_WINDOW_CLASS(klass)          GTK_CHECK_CLASS_CAST (klass, ol_osd_window_get_type (), OlOsdWindowClass)
#define OL_IS_OSD_WINDOW(obj)               GTK_CHECK_TYPE (obj, ol_osd_window_get_type ())
#define OL_OSD_WINDOW_MAX_LINE_COUNT        2

typedef struct _OlOsdWindow                 OlOsdWindow;
typedef struct _OlOsdWindowClass            OlOsdWindowClass;

struct _OlOsdWindow
{
  GtkWidget widget;
  GdkWindow *event_window;
  GdkWindow *bg_window;
  GdkScreen *screen;
  gchar *lyrics[OL_OSD_WINDOW_MAX_LINE_COUNT];
  double line_alignment[OL_OSD_WINDOW_MAX_LINE_COUNT];
  guint current_line;           /* which line is playing currently */
  double percentage[OL_OSD_WINDOW_MAX_LINE_COUNT];
  GdkPixmap *active_lyric_pixmap[OL_OSD_WINDOW_MAX_LINE_COUNT];
  OlOsdRenderContext *render_context;
};

struct _OlOsdWindowClass
{
  GtkWidgetClass parent_class;
};

GtkType ol_osd_window_get_type (void);

/** 
 * @brief Creates a new OSD Window.
 */
GtkWidget* ol_osd_window_new (void);

/** 
 * @brief Sets the alignment of the OSD Window, respect to the screen
 * 
 * @param osd an OlOsdWindow
 * @param xalign the horizontal position of the OSD Window. 0.0 is left aligned, 1.0 is right aligned.
 * @param yalign the vertical position of the OSD Window. 0.0 is top aligned, 1.0 is bottom aligned.
 */
void ol_osd_window_set_alignment (OlOsdWindow *osd, float xalign, float yalign);

/** 
 * @brief Gets the alignment of the OSD Window, respect to the screen
 * 
 * @param osd An OlOsdWindow
 * @param xalign the horizontal position of the OSD Window. 0.0 is left aligned, 1.0 is right aligned.
 * @param yalign the vertical position of the OSD Window. 0.0 is top aligned, 1.0 is bottom aligned.
 */
void ol_osd_window_get_alignment (OlOsdWindow *osd, float *xalign, float *yalign);

/** 
 * @brief Resizes an OSD window
 * 
 * @param osd An OlOsdWindow
 * @param width The width of the window
 * @param height The height of the window
 */
void ol_osd_window_resize (OlOsdWindow *osd, gint width, gint height);
/** 
 * @brief Gets the size of an OSD window
 * 
 * @param osd An OlOsdWindow
 * @param width The width of the window, can be NULL
 * @param height The height of the window, can be NULL
 */
void ol_osd_window_get_size (OlOsdWindow *osd, gint *width, gint *height);

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

#endif // __OSD_WINDOW_H__
