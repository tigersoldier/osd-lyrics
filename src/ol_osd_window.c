#include <gtk/gtkprivate.h>
#include "ol_osd_window.h"

#define OL_OSD_WINDOW_GET_PRIVATE(obj)   (G_TYPE_INSTANCE_GET_PRIVATE  \
                                       ((obj),                      \
                                        ol_osd_window_get_type (),     \
                                        OlOsdWindowPrivate))

enum {
  PROP_0,
  PROP_XALIGN,
  PROP_YALIGN,
  PROP_LOCKED,
};

typedef struct __OlOsdWindowPrivate OlOsdWindowPrivate;
struct __OlOsdWindowPrivate
{
  float xalign;
  float yalign;
  gboolean locked;
  gboolean pressed;
  gint mouse_x;
  gint mouse_y;
  gint old_x;
  gint old_y;
};

struct OsdLrc
{
  gint id;
  gchar *lyric;
};

GType ol_osd_window_get_type (void);
static void ol_osd_window_init (OlOsdWindow *self);
static void ol_osd_window_class_init (OlOsdWindowClass *klass);
static void ol_osd_window_set_property (GObject *object, guint prop_id,
                                     const GValue *value, GParamSpec *pspec);
static void ol_osd_window_get_property (GObject *object, guint prop_id,
                                     GValue *value, GParamSpec *pspec);
static void ol_osd_window_realize (GtkWidget *widget);
static void ol_osd_window_map (GtkWidget *widget);
static void ol_osd_window_unrealize (GtkWidget *widget);
static void ol_osd_window_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static void ol_osd_window_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void ol_osd_window_show (GtkWidget *widget);
static gboolean ol_osd_window_expose (GtkWidget *widget, GdkEventExpose *event);
static gboolean ol_osd_window_enter_notify (GtkWidget *widget, GdkEventCrossing *event);
static gboolean ol_osd_window_leave_notify (GtkWidget *widget, GdkEventCrossing *event);
static gboolean ol_osd_window_button_press (GtkWidget *widget, GdkEventButton *event);
static gboolean ol_osd_window_motion_notify (GtkWidget *widget, GdkEventMotion *event);
static gboolean ol_osd_window_button_release (GtkWidget *widget, GdkEventButton *event);
static void ol_osd_window_compute_position (OlOsdWindow *osd, GtkAllocation *allocation);
static void ol_osd_window_compute_alignment (OlOsdWindow *osd, gint x, gint y,
                                          gdouble *xalign, gdouble *yalign);
/** 
 * @brief Paint the layout to be OSD style
 *
 * Paint the lyrics, the odd_lrc will be shown upper-left and the even_lrc will be shown bottom-right
 * @param osd An OlOsdWindow
 * @param cr A cairo_t to be painted
 */
static void ol_osd_window_paint_lyrics (OlOsdWindow *osd, cairo_t *cr);
static void ol_osd_window_update_pixmap (OlOsdWindow *osd);
static void ol_osd_window_update_shape (OlOsdWindow *osd);

static GtkWidgetClass *parent_class = NULL;

GType
ol_osd_window_get_type (void)
{
  static GType ol_osd_type = 0;
  if (!ol_osd_type)
  {
    static const GTypeInfo ol_osd_info =
      {
        sizeof (OlOsdWindowClass),
        NULL,                   /* base_init */
        NULL,                   /* base_finalize */
        (GClassInitFunc) ol_osd_window_class_init,
        NULL,                   /* class_finalize */
        NULL,                   /* class_data */
        sizeof (OlOsdWindow),
        16,                     /* n_preallocs */
        (GInstanceInitFunc) ol_osd_window_init,
      };
    ol_osd_type = g_type_register_static (GTK_TYPE_WIDGET, "OlOsdWindow",
                                       &ol_osd_info, 0);
  }
  return ol_osd_type;
}

static gboolean
ol_osd_window_button_press (GtkWidget *widget, GdkEventButton *event)
{
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  if (event->window == osd->event_window)
    printf ("press\n");
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  priv->pressed = TRUE;
  priv->old_x = widget->allocation.x;
  priv->old_y = widget->allocation.y;
  priv->mouse_x = event->x_root;
  priv->mouse_y = event->y_root;
  return FALSE;
}

