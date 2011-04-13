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
static const gint DEFAULT_HEIGHT = 400;
static const gint DEFAULT_LINE_COUNT = 20;
static const OlColor DEFAULT_ACTIVE_COLOR = {1,1,1};
static const OlColor DEFAULT_INACTIVE_COLOR = {1,0,0};
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
  gchar *font_family;              /*字体*/
  double font_size;                /*字体大小*/
  double alignment;                /*对齐方式*/ 
  gint outline_width;              /*边栏宽度*/

};


static void ol_scroll_window_init (OlScrollWindow *self);
static void ol_scroll_window_class_init (OlScrollWindowClass *klass);
static void ol_scroll_window_destroy (GtkObject *object);
static gboolean ol_scroll_window_expose (GtkWidget *widget, GdkEventExpose *event);

static void ol_scroll_window_paint (OlScrollWindow *scroll);
static void ol_scroll_window_set_paint_lyrics (OlScrollWindow *scroll);
static int ol_scroll_window_compute_line_count (OlScrollWindow *scroll);
static int ol_scroll_window_get_font_height (OlScrollWindow *scroll);


static GtkWidgetClass *parent_class = NULL;


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
  return GTK_WIDGET (scroll);
}

static void
ol_scroll_window_init (OlScrollWindow *self)
{
  /*basic*/
  self->paint_lyrics = NULL;
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
  priv->font_family = g_strdup (DEFAULT_FONT_FAMILY);
  priv->font_size = DEFAULT_FONT_SIZE;
  priv->alignment = DEFAULT_ALIGNMENT;
  priv->outline_width = DEFAULT_OUTLINE_WIDTH;
  /*set allocation*/
  gtk_window_resize(GTK_WINDOW(self), DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

static void
ol_scroll_window_class_init (OlScrollWindowClass *klass)
{
  printf("scroll window class_init\n");
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;
  GtkObjectClass *gtkobject_class;
  parent_class = g_type_class_peek_parent (klass);
  gobject_class = G_OBJECT_CLASS (klass);
  gtkobject_class = GTK_OBJECT_CLASS (klass);
  widget_class = (GtkWidgetClass*) klass;
  widget_class->expose_event = ol_scroll_window_expose;
  gtkobject_class->destroy = ol_scroll_window_destroy;
  /*add private variables into OlScrollWindow*/
  g_type_class_add_private (gobject_class, sizeof (OlScrollWindowPrivate));
  
}

static void
ol_scroll_window_destroy (GtkObject *object)
{
  OlScrollWindow *scroll = OL_SCROLL_WINDOW (object);
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (object);
  if (priv->font_family != NULL)
    g_free (priv->font_family);
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


static void
ol_scroll_window_paint (OlScrollWindow *scroll)
{
  ol_log_func ();
  g_return_if_fail (scroll != NULL);
  GtkWidget *widget = GTK_WIDGET (scroll);
  if (!GTK_WIDGET_REALIZED (widget))
    gtk_widget_realize (widget);
  
  cairo_t *cr;
  cr = gdk_cairo_create (widget->window);
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  int line_height = ol_scroll_window_get_font_height (scroll) + priv->outline_width;
  int count = ol_scroll_window_compute_line_count (scroll);
  double percentage = scroll->percentage;
  cairo_set_source_rgb (cr, DEFAULT_BG_COLOR.r, DEFAULT_BG_COLOR.b, DEFAULT_BG_COLOR.g);
  cairo_rectangle (cr, 0, 0, priv->width, priv->height);
  cairo_fill(cr);
  /*clip the disply area*/
  cairo_set_source_rgb (cr, DEFAULT_BG_COLOR.r, DEFAULT_BG_COLOR.b, DEFAULT_BG_COLOR.g);
  cairo_rectangle (cr, 0,line_height/2 , priv->width, line_height*(count-1)+line_height/2);
  cairo_clip (cr);
  
  /* set the font */
  PangoLayout *layout = pango_cairo_create_layout (cr);
  gchar *font_string = g_strdup_printf ("%s %0.0lf", priv->font_family, priv->font_size);
  PangoFontDescription *desc = pango_font_description_from_string (font_string);
  pango_font_description_set_style (desc, PANGO_STYLE_OBLIQUE);
  pango_font_description_set_weight (desc, PANGO_WEIGHT_BOLD);
  //pango_font_description_set_variant (desc, PANGO_STRETCH_SEMI_EXPANDED);
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);
 
  /* paint the lyrics*/
  int i;
  cairo_set_source_rgb(cr, priv->inactive_color.r, priv->inactive_color.g, priv->inactive_color.b);
  if (scroll->paint_lyrics != NULL) {
    for (i = 0; i < count; i++) {
      pango_layout_set_text (layout, g_strdup (g_ptr_array_index (scroll->paint_lyrics, i)), -1);
      cairo_save (cr);
      if (i == count/2||(i == count/2+1&&scroll->percentage >= 0.6))
	cairo_set_source_rgb (cr, priv->active_color.r, priv->active_color.g, priv->active_color.b);
      cairo_move_to (cr, 0, line_height*(1-percentage) + i*line_height);
      pango_cairo_update_layout (cr, layout);
      pango_cairo_show_layout (cr, layout);
      cairo_restore (cr);
      cairo_set_source_rgb (cr, priv->inactive_color.r, priv->inactive_color.g, priv->inactive_color.b);
    }
    g_object_unref (layout);
  }
  cairo_destroy (cr);
}

  

void
ol_scroll_window_set_lyric (OlScrollWindow *scroll, const int lyric_id)
{
  ol_log_func ();
  g_return_if_fail (OL_IS_SCROLL_WINDOW (scroll));
  scroll->current_lyric_id = lyric_id; 
  ol_scroll_window_set_paint_lyrics (scroll);
}

void
ol_scroll_window_set_current_percentage (OlScrollWindow *scroll, double percentage)
{
  g_return_if_fail (OL_IS_SCROLL_WINDOW (scroll));
  scroll->percentage = percentage;
  gtk_widget_queue_draw (GTK_WIDGET (scroll));
}

static void
ol_scroll_window_set_paint_lyrics (OlScrollWindow *scroll)
{
  int tag,i,k;
  int current_id = scroll->current_lyric_id;
  int whole_lyrics_len = scroll->whole_lyrics_len;
  int count = ol_scroll_window_compute_line_count (scroll);
  scroll->paint_lyrics = g_ptr_array_new_with_free_func (g_free);
  /*the whole lyrics lenth are bigger than the paint lyrics count*/
  if (whole_lyrics_len >= count) {
    if (current_id < 0)
      tag = 0;
    else if (current_id < count/2)
      tag = 1;
    else if (current_id <= whole_lyrics_len-count+count/2)
      tag = 2;
    else
      tag = 3;
    switch (tag) {
    case 0:
      for (i = 0; i < count; i++)
	g_ptr_array_add (scroll->paint_lyrics, g_strdup (""));
      break;
    case 1:
      k = 0;
      for (i = 0; i < count/2-current_id; i++)
	g_ptr_array_add (scroll->paint_lyrics, g_strdup (""));
    
      for (i = count/2-current_id; i < count; i++) {
	g_ptr_array_add (scroll->paint_lyrics, g_strdup (g_ptr_array_index (scroll->whole_lyrics, k)));
	k++;
      }
      break;
    case 2:
      k = current_id - count/2;;
      for (i = 0; i < count; i++) {
	g_ptr_array_add (scroll->paint_lyrics, g_strdup (g_ptr_array_index (scroll->whole_lyrics, k)));
	k++;
      }
      break;
    case 3:
      k = current_id - count/2;
      for (i = 0; i < count; i++) {
	if (k < whole_lyrics_len) {
	  g_ptr_array_add (scroll->paint_lyrics, g_strdup (g_ptr_array_index (scroll->whole_lyrics, k)));
	  k++;
	}
	else
	  g_ptr_array_add (scroll->paint_lyrics, g_strdup (""));
      }
      break;
    }
  }
  /*the whole lyrics lenth are smaller than the paint lyrics count*/
  else if (whole_lyrics_len < count) {
    if (current_id < 0)
      tag = 0;
    else if (current_id < count/2) {
      if (current_id <= whole_lyrics_len - count + count/2)
	tag = 1;
      else
	tag = 2;
    }
    else if (current_id >= count/2)
      tag = 3;
    switch (tag) {
    case 0:
      for (i = 0; i < count; i++)
	g_ptr_array_add (scroll->paint_lyrics, g_strdup (""));
      break;
    case 1:
      k = 0;
      for (i = 0; i < count/2-current_id; i++)
	g_ptr_array_add (scroll->paint_lyrics, g_strdup (""));
    
      for (i = count/2-current_id; i < count; i++) {
	g_ptr_array_add (scroll->paint_lyrics, g_strdup (g_ptr_array_index (scroll->whole_lyrics, k)));
	k++;
      }
      break;
    case 2:
      k = 0;
      for (i = 0; i < count/2-current_id; i++)
	g_ptr_array_add (scroll->paint_lyrics, g_strdup (""));
      for (i = count/2-current_id; i < whole_lyrics_len; i++) {
	g_ptr_array_add (scroll->paint_lyrics, g_strdup (g_ptr_array_index (scroll->whole_lyrics, k)));
	k++;
      }
      for (i = whole_lyrics_len; i < count; i++)
        g_ptr_array_add (scroll->paint_lyrics, g_strdup (""));
      break;
    case 3:
      k = current_id - count/2;
      for (i = 0; i < count; i++) {
	if (k < whole_lyrics_len) {
	  g_ptr_array_add (scroll->paint_lyrics, g_strdup (g_ptr_array_index (scroll->whole_lyrics, k)));
	  k++;
	}
	else
	  g_ptr_array_add (scroll->paint_lyrics, g_strdup (""));
      }
      break;
    }
  }
}

static int
ol_scroll_window_compute_line_count (OlScrollWindow *scroll)
{
  ol_assert_ret (OL_IS_SCROLL_WINDOW (scroll), 0);
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  int font_height = ol_scroll_window_get_font_height (scroll) + priv->outline_width;
  int line_count = priv->height/font_height;
  return line_count;
}


static int
ol_scroll_window_get_font_height (OlScrollWindow *scroll)
{
  ol_assert_ret (OL_IS_SCROLL_WINDOW (scroll), 0);
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
  ol_assert_ret (OL_IS_SCROLL_WINDOW (scroll), -1);
  return scroll->current_lyric_id;
}

void
ol_scroll_window_set_font_family (OlScrollWindow *scroll,
                                  const char *font_family)
{
  if (scroll == NULL || font_family == NULL)
    return;
  OlScrollWindowPrivate *priv = OL_SCROLL_WINDOW_GET_PRIVATE (scroll);
  if (priv->font_family != NULL)
    g_free (priv->font_family);
  priv->font_family = g_strdup (font_family);
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
