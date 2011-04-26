#include <math.h>
#include "ol_scroll_window.h"
#include <ol_debug.h>
#include <pango/pangocairo.h>
#include <glib.h>
#include <ol_color.h>


#define OL_SCROLL_WINDOW_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE \
                                              ((obj),                   \
                                               ol_scroll_window_get_type (), \
                                               OlScrollWindowPrivate))

/*************default setting******************/
static const gint DEFAULT_WIDTH = 600;
static const gint DEFAULT_HEIGHT = 600;
static const gint DEFAULT_LINE_COUNT = 20;
static const OlColor DEFAULT_ACTIVE_COLOR = {0.89,0.81,0};
static const OlColor DEFAULT_INACTIVE_COLOR = {0.98,0.92,0.84};
static const OlColor DEFAULT_BG_COLOR = {0,0,0};
static const char *DEFAULT_FONT_NAME = "serif 13";
static const double DEFAULT_ALIGNMENT = 0.5;
static const gint DEFAULT_LINE_MARGIN = 1;
static const gint DEFAULT_PADDING_X = 10;
static const gint DEFAULT_PADDING_Y = 5;
static const gint DEFAULT_CORNER_RADIUS = 10;
static const double DEFAULT_BG_OPACITY = 0.9;
/**********************************************/


typedef struct __OlScrollWindowPrivate OlScrollWindowPrivate;
struct __OlScrollWindowPrivate
{
  gint line_count;
  OlColor active_color;            /*Color of playing lyric*/
  OlColor inactive_color;          /*Color of non-playing lyric*/
  OlColor bg_color;
  gchar *font_name;                /*Font string, including family, size, and style*/
  double alignment;                /*Lyric alignment, 0.0 is left, 1.0 is right*/ 
  gint line_margin;                /*Margin between two lines*/
  gint padding_x;
  gint padding_y;
  gint corner_radius;
  double bg_opacity;
};


static cairo_t* _get_cairo (OlScrollWindow *scroll);
static PangoLayout* _get_pango (OlScrollWindow *scroll, cairo_t *cr);
static void _paint_bg (OlScrollWindow *scroll, cairo_t *cr);

static void ol_scroll_window_init (OlScrollWindow *self);
static void ol_scroll_window_class_init (OlScrollWindowClass *klass);
static void ol_scroll_window_destroy (GtkObject *object);
static gboolean ol_scroll_window_expose (GtkWidget *widget, GdkEventExpose *event);

static void ol_scroll_window_paint (OlScrollWindow *scroll);
static int ol_scroll_window_compute_line_count (OlScrollWindow *scroll);
static int ol_scroll_window_get_font_height (OlScrollWindow *scroll);

static PangoLayout* _get_pango (OlScrollWindow *scroll, cairo_t *cr);

static gboolean ol_scroll_window_button_press (GtkWidget *widget,
                                               GdkEventButton *event);
static gboolean ol_scroll_window_button_release (GtkWidget *widget,
                                                 GdkEventButton *event);
static gboolean ol_scroll_window_motion_notify (GtkWidget *widget,
                                                GdkEventMotion *event);

static void _draw_destory_button (OlScrollWindow *scroll, cairo_t *cr, double size);

G_DEFINE_TYPE (OlScrollWindow, ol_scroll_window, GTK_TYPE_WINDOW);


GtkWidget*
ol_scroll_window_new ()
{
  OlScrollWindow *scroll;
  scroll = g_object_new (ol_scroll_window_get_type (), NULL);
  gtk_window_set_decorated (GTK_WINDOW(scroll), FALSE);
  /* gtk_window_set_opacity(GTK_WINDOW(scroll), 0.1); */
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
  priv->line_count = DEFAULT_LINE_COUNT;
  priv->active_color = DEFAULT_ACTIVE_COLOR;
  priv->inactive_color = DEFAULT_INACTIVE_COLOR;
  priv->bg_color = DEFAULT_BG_COLOR;
  priv->font_name = g_strdup (DEFAULT_FONT_NAME);
  priv->alignment = DEFAULT_ALIGNMENT;
  priv->line_margin = DEFAULT_LINE_MARGIN;
  priv->padding_x = DEFAULT_PADDING_X;
  priv->padding_y = DEFAULT_PADDING_Y;
  priv->corner_radius = DEFAULT_CORNER_RADIUS;
  priv->bg_opacity = DEFAULT_BG_OPACITY;
  /*set allocation*/
  gtk_window_resize(GTK_WINDOW(self), DEFAULT_WIDTH, DEFAULT_HEIGHT);
  gtk_widget_add_events (GTK_WIDGET (self),
                         GDK_BUTTON_PRESS_MASK |
                         GDK_BUTTON_RELEASE_MASK |
                         GDK_POINTER_MOTION_MASK |
                         GDK_POINTER_MOTION_HINT_MASK |
                         GDK_DESTROY);
  /* Set RGBA Colormap */
  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (self));
  GdkColormap* colormap = gdk_screen_get_rgba_colormap (screen);
  if (colormap == NULL)
    colormap = gdk_screen_get_rgb_colormap (screen);
  gtk_widget_set_colormap (GTK_WIDGET (self), colormap);
}

