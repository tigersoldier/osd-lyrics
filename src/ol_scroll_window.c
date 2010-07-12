#include <gtk/gtkprivate.h>
#include "ol_scroll_window.h"
#include <ol_debug.h>
#include <pango/pangocairo.h>
#include <glib.h>
#include <ol_color.h>



#define OL_SCROLL_WINDOW_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE \
                                                 ((obj),                        \
                                                 ol_scroll_window_get_type (),\
                                                  OlScrollWindowPrivate))

/*************default setting******************/
static const gint DEFAULT_WIDTH = 600;
static const gint DEFAULT_HEIGHT = 600;
static const gint DEFAULT_LINE_COUNT = 20;
static const OlColor DEFAULT_ACTIVE_COLOR = {0.89,0.81,0};
static const OlColor DEFAULT_INACTIVE_COLOR = {0.98,0.92,0.84};
static const OlColor DEFAULT_BG_COLOR = {0,0,0};
static const char *DEFAULT_FONT_FAMILY = "serif";
static const double DEFAULT_FONT_SIZE = 13.0;
static const double DEFAULT_ALIGNMENT = 0.5;
static const gint DEFAULT_OUTLINE_WIDTH = 10;
/**********************************************/


typedef struct __OlScrollWindowPrivate OlScrollWindowPrivate;
struct __OlScrollWindowPrivate
{
  gint width;                      /*窗体宽度*/
  gint height;                     /*窗体高度*/
  gint line_count;                 /*歌词行数*/
  OlColor active_color;            /*播放歌词颜色*/
  OlColor inactive_color;          /*未播放歌词颜色*/
  OlColor bg_color;                /*背景颜色*/
  const gchar *font_family;        /*字体*/
  double font_size;                /*字体大小*/
  double alignment;                /*对齐方式*/ 
  gint outline_width;              /*边栏宽度*/

};


static void ol_scroll_window_init (OlScrollWindow *self);
static void ol_scroll_window_class_init (OlScrollWindowClass *klass);
static void ol_scroll_window_destroy (GtkObject *object);
static gboolean ol_scroll_window_expose (GtkWidget *widget, GdkEventExpose *event);

static void ol_scroll_window_paint (OlScrollWindow *scroll);
static int ol_scroll_window_compute_line_count (OlScrollWindow *scroll);
static int ol_scroll_window_get_font_height (OlScrollWindow *scroll);

static void ol_scroll_window_resize (OlScrollWindow *scroll);
static PangoLayout* _get_pango (OlScrollWindow *scroll, cairo_t *cr);

static gboolean ol_scroll_window_button_press (GtkWidget * widget,  GdkEventButton * event);
static gboolean ol_scroll_window_button_release (GtkWidget * widget, GdkEventButton * event);
static gboolean ol_scroll_window_motion_notify (GtkWidget * widget,  GdkEventButton * event);

static void _draw_destory_button (OlScrollWindow *scroll, cairo_t *cr, double size);

static GtkWidgetClass *parent_class = NULL;

static gboolean drag = FALSE;
static int nX = 0;
static int nY = 0;


GType
ol_scroll_window_get_type (void)
{
  static GType ol_scroll_type = 0;
  if (!ol_scroll_type)
  {
    static const GTypeInfo ol_scroll_info =
      {
        sizeof (OlScrollWindowClass),
        NULL,                   /* base_init */
        NULL,                   /* base_finalize */
        (GClassInitFunc) ol_scroll_window_class_init,
        NULL,                   /* class_finalize */
        NULL,                   /* class_data */
        sizeof (OlScrollWindow),
        16,                     /* n_preallocs */
        (GInstanceInitFunc) ol_scroll_window_init,
      };
    ol_scroll_type = g_type_register_static (GTK_TYPE_WINDOW, "OlScrollWindow",
                                          &ol_scroll_info, 0);
  }
  return ol_scroll_type;
}


GtkWidget*
ol_scroll_window_new ()
{
  OlScrollWindow *scroll;
  scroll = g_object_new (ol_scroll_window_get_type (), NULL);
  gtk_window_set_decorated (GTK_WINDOW(scroll),FALSE);
  gtk_window_set_opacity(GTK_WINDOW(scroll), 0.1);
  return GTK_WIDGET (scroll);
}

