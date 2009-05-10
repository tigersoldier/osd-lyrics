#ifndef __OSD_WINDOW_H_
#define __OSD_WINDOW_H_

#include <gtk/gtk.h>

#define OSD_WINDOW(obj)                  GTK_CHECK_CAST (obj, osd_window_get_type (), OsdWindow)
#define OSD_WINDOW_CLASS(klass)          GTK_CHECK_CLASS_CAST (klass, osd_window_get_type (), OsdWindowClass)
#define IS_OSD_WINDOW(obj)               GTK_CHECK_TYPE (obj, osd_window_get_type ())
#define OSD_WINDOW_MAX_LINE_COUNT        2

typedef struct _OsdWindow                OsdWindow;
typedef struct _OsdWindowClass           OsdWindowClass;

struct _OsdWindow
{
  GtkWidget widget;
  GdkWindow *event_window;
  GdkWindow *bg_window;
  GdkScreen *screen;
  gchar *lyrics[OSD_WINDOW_MAX_LINE_COUNT];
  double line_alignment[OSD_WINDOW_MAX_LINE_COUNT];
  guint current_line;           /* which line is playing currently */
  double current_percentage;
};

struct _OsdWindowClass
{
  GtkWidgetClass parent_class;
};

GtkType osd_window_get_type (void);

/** 
 * @brief Creates a new OSD Window.
 */
GtkWidget* osd_window_new (void);

/** 
 * @brief Sets the alignment of the OSD Window, respect to the screen
 *
 * @param osd an OsdWindow
 * @param xalign the horizontal position of the OSD Window. 0.0 is left aligned, 1.0 is right aligned.
 * @param yalign the vertical position of the OSD Window. 0.0 is top aligned, 1.0 is bottom aligned.
 */
void osd_window_set_alignment (OsdWindow *osd, float xalign, float yalign);

/** 
 * @brief Gets the alignment of the OSD Window, respect to the screen
 * 
 * @param osd 
 * @param xalign 
 * @param yalign 
 */
void osd_window_get_alignment (OsdWindow *osd, float *xalign, float *yalign);

void osd_window_resize (OsdWindow *osd, gint width, gint height);
void osd_window_get_size (OsdWindow *osd, gint *width, gint *height);

void osd_window_set_locked (OsdWindow *osd, gboolean locked);
gboolean osd_window_get_locked (OsdWindow *osd);

void osd_paint (OsdWindow *widget, const char* odd_lyric, const char* even_lyric, double percentage);

void osd_window_set_current_percentage (OsdWindow *osd, double percentage);
double osd_window_get_current_percentage (OsdWindow *osd);

void osd_window_set_current_line (OsdWindow *osd, gint line);
gint osd_window_get_current_line (OsdWindow *osd);

void osd_window_set_lyric (OsdWindow *osd, gint line, const char *lyric);
void osd_window_set_line_alignment (OsdWindow *osd, gint line, double alignment);

#endif // __OSD_WINDOW_H__