static gboolean
ol_osd_window_button_release (GtkWidget *widget, GdkEventButton *event)
{
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
/*   if (event->window == osd->event_window) */
  printf ("release\n");
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  priv->pressed = FALSE;
  return FALSE;
}

static gboolean
ol_osd_window_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
  printf ("motion\n");
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  if (priv->pressed && !priv->locked)
  {
    int x = priv->old_x + (event->x_root - priv->mouse_x);
    int y = priv->old_y + (event->y_root - priv->mouse_y);
    gdouble xalign, yalign;
    OlOsdWindow *osd = OL_OSD_WINDOW (widget);
    ol_osd_window_compute_alignment (osd, x, y, &xalign, &yalign);
    ol_osd_window_set_alignment (osd, xalign, yalign);
  }
  return FALSE;
}

static gboolean
ol_osd_window_expose (GtkWidget *widget, GdkEventExpose *event)
{
  ol_osd_window_update_pixmap (OL_OSD_WINDOW (widget));
  return FALSE;
}

static gboolean
ol_osd_window_enter_notify (GtkWidget *widget, GdkEventCrossing *event)
{
  printf ("enter\n");
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  if (!priv->locked && !gdk_window_is_visible (osd->bg_window))
  {
    gdk_window_show (osd->bg_window);
    gdk_window_show (widget->window);
  }
  return FALSE;
}

static gboolean
ol_osd_window_leave_notify (GtkWidget *widget, GdkEventCrossing *event)
{
  printf ("leave\n"
          "  (%0.0lf,%0.0lf)\n",
          event->x_root, event->y_root);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  if (!priv->pressed &&
      (event->x_root < widget->allocation.x ||
       event->x_root >= widget->allocation.x + widget->allocation.width ||
       event->y_root < widget->allocation.y ||
       event->y_root >= widget->allocation.y + widget->allocation.height) &&
      gdk_window_is_visible (osd->bg_window))
  {
    gdk_window_hide (osd->bg_window);
  }
  return FALSE;
}

static void
ol_osd_window_show (GtkWidget *widget)
{
  printf ("show\n");
  GTK_WIDGET_SET_FLAGS (widget, GTK_VISIBLE);
  gtk_widget_map (widget);
}

static void
ol_osd_window_realize (GtkWidget *widget)
{
  g_return_if_fail (!GTK_WIDGET_REALIZED (widget));
  printf ("realize\n");
  OlOsdWindow *osd;
  GdkWindowAttr attr;
  GdkWindow *parent_window;
  gint attr_mask;

  osd = OL_OSD_WINDOW (widget);

  osd->screen = gtk_widget_get_screen (widget);
  
  /* ensure the size allocation */
  GtkAllocation allocation = {0, 0, 150, 150};
  if (widget->allocation.width <= 1 &&
      widget->allocation.height <= 1)
  {
    printf ("ensure\n");
    GtkRequisition requisition;
    gtk_widget_size_request (widget, &requisition);
    if (requisition.width || requisition.height)
    {
      allocation.width = requisition.width;
      allocation.height = requisition.height;
    }
    widget->allocation = allocation;
  }
  ol_osd_window_compute_position (osd, &allocation);
  gtk_widget_size_allocate (widget, &allocation);

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  /* create osd window */
  parent_window = gtk_widget_get_root_window (widget);
  
  attr.window_type = GDK_WINDOW_TEMP;
  attr.x = widget->allocation.x;
  attr.y = widget->allocation.y;
  attr.width = widget->allocation.width;
  attr.height = widget->allocation.height;
  attr.visual = gtk_widget_get_visual (widget);
  attr.colormap = gtk_widget_get_colormap (widget);
  attr.wclass = GDK_INPUT_OUTPUT;
  attr.event_mask = gtk_widget_get_events (widget);
  attr.event_mask |= (GDK_BUTTON_MOTION_MASK |
                      GDK_ENTER_NOTIFY_MASK |
                      GDK_LEAVE_NOTIFY_MASK |
                      GDK_BUTTON_PRESS_MASK |
                      GDK_BUTTON_RELEASE_MASK |
                      GDK_EXPOSURE_MASK);
  attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  
  widget->window = gdk_window_new (parent_window, &attr, attr_mask);
  gdk_window_set_colormap (widget->window, gdk_screen_get_rgba_colormap (osd->screen));

  gdk_window_set_user_data (widget->window, osd);
  widget->style = gtk_style_attach (widget->style, widget->window);

  /* create background window */
  osd->bg_window = gdk_window_new (parent_window, &attr, attr_mask);
  gtk_style_set_background (widget->style, osd->bg_window, GTK_STATE_NORMAL);
  gdk_window_set_opacity (osd->bg_window, 0.5);
  gdk_window_set_user_data (osd->bg_window, osd);

  /* create event window */
  attr.wclass = GDK_INPUT_ONLY;
  attr.override_redirect = TRUE;
  attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_NOREDIR;
  
  osd->event_window = gdk_window_new (parent_window, &attr, attr_mask);
  gdk_window_set_user_data (osd->event_window, osd);
  gdk_window_set_decorations (osd->event_window, 0);

  /* setup input shape mask for osd window */
  gint screen_width, screen_height;
  screen_width = gdk_screen_get_width (osd->screen);
  screen_height = gdk_screen_get_height (osd->screen);
  GdkPixmap *pixmap = gdk_pixmap_new (NULL, screen_width, screen_height, 1);
  gtk_widget_input_shape_combine_mask (widget, pixmap, 0, 0);
  g_object_unref (pixmap);
  ol_osd_window_update_shape (osd);
/*   ol_osd_window_update_pixmap (osd); */
}