static void
ol_scroll_window_init (OlScrollWindow *self)
{
  /*basic*/
  self->percentage = 0.0;
  self->whole_lyrics = NULL;
  self->whole_lyrics_len = 0;
  self->current_lyric_id = -1;
  /*privat data*/
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (self);
  priv->width = DEFAULT_WIDTH;
  priv->height = DEFAULT_HEIGHT;
  priv->line_count = DEFAULT_LINE_COUNT;
  priv->active_color = DEFAULT_ACTIVE_COLOR;
  priv->inactive_color = DEFAULT_INACTIVE_COLOR;
  priv->bg_color = DEFAULT_BG_COLOR;
  priv->font_family = DEFAULT_FONT_FAMILY;
  priv->font_size = DEFAULT_FONT_SIZE;
  priv->alignment = DEFAULT_ALIGNMENT;
  priv->outline_width = DEFAULT_OUTLINE_WIDTH;
  /*set allocation*/
  gtk_window_resize(GTK_WINDOW(self), priv->width, priv->height);
  gtk_signal_connect (GTK_OBJECT (self), "size-allocate",
                            GTK_SIGNAL_FUNC (ol_scroll_window_resize), self);
  gtk_widget_add_events (GTK_WIDGET (self), GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK|GDK_POINTER_MOTION_HINT_MASK|GDK_DESTROY);
}
static void
ol_scroll_window_resize (OlScrollWindow *scroll)
{
  g_return_if_fail (scroll != NULL);
  gint width,height;
  gtk_window_get_size (GTK_WINDOW (scroll), &width, &height);
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  priv->width = width;
  priv->height = height;
  gtk_widget_queue_draw (GTK_WIDGET (scroll));
}

static void
ol_scroll_window_class_init (OlScrollWindowClass *klass)
{
  printf("scroll window class_init\n");
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  parent_class = g_type_class_peek_parent (klass);
  gobject_class = G_OBJECT_CLASS (klass);
  object_class = (GtkObjectClass*) klass;
  widget_class = (GtkWidgetClass*) klass;
  widget_class->expose_event = ol_scroll_window_expose;
  
  widget_class->button_press_event = ol_scroll_window_button_press;
  widget_class->button_release_event = ol_scroll_window_button_release;
  widget_class->motion_notify_event = ol_scroll_window_motion_notify;

  /*add private variables into OlScrollWindow*/
  g_type_class_add_private (gobject_class, sizeof (OlScrollWindowPrivate));
  
}

static void
ol_scroll_window_destroy (GtkObject *object)
{
  OlScrollWindow *scroll = OL_SCROLL_WINDOW (object);
  if (scroll->current_lyric_id!= -1)
  {
    scroll->current_lyric_id = -1;
  }
  GTK_OBJECT_CLASS (parent_class)->destroy (object);
}


static gboolean
ol_scroll_window_expose (GtkWidget *widget, GdkEventExpose *event)
{
  ol_scroll_window_paint (OL_SCROLL_WINDOW (widget));
  return FALSE;
}

void 
ol_scroll_window_set_whole_lyrics(OlScrollWindow *scroll, GPtrArray *whole_lyrics, gint whole_lyrics_len)
{
  ol_log_func ();
  g_return_if_fail (scroll != NULL);
  if (whole_lyrics == NULL) {
    scroll->whole_lyrics = NULL;
  }
  else {
    int i;
    scroll->whole_lyrics = g_ptr_array_new_with_free_func (g_free);
    for (i = 0; i < whole_lyrics_len; i++) {
      g_ptr_array_add (scroll->whole_lyrics, g_strdup (g_ptr_array_index (whole_lyrics, i)));
    }
  }
  scroll->whole_lyrics_len = whole_lyrics_len;
}

static PangoLayout*
_get_pango (OlScrollWindow *scroll, cairo_t *cr)
{
  ol_assert (OL_IS_SCROLL_WINDOW (scroll));
  ol_assert (cr != NULL);
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  PangoLayout *layout = pango_cairo_create_layout (cr);
  gchar *font_string = g_strdup_printf ("%s %0.0lf",
                                        priv->font_family,
                                        priv->font_size);
  PangoFontDescription *desc = pango_font_description_from_string (font_string);
  pango_font_description_set_style (desc, PANGO_STYLE_OBLIQUE);
  pango_font_description_set_weight (desc, PANGO_WEIGHT_BOLD);
  //pango_font_description_set_variant (desc, PANGO_STRETCH_SEMI_EXPANDED);
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);
  return layout;
}