static void
ol_scroll_window_class_init (OlScrollWindowClass *klass)
{
  printf("scroll window class_init\n");
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;
  GtkObjectClass *gtkobject_class;
  gobject_class = G_OBJECT_CLASS (klass);
  gtkobject_class = GTK_OBJECT_CLASS (klass);
  widget_class = (GtkWidgetClass*) klass;
  widget_class->expose_event = ol_scroll_window_expose;
  widget_class->button_press_event = ol_scroll_window_button_press;
  widget_class->button_release_event = ol_scroll_window_button_release;
  widget_class->motion_notify_event = ol_scroll_window_motion_notify;
  gtkobject_class->destroy = ol_scroll_window_destroy;
  /*add private variables into OlScrollWindow*/
  g_type_class_add_private (gobject_class, sizeof (OlScrollWindowPrivate));
  
}

static void
ol_scroll_window_destroy (GtkObject *object)
{
  OlScrollWindow *scroll = OL_SCROLL_WINDOW (object);
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (object);
  if (priv->font_name != NULL)
    g_free (priv->font_name);
  if (scroll->current_lyric_id!= -1)
  {
    scroll->current_lyric_id = -1;
  }
  GTK_OBJECT_CLASS (ol_scroll_window_parent_class)->destroy (object);
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
  ol_assert_ret (OL_IS_SCROLL_WINDOW (scroll), NULL);
  ol_assert_ret (cr != NULL, NULL);
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  PangoLayout *layout = pango_cairo_create_layout (cr);
  PangoFontDescription *desc = pango_font_description_from_string (priv->font_name);
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);
  return layout;
}

static cairo_t*
_get_cairo (OlScrollWindow *scroll)
{
  ol_assert_ret (OL_IS_SCROLL_WINDOW (scroll), NULL);
  cairo_t *cr;
  cr = gdk_cairo_create (GTK_WIDGET (scroll)->window);
  cairo_save (cr);
  cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);
  cairo_restore (cr);
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
_paint_bg (OlScrollWindow *scroll, cairo_t *cr)
{
  ol_assert (OL_IS_SCROLL_WINDOW (scroll));
  ol_assert (cr != NULL);
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  cairo_set_source_rgb (cr, DEFAULT_BG_COLOR.r,
                        DEFAULT_BG_COLOR.b,
                        DEFAULT_BG_COLOR.g);
  gint width, height;
  gdk_drawable_get_size (gtk_widget_get_window (GTK_WIDGET (scroll)),
			 &width, &height);
  cairo_save (cr);
  cairo_new_path (cr);
  /* Top-left */
  cairo_arc (cr,
             priv->corner_radius, priv->corner_radius,
             priv->corner_radius,
             M_PI, M_PI * 1.5);
  cairo_line_to (cr, width - priv->corner_radius, 0);
  /* Top-right */
  cairo_arc (cr,
             width - priv->corner_radius, priv->corner_radius,
             priv->corner_radius,
             M_PI * 1.5, M_PI * 2);
  cairo_line_to (cr, width, height - priv->corner_radius);
  /* Bottom-right */
  cairo_arc (cr,
             width - priv->corner_radius, height - priv->corner_radius,
             priv->corner_radius,
             0, M_PI * 0.5);
  cairo_line_to (cr, priv->corner_radius, height);
  /* Bottom-left */
  cairo_arc (cr,
             priv->corner_radius, height - priv->corner_radius,
             priv->corner_radius,
             M_PI * 0.5, M_PI);
  cairo_close_path (cr);
  cairo_set_source_rgba (cr, DEFAULT_BG_COLOR.r, DEFAULT_BG_COLOR.b, DEFAULT_BG_COLOR.g, priv->bg_opacity);
  cairo_fill(cr);
  /*clip the disply area*/
  cairo_restore (cr);
}