static void
ol_osd_window_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
  requisition->width = 180;
  requisition->height = 190;
}

static void ol_osd_window_unrealize (GtkWidget *widget)
{
  printf ("unrealize\n");
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  if (osd->event_window)
  {
    gdk_window_set_user_data (osd->event_window, NULL);
    gdk_window_destroy (osd->event_window);
    osd->event_window = NULL;
  }
  if (osd->bg_window)
  {
    gdk_window_set_user_data (osd->bg_window, NULL);
    gdk_window_destroy (osd->bg_window);
    osd->bg_window = NULL;
  }
  GTK_WIDGET_CLASS(parent_class)->unrealize (widget);
}

static void
ol_osd_window_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
  printf ("%s\n"
          "(%d, %d) - %d x %d\n",
          __FUNCTION__,
          allocation->x,
          allocation->y,
          allocation->width,
          allocation->height);
  widget->allocation = *allocation;
/*   GtkAllocation alloc; */
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
/*   ol_osd_window_compute_position (osd, &alloc); */
/*   widget->allocation = alloc; */
/*   printf ("  computed: (%d, %d) - %d x %d\n", alloc.x, alloc.y, alloc.width, alloc.height); */
  if (GTK_WIDGET_REALIZED (widget))
  {
    OlOsdWindow *osd = OL_OSD_WINDOW (widget);
    gdk_window_move_resize (widget->window,
                            widget->allocation.x,
                            widget->allocation.y,
                            widget->allocation.width,
                            widget->allocation.height);
    gdk_window_move_resize (osd->event_window,
                            widget->allocation.x,
                            widget->allocation.y,
                            widget->allocation.width,
                            widget->allocation.height);
    gdk_window_move_resize (osd->bg_window,
                            widget->allocation.x,
                            widget->allocation.y,
                            widget->allocation.width,
                            widget->allocation.height);
  }
}

static void
ol_osd_window_map (GtkWidget *widget)
{
  printf ("map\n");
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  GTK_WIDGET_CLASS (parent_class)->map (widget);
  GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);
  if (!priv->locked)
  {
    gdk_window_show (widget->window);
    gdk_window_show (osd->event_window);
    gdk_window_raise (osd->event_window);
  }
}