static cairo_t*
_get_cairo (OlScrollWindow *scroll)
{
  ol_assert (OL_IS_SCROLL_WINDOW (scroll));
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  cairo_t *cr;
  cr = gdk_cairo_create (GTK_WIDGET (scroll)->window);
  cairo_set_source_rgb (cr, DEFAULT_BG_COLOR.r, DEFAULT_BG_COLOR.b, DEFAULT_BG_COLOR.g);
  gint width, height;
  gdk_drawable_get_size (gtk_widget_get_window (GTK_WIDGET (scroll)),
			 &width, &height);
  cairo_rectangle (cr, 0, 0, width, height);
  cairo_fill(cr);
  /*clip the disply area*/
  cairo_set_source_rgb (cr, DEFAULT_BG_COLOR.r, DEFAULT_BG_COLOR.b, DEFAULT_BG_COLOR.g);
  cairo_rectangle (cr, 0, 0, width, height);
  cairo_clip (cr);
  return cr;
}

static double
_get_active_color_ratio (OlScrollWindow *scroll, int line)
{
  double ratio = 0.0;
  if (line == scroll->current_lyric_id)
  {
    ratio = (1.0 - scroll->percentage) / 0.1;
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;
    return ratio;
  }
  else if (line == scroll->current_lyric_id + 1)
  {
    ratio = (scroll->percentage - 0.9) / 0.1;
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;
  }
  return ratio;
}

static void 
_draw_destory_button (OlScrollWindow *scroll, cairo_t *cr, double size)
{
   ol_assert (OL_IS_SCROLL_WINDOW (scroll));
   ol_assert (cr != NULL);
   gint width;
   gdk_drawable_get_size (gtk_widget_get_window (GTK_WIDGET (scroll)),
			 &width, NULL);
   cairo_move_to (cr, width-size-3.0, 3.0);
   cairo_line_to (cr, width-3.0, size+3.0);

   cairo_move_to (cr, width-size-3.0, size+3.0);
   cairo_line_to (cr, width-3.0, 3.0);
   cairo_set_source_rgb (cr, 1, 1, 1);
   cairo_set_line_width (cr, 2.0);
   cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
   cairo_stroke (cr);
}
static void
ol_scroll_window_paint (OlScrollWindow *scroll)
{
  ol_log_func ();
  ol_assert (OL_IS_SCROLL_WINDOW (scroll));
  GtkWidget *widget = GTK_WIDGET (scroll);
  ol_assert (GTK_WIDGET_REALIZED (widget));
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  int line_height = ol_scroll_window_get_font_height (scroll) + priv->outline_width;
  int count = ol_scroll_window_compute_line_count (scroll);
  double percentage = scroll->percentage;
  gint width, height;
  gdk_drawable_get_size (gtk_widget_get_window (GTK_WIDGET (scroll)),
                         &width, &height);
  
  cairo_t *cr = _get_cairo (scroll);
  /* set the font */
  PangoLayout *layout = _get_pango (scroll, cr);
  /* paint the destory button*/
  _draw_destory_button (scroll, cr, 10);
  /* paint the lyrics*/
  int i;
  int begin = scroll->current_lyric_id - count / 2;
  int end = scroll->current_lyric_id + count / 2 + 1;
  int ypos = height / 2  - line_height * percentage - (count / 2 + 1) * line_height;
  cairo_set_source_rgb(cr,
                       priv->inactive_color.r,
                       priv->inactive_color.g,
                       priv->inactive_color.b);
  if (scroll->whole_lyrics != NULL) {
    for (i = begin; i < end; i++) {
      ypos += line_height;
      if (i < 0) continue;
      if (i >= scroll->whole_lyrics->len)
        break;
      pango_layout_set_text (layout,
                             g_strdup (g_ptr_array_index (scroll->whole_lyrics, i)),
                             -1);
      cairo_save (cr);
      double ratio = _get_active_color_ratio (scroll, i);
      cairo_set_source_rgb (cr,
                            priv->active_color.r * ratio +
                            priv->inactive_color.r * (1 - ratio),
                            priv->active_color.g * ratio +
                            priv->inactive_color.g * (1 - ratio),
                            priv->active_color.b * ratio +
                            priv->inactive_color.b * (1 - ratio));
      cairo_move_to (cr, 0, ypos);
      pango_cairo_update_layout (cr, layout);
      pango_cairo_show_layout (cr, layout);
      cairo_restore (cr);
    }
  }
  g_object_unref (layout);
  cairo_destroy (cr);
}

  

