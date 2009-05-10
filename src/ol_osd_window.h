#ifndef __OSD_WINDOW_H_
#define __OSD_WINDOW_H_

#include <gtk/gtk.h>

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
  double current_percentage;
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
 * @param osd 
 * @param xalign 
 * @param yalign 
 */
void ol_osd_window_get_alignment (OlOsdWindow *osd, float *xalign, float *yalign);

void ol_osd_window_resize (OlOsdWindow *osd, gint width, gint height);
void ol_osd_window_get_size (OlOsdWindow *osd, gint *width, gint *height);

void ol_osd_window_set_locked (OlOsdWindow *osd, gboolean locked);
gboolean ol_osd_window_get_locked (OlOsdWindow *osd);

void ol_osd_paint (OlOsdWindow *widget, const char* odd_lyric, const char* even_lyric, double percentage);

void ol_osd_window_set_current_percentage (OlOsdWindow *osd, double percentage);
double ol_osd_window_get_current_percentage (OlOsdWindow *osd);

void ol_osd_window_set_current_line (OlOsdWindow *osd, gint line);
gint ol_osd_window_get_current_line (OlOsdWindow *osd);

void ol_osd_window_set_lyric (OlOsdWindow *osd, gint line, const char *lyric);
void ol_osd_window_set_line_alignment (OlOsdWindow *osd, gint line, double alignment);

#endif // __OSD_WINDOW_H__