static void
ol_osd_window_class_init (OlOsdWindowClass *klass)
{
  printf ("class init\n");
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  gobject_class = G_OBJECT_CLASS (klass);
  object_class = (GtkObjectClass*)klass;
  widget_class = (GtkWidgetClass*)klass;
  parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = ol_osd_window_set_property;
  
  widget_class->realize = ol_osd_window_realize;
  widget_class->unrealize = ol_osd_window_unrealize;
  widget_class->size_allocate = ol_osd_window_size_allocate;
  widget_class->size_request = ol_osd_window_size_request;
  widget_class->map = ol_osd_window_map;
  widget_class->show = ol_osd_window_show;
  widget_class->expose_event = ol_osd_window_expose;
  widget_class->button_press_event = ol_osd_window_button_press;
  widget_class->button_release_event = ol_osd_window_button_release;
  widget_class->motion_notify_event = ol_osd_window_motion_notify;
  widget_class->enter_notify_event = ol_osd_window_enter_notify;
  widget_class->leave_notify_event = ol_osd_window_leave_notify;
  
  /* set up properties */
  g_object_class_install_property (gobject_class,
                                   PROP_XALIGN,
                                   g_param_spec_float ("xalign",
                                                       ("Horizontal alignment for child"),
                                                       ("Horizontal position of window in desktop. "
                                                        "0.0 is left aligned, 1.0 is right aligned"),
                                                       0.0,
                                                       1.0,
                                                       0.5,
                                                       G_PARAM_READABLE | G_PARAM_WRITABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_YALIGN,
                                   g_param_spec_float ("yalign",
                                                       ("Horizontal alignment for child"),
                                                       ("Horizontal position of window in desktop. "
                                                        "0.0 is left aligned, 1.0 is right aligned"),
                                                       0.0,
                                                       1.0,
                                                       0.5,
                                                       G_PARAM_READABLE | G_PARAM_WRITABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_LOCKED,
                                   g_param_spec_boolean ("locked",
                                                         ("Whether the it is possible to move the OSD window"),
                                                         ("If TRUE, there will be a window displayed under the"
                                                          "OSD indicating that it can be moved, and so it is"),
                                                         FALSE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));
  g_type_class_add_private (gobject_class, sizeof (OlOsdWindowPrivate));
}

static void
ol_osd_window_set_property (GObject *object, guint prop_id,
                         const GValue *value, GParamSpec *pspec)
{
  OlOsdWindow *osd = OL_OSD_WINDOW (object);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  switch (prop_id)
  {
  case PROP_XALIGN:
    ol_osd_window_set_alignment (osd, g_value_get_float (value), priv->yalign);
    break;
  case PROP_YALIGN:
    ol_osd_window_set_alignment (osd, g_value_get_float (value), priv->xalign);
    break;
  }
}

static void
ol_osd_window_get_property (GObject *object, guint prop_id,
                                     GValue *value, GParamSpec *pspec)
{
  OlOsdWindow *osd = OL_OSD_WINDOW (object);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (priv);
  switch (prop_id)
  {
  case PROP_XALIGN:
    g_value_set_float (value, priv->xalign);
    break;
  case PROP_YALIGN:
    g_value_set_float (value, priv->yalign);
    break;
  case PROP_LOCKED:
    g_value_set_boolean (value, priv->locked);
  }
}
  
static void
ol_osd_window_init (OlOsdWindow *self)
{
  printf ("init\n");
  GTK_WIDGET_SET_FLAGS (self, GTK_CAN_FOCUS | GTK_RECEIVES_DEFAULT);
  GTK_WIDGET_SET_FLAGS (self, GTK_TOPLEVEL);
  GTK_PRIVATE_SET_FLAG (self, GTK_ANCHORED);
  self->screen = NULL;
  self->event_window = NULL;
  self->lyrics[0] = self->lyrics[1] = NULL;
  self->line_alignment[0] = 0.0;
  self->line_alignment[1] = 1.0;
  self->current_line = 0;
  self->current_percentage = 0.0;

  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (self);
  priv->xalign = priv->yalign = 0.5;
  priv->pressed = FALSE;
  priv->locked = TRUE;
}

GtkWidget*
ol_osd_window_new ()
{
  printf ("new\n");
  OlOsdWindow *osd;
  osd = g_object_new (ol_osd_window_get_type (), NULL);
  return GTK_WIDGET (osd);
}

void
ol_osd_window_set_alignment (OlOsdWindow *osd, float xalign, float yalign)
{
  printf ("%s\n"
          "  xalign %f\n"
          "  yalign %f\n",
          __FUNCTION__,
          xalign,
          yalign
          );
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  GtkWidget *widget = GTK_WIDGET (osd);
  priv->xalign = xalign;
  priv->yalign = yalign;
  if (GTK_WIDGET_REALIZED (osd))
  {
    GtkAllocation alloc;
    ol_osd_window_compute_position (osd, &alloc);
    gtk_widget_size_allocate (widget, &alloc);
  }
}

void
ol_osd_window_get_alignment (OlOsdWindow *osd, float *xalign, float *yalign)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  if (xalign == NULL && yalign == NULL)
    return;
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  if (xalign)
    *xalign = priv->xalign;
  if (yalign)
    *yalign = priv->yalign;
}

void ol_osd_window_resize (OlOsdWindow *osd, gint width, gint height)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  GtkWidget *widget = GTK_WIDGET (osd);
  if (width > 0)
    widget->allocation.width = width;
  if (height > 0)
    widget->allocation.height = height;
  GtkAllocation allo;
  ol_osd_window_compute_position (osd, &allo);
  gtk_widget_size_allocate (widget, &allo);
/*   ol_osd_paint (osd); */
}