void
ol_scroll_window_set_lyric (OlScrollWindow *scroll, const int lyric_id)
{
  ol_log_func ();
  g_return_if_fail (OL_IS_SCROLL_WINDOW (scroll));
  scroll->current_lyric_id = lyric_id; 
}

void
ol_scroll_window_set_current_percentage (OlScrollWindow *scroll, double percentage)
{
  g_return_if_fail (OL_IS_SCROLL_WINDOW (scroll));
  scroll->percentage = percentage;
  gtk_widget_queue_draw (GTK_WIDGET (scroll));
}

static int
ol_scroll_window_compute_line_count (OlScrollWindow *scroll)
{
  g_return_if_fail (OL_IS_SCROLL_WINDOW (scroll));
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  int font_height = ol_scroll_window_get_font_height (scroll) + priv->outline_width;
  int line_count = priv->height/font_height;
  return line_count;
}


static int
ol_scroll_window_get_font_height (OlScrollWindow *scroll)
{
  g_return_if_fail (OL_IS_SCROLL_WINDOW (scroll));
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  
  PangoContext *pango_context = gdk_pango_context_get ();
  PangoLayout *pango_layout = pango_layout_new (pango_context);
  gchar *font_string = g_strdup_printf ("%s %0.0lf", priv->font_family, priv->font_size);
  PangoFontDescription *font_desc = pango_font_description_from_string (font_string);
  g_free (font_string);
  pango_layout_set_font_description (pango_layout, font_desc);

  PangoFontMetrics *metrics = pango_context_get_metrics (pango_context,
                                                         pango_layout_get_font_description (pango_layout), /* font desc */
                                                         NULL); /* languague */
  if (metrics == NULL)
  {
    return priv->font_size;
  }
  int height = 0;
  int ascent, descent;
  ascent = pango_font_metrics_get_ascent (metrics);
  descent = pango_font_metrics_get_descent (metrics);
  pango_font_metrics_unref (metrics);
    
  height += PANGO_PIXELS (ascent + descent);
  return height;
}

int 
ol_scroll_window_get_current_lyric_id (OlScrollWindow *scroll)
{
  g_return_if_fail (OL_IS_SCROLL_WINDOW (scroll));
  return scroll->current_lyric_id;
}

void
ol_scroll_window_set_font_family (OlScrollWindow *scroll,
                               const char *font_family)
{
  if (scroll == NULL || font_family == NULL)
    return;
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  priv->font_family = font_family;
  gtk_widget_queue_draw (GTK_WIDGET (scroll));
}

const char*
ol_scroll_window_get_font_family (OlScrollWindow *scroll)
{
  if (scroll == NULL)
    return NULL;
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  return priv->font_family;
}

static gboolean 
ol_scroll_window_button_press (GtkWidget * widget, 
			      GdkEventButton * event)
{
  if (event->button == 1) {
    drag = TRUE;
    nX = event->x;
    nY = event->y;
  }
  return TRUE;
}

static gboolean
ol_scroll_window_button_release (GtkWidget * widget, 
				GdkEventButton * event)
{
  gint width;
  gdk_drawable_get_size (gtk_widget_get_window (widget),
			 &width, NULL);
  if (event->x <= width-3 &&event->x >= width-13 && event->y >= 3 && event->y <= 13) {
    gtk_widget_destroy (widget);
  }
  if (event->button == 1)
    drag = FALSE;
  return TRUE;
}

static gboolean
ol_scroll_window_motion_notify (GtkWidget * widget, 
			      GdkEventButton * event)
{
  if (drag) {
    int x, y;
    gtk_window_get_position ((GtkWindow *) widget, &x, &y);
    gtk_window_move((GtkWindow *) widget, x + event->x - nX, y + event->y - nY);
  }
  return TRUE;
} 