static void
ol_scroll_window_paint (OlScrollWindow *scroll)
{
  ol_log_func ();
  ol_assert (OL_IS_SCROLL_WINDOW (scroll));
  GtkWidget *widget = GTK_WIDGET (scroll);
  ol_assert (GTK_WIDGET_REALIZED (widget));
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  int line_height = ol_scroll_window_get_font_height (scroll) + priv->line_margin;
  int count = ol_scroll_window_compute_line_count (scroll);
  double percentage = scroll->percentage;
  gint width, height;
  gdk_drawable_get_size (gtk_widget_get_window (GTK_WIDGET (scroll)),
                         &width, &height);
  
  cairo_t *cr = _get_cairo (scroll);
  _paint_bg (scroll, cr);
  /* set the font */
  PangoLayout *layout = _get_pango (scroll, cr);
  /* paint the destory button*/
  _draw_destory_button (scroll, cr, 10);
  /* paint the lyrics*/
  cairo_save (cr);
  cairo_new_path (cr);
  cairo_rectangle (cr,
                   priv->padding_x, 0,
                   width - priv->padding_x * 2,
                   height - priv->padding_y * 2);
  cairo_close_path (cr);
  cairo_clip (cr);
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
      double alpha = 1.0;
      if (ypos < line_height / 2.0 + priv->padding_y)
        alpha = 1.0 - (line_height / 2.0 + priv->padding_y - ypos) * 1.0 / line_height * 2;
      else if (ypos > height - line_height * 1.5 - priv->padding_y)
        alpha = (height - line_height - priv->padding_y - ypos) * 1.0 / line_height * 2;
      if (alpha < 0.0) alpha = 0.0;
      cairo_set_source_rgba (cr,
                             priv->active_color.r * ratio + 
                             priv->inactive_color.r * (1 - ratio),
                             priv->active_color.g * ratio +
                             priv->inactive_color.g * (1 - ratio),
                             priv->active_color.b * ratio +
                             priv->inactive_color.b * (1 - ratio),
                             alpha);
      cairo_move_to (cr, priv->padding_x, ypos);
      pango_cairo_update_layout (cr, layout);
      pango_cairo_show_layout (cr, layout);
      cairo_restore (cr);
    }
  }
  g_object_unref (layout);
  cairo_reset_clip (cr);
  cairo_restore (cr);
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
  ol_assert_ret (OL_IS_SCROLL_WINDOW (scroll), 0);
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  int font_height = ol_scroll_window_get_font_height (scroll) + priv->line_margin;
  gint width, height;
  gtk_window_get_size (GTK_WINDOW (scroll), &width, &height);
  int line_count = height - priv->padding_y * 2 / font_height;
  return line_count;
}


static int
ol_scroll_window_get_font_height (OlScrollWindow *scroll)
{
  ol_assert_ret (OL_IS_SCROLL_WINDOW (scroll), 0);
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  
  PangoContext *pango_context = gdk_pango_context_get ();
  PangoLayout *pango_layout = pango_layout_new (pango_context);
  PangoFontDescription *font_desc = pango_font_description_from_string (priv->font_name);
  pango_layout_set_font_description (pango_layout, font_desc);

  PangoFontMetrics *metrics = pango_context_get_metrics (pango_context,
                                                         pango_layout_get_font_description (pango_layout), /* font desc */
                                                         NULL); /* languague */
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
  ol_assert_ret (OL_IS_SCROLL_WINDOW (scroll), -1);
  return scroll->current_lyric_id;
}

void
ol_scroll_window_set_font_name (OlScrollWindow *scroll,
                                const char *font_name)
{
  if (scroll == NULL || font_name == NULL)
    return;
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  if (priv->font_name != NULL)
    g_free (priv->font_name);
  priv->font_name = g_strdup (font_name);
  gtk_widget_queue_draw (GTK_WIDGET (scroll));
}

const char*
ol_scroll_window_get_font_name (OlScrollWindow *scroll)
{
  if (scroll == NULL)
    return NULL;
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  return priv->font_name;
}

static gboolean 
ol_scroll_window_button_press (GtkWidget * widget, 
                               GdkEventButton * event)
{
  if (event->button == 1) {
    gtk_window_begin_move_drag (GTK_WINDOW (widget),
                                event->button,
                                (gint)event->x_root,
                                (gint)event->y_root,
                                event->time);
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
  if (event->x <= width-3 &&event->x >= width-13 && event->y >= 3 && event->y <= 13)
  {
    gtk_widget_destroy (widget);
  }
  return TRUE;
}

static gboolean
ol_scroll_window_motion_notify (GtkWidget *widget, 
                                GdkEventMotion *event)
{
  return TRUE;
}