void ol_osd_window_get_size (OlOsdWindow *osd, gint *width, gint *height)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  if (width == NULL && width == NULL)
    return;
  gint w, h;
  if (GTK_WIDGET_MAPPED (osd))
  {
    gdk_drawable_get_size (GTK_WIDGET (osd)->window, &w, &h);
  }
  else
  {
    w = GTK_WIDGET (osd)->allocation.width;
    h = GTK_WIDGET (osd)->allocation.height;
  }
  if (width)
    *width = w;
  if (height)
    *height = h;
}

void
ol_osd_window_set_locked (OlOsdWindow *osd, gboolean locked)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  if (priv->locked == locked)
    return;
  priv->locked = locked;
  if (!GTK_WIDGET_MAPPED (GTK_WIDGET (osd)))
    return;
  if (locked)
  {
    gdk_window_hide (osd->event_window);
    gdk_window_hide (osd->bg_window);
  }
  else
  {
    gdk_window_show (osd->bg_window);
    gdk_window_show (GTK_WIDGET (osd)->window);
    gdk_window_show (osd->event_window);
  }
}

static void
ol_osd_window_compute_position (OlOsdWindow *osd, GtkAllocation *alloc)
{
  printf ("%s\n",
          __FUNCTION__);
  if (alloc == NULL)
    return;
  GtkWidget *widget = GTK_WIDGET (osd);
  alloc->x = widget->allocation.x;
  alloc->y = widget->allocation.y;
  alloc->width = widget->allocation.width;
  alloc->height = widget->allocation.height;
  if (osd->screen != NULL)
  {
    gint screen_width, screen_height;
    screen_width = gdk_screen_get_width (osd->screen);
    screen_height = gdk_screen_get_height (osd->screen);
    OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
    printf ("  xalign: %f, yalign: %f\n", priv->xalign, priv->yalign);
    alloc->x = priv->xalign * (screen_width - alloc->width);
    alloc->y = priv->yalign * (screen_height - alloc->height);
  }
}

static void
ol_osd_window_compute_alignment (OlOsdWindow *osd,
                              gint x, gint y,
                              gdouble *xalign, gdouble *yalign)
{
  if (!osd || !OL_IS_OSD_WINDOW (osd))
    return;
  GtkWidget *widget = GTK_WIDGET (osd);
  if (xalign)
  {
    gint screen_width = gdk_screen_get_width (osd->screen);
    *xalign = (double) x / (screen_width - widget->allocation.width);
    if (*xalign < 0.0)
      *xalign = 0;
    else if (*xalign > 1.0)
      *xalign = 1.0;
  }
  if (yalign)
  {
    gint screen_height = gdk_screen_get_height (osd->screen);
    
    *yalign = (double) y / (screen_height - widget->allocation.height);
    if (*yalign < 0.0)
      *yalign = 0;
    else if (*yalign > 1.0)
      *yalign = 1.0;
  }
  printf ("%s\n"
          "  %lf, %lf\n",
          __FUNCTION__,
          *xalign,
          *yalign);
}

static void
ol_osd_window_paint_lyrics (OlOsdWindow *osd,
                         cairo_t *cr)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  g_return_if_fail (cr != NULL);
  gint w, h;
  int width, height;
  gdk_drawable_get_size ( GTK_WIDGET (osd)->window, &w, &h);
  cairo_save (cr);
  cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE); // set drawing compositing operator
                                                 // SOURCE -> replace destination
  cairo_paint(cr); // paint source
  cairo_restore (cr);
  double percentage = osd->current_percentage;
  PangoLayout *layout = pango_cairo_create_layout (cr);
  PangoFontDescription *font_desc = pango_font_description_from_string ("AR PL UKai CN 30");
  pango_layout_set_width (layout, -1);
  pango_layout_set_font_description (layout, font_desc);
  int line;
  gdouble ypos = 0, xpos;
  for (line = 0; line < OL_OSD_WINDOW_MAX_LINE_COUNT; line++)
  {
    /* paint the first lyric */
    if (osd->lyrics[line] != NULL)
    {
      pango_layout_set_text (layout, osd->lyrics[line], -1);
      pango_layout_get_pixel_size (layout, &width, &height);
      xpos = (w - width) * osd->line_alignment[line];
      cairo_move_to (cr, xpos, ypos);
      cairo_set_source_rgb (cr, 0.6, 1.0, 1.0);
      //cairo_set_source(cr, pat0);

     /*show the path*/
      cairo_save (cr);
      pango_cairo_layout_path(cr,layout);
      cairo_set_source_rgb (cr, 0, 0, 0);
      cairo_set_line_width (cr, 1.56);
      cairo_stroke (cr);
      cairo_restore (cr);
      cairo_new_path (cr);
      
      cairo_move_to (cr, xpos, ypos);
      pango_cairo_show_layout (cr, layout);
 
      if (osd->current_line == line)
      {
        cairo_save (cr);
        cairo_pattern_t *pat1 = cairo_pattern_create_linear (xpos, 0.0, xpos + (double)w * percentage, 0.0);
        cairo_pattern_add_color_stop_rgb(pat1, 0, 0, 0, 0);
        cairo_pattern_add_color_stop_rgb(pat1, 0.5, 1, 1, 0);
        cairo_pattern_add_color_stop_rgb(pat1, 1, 0, 0, 0);
  
        cairo_rectangle (cr, xpos, ypos, (double)width * percentage, height);
        cairo_clip (cr);

        cairo_set_source (cr, pat1);
        cairo_move_to (cr, xpos, ypos);
        pango_cairo_show_layout (cr, layout);
        /*show the path
        pango_cairo_layout_path(cr,layout);
        cairo_set_source_rgb (cr, 0, 0, 0);
        cairo_set_line_width (cr, 1.56);
        cairo_stroke (cr);*/
        
        cairo_restore (cr);
      }
      ypos += height;
    }
  }
/*   /\* paint the second text *\/ */
/*   if (osd->lyrics[1] != NULL) */
/*   { */
/*     pango_layout_set_text (layout, osd->lyrics[1], -1); */
/*     pango_layout_get_pixel_size (layout,&width,&height); */
/*     cairo_set_source_rgb (cr, 0.6, 1.0, 1.0); */
/*     cairo_move_to (cr, w - width, height); */
/*     pango_cairo_show_layout (cr, layout); */
/*   } */
  pango_font_description_free (font_desc);
  g_object_unref (layout);
}

void
ol_osd_window_set_current_percentage (OlOsdWindow *osd, double percentage)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  osd->current_percentage = percentage;
/*   ol_osd_window_update_pixmap (osd); */
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

double
ol_osd_window_get_current_percentage (OlOsdWindow *osd)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  return osd->current_percentage;
}

void
ol_osd_window_set_current_line (OlOsdWindow *osd, gint line)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  osd->current_line = line;
}

gint ol_osd_window_get_current_line (OlOsdWindow *osd)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  return osd->current_line;
}

void
ol_osd_window_set_lyric (OlOsdWindow *osd, gint line, const char *lyric)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  g_return_if_fail (lyric != NULL);
  if (line < 0 || line >= OL_OSD_WINDOW_MAX_LINE_COUNT)
    return;
  if (osd->lyrics[line] != NULL)
    g_free (osd->lyrics[line]);
  if (lyric != NULL)
    osd->lyrics[line] = g_strdup (lyric);
  else
    osd->lyrics[line] = NULL;
  /* checks whether all lyrics is empty */
  gboolean is_empty = TRUE;
  int i;
  for (i = 0; i < OL_OSD_WINDOW_MAX_LINE_COUNT; i++)
  {
    if (!ol_is_string_empty (osd->lyrics[line]))
    {
      is_empty = FALSE;
      break;
    }
  }
  if (!is_empty)
  {
    ol_osd_window_update_shape (osd);
    /*   ol_osd_window_update_pixmap (osd); */
    gtk_widget_queue_draw (GTK_WIDGET (osd));
  }
  else
  {
    /* if all lyrics is empty, we simply hide the window */
    /* because there will be problems when the shape mask is empty */
    gtk_widget_hide (GTK_WIDGET (osd));
  }
}

void
ol_osd_window_set_line_alignment (OlOsdWindow *osd, gint line, double alignment)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  if (line < 0 || line >= OL_OSD_WINDOW_MAX_LINE_COUNT)
    return;
  if (alignment < 0.0)
    alignment = 0.0;
  else if (alignment > 1.0)
    alignment = 1.0;
  osd->line_alignment[line] = alignment;
  ol_osd_window_update_shape (osd);
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

static void
ol_osd_window_update_pixmap (OlOsdWindow *osd)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  GtkWidget *widget = GTK_WIDGET (osd);
  g_return_if_fail (GTK_WIDGET_REALIZED (widget));
  gint w, h;
  gdk_drawable_get_size (widget->window, &w, &h);
  cairo_t *cr;
  cr = gdk_cairo_create (widget->window);
  ol_osd_window_paint_lyrics (osd, cr);
  cairo_destroy (cr); 
}

static void
ol_osd_window_update_shape (OlOsdWindow *osd)
{
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  GtkWidget *widget = GTK_WIDGET (osd);
  g_return_if_fail (GTK_WIDGET_REALIZED (widget));
  gint w, h;
  gdk_drawable_get_size (widget->window, &w, &h);
  GdkPixmap *shape_mask = gdk_pixmap_new (widget->window, w, h, 1);
  GdkGC *fg_gc = gdk_gc_new (shape_mask);
/*   gdk_gc_set_colormap (fg_gc, gtk_widget_get_colormap (widget)); */
  GdkColor color;
  color.pixel = 0;
  gdk_gc_set_foreground (fg_gc, &color);
  color.pixel = 0;
  gdk_gc_set_background (fg_gc, &color);
  cairo_t *cr = gdk_cairo_create (shape_mask);
  ol_osd_window_paint_lyrics (osd, cr);
  cairo_destroy (cr); 
  gtk_widget_shape_combine_mask (widget, shape_mask, 0, 0);
  g_object_unref (shape_mask);
}

void
ol_osd_paint (OlOsdWindow *osd, const char* odd_lyric, const char* even_lyric, double percentage)
{
  printf ("paint\n");
  if (osd == NULL)
    return;
  g_return_if_fail (OL_IS_OSD_WINDOW (osd));
  GtkWidget *widget = GTK_WIDGET (osd);
  if (!GTK_WIDGET_REALIZED (widget))
    gtk_widget_realize (widget);
  ol_osd_window_set_lyric (osd, 0, odd_lyric);
  ol_osd_window_set_lyric (osd, 1, even_lyric);
  ol_osd_window_update_shape (osd);
  ol_osd_window_update_pixmap (osd);
}
