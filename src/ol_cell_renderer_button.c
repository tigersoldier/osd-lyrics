#include <gtk/gtkprivate.h>
#include "ol_cell_renderer_button.h"
#include "ol_marshal.h"
#include "ol_debug.h"

static void ol_cell_renderer_button_finalize   (GObject *object);

enum {
  EDITED,
  LAST_SIGNAL
};

enum {
  PROP_0,

  PROP_TEXT,
  /* PROP_MARKUP, */
  /* PROP_ATTRIBUTES, */
  /* PROP_SINGLE_PARAGRAPH_MODE, */
  /* PROP_WIDTH_CHARS, */
  /* PROP_WRAP_WIDTH, */
  /* PROP_ALIGN, */
  
  /* /\* Style args *\/ */
  /* PROP_BACKGROUND, */
  /* PROP_FOREGROUND, */
  /* PROP_BACKGROUND_GDK, */
  /* PROP_FOREGROUND_GDK, */
  /* PROP_FONT, */
  /* PROP_FONT_DESC, */
  /* PROP_FAMILY, */
  /* PROP_STYLE, */
  /* PROP_VARIANT, */
  /* PROP_WEIGHT, */
  /* PROP_STRETCH, */
  /* PROP_SIZE, */
  /* PROP_SIZE_POINTS, */
  /* PROP_SCALE, */
  /* PROP_EDITABLE, */
  /* PROP_STRIKETHROUGH, */
  /* PROP_UNDERLINE, */
  /* PROP_RISE, */
  /* PROP_LANGUAGE, */
  /* PROP_ELLIPSIZE, */
  /* PROP_WRAP_MODE, */
  
  /* /\* Whether-a-style-arg-is-set args *\/ */
  /* PROP_BACKGROUND_SET, */
  /* PROP_FOREGROUND_SET, */
  /* PROP_FAMILY_SET, */
  /* PROP_STYLE_SET, */
  /* PROP_VARIANT_SET, */
  /* PROP_WEIGHT_SET, */
  /* PROP_STRETCH_SET, */
  /* PROP_SIZE_SET, */
  /* PROP_SCALE_SET, */
  /* PROP_EDITABLE_SET, */
  /* PROP_STRIKETHROUGH_SET, */
  /* PROP_UNDERLINE_SET, */
  /* PROP_RISE_SET, */
  /* PROP_LANGUAGE_SET, */
  /* PROP_ELLIPSIZE_SET, */
  /* PROP_ALIGN_SET */
};

static guint cell_renderer_button_signals [LAST_SIGNAL];

#define OL_CELL_RENDERER_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), OL_TYPE_CELL_RENDERER_BUTTON, OlCellRendererButtonPrivate))

typedef struct _OlCellRendererButtonPrivate OlCellRendererButtonPrivate;
struct _OlCellRendererButtonPrivate
{
  gulong focus_out_id;
  PangoLanguage *language;
  PangoEllipsizeMode ellipsize;
  PangoWrapMode wrap_mode;
  PangoAlignment align;
  
  gulong populate_popup_id;
  gulong entry_menu_popdown_timeout;
  gboolean in_entry_menu;
  
  gint width_chars;
  gint wrap_width;
  
  GtkWidget *entry;
};

G_DEFINE_TYPE (OlCellRendererButton, ol_cell_renderer_button, GTK_TYPE_CELL_RENDERER)

static void ol_cell_renderer_button_get_property (GObject *object,
                                           guint param_id,
                                           GValue *value,
                                           GParamSpec *pspec);
static void ol_cell_renderer_button_set_property (GObject *object,
                                           guint param_id,
                                           const GValue *value,
                                           GParamSpec *pspec);
static void ol_cell_renderer_button_get_size (GtkCellRenderer *cell,
                                       GtkWidget *widget,
                                       GdkRectangle *cell_area,
                                       gint *x_offset,
                                       gint *y_offset,
                                       gint *width,
                                       gint *height);
static void ol_cell_renderer_button_render (GtkCellRenderer *cell,
                                     GdkWindow *window,
                                     GtkWidget *widget,
                                     GdkRectangle *background_area,
                                     GdkRectangle *cell_area,
                                     GdkRectangle *expose_area,
                                     GtkCellRendererState flags);

static GtkCellEditable *ol_cell_renderer_button_start_editing (GtkCellRenderer *cell,
                                                        GdkEvent *event,
                                                        GtkWidget *widget,
                                                        const gchar *path,
                                                        GdkRectangle *background_area,
                                                        GdkRectangle *cell_area,
                                                        GtkCellRendererState flags);
static gint ol_cell_renderer_button_activate (GtkCellRenderer *cell,
                                       GdkEvent *event,
                                       GtkWidget *widget,
                                       const gchar *path,
                                       GdkRectangle *background_area,
                                       GdkRectangle *cell_area,
                                       GtkCellRendererState flags);

static PangoLayout* get_layout (OlCellRendererButton *celltext,
                                GtkWidget *widget,
                                gboolean will_render,
                                GtkCellRendererState flags);
static void get_size (GtkCellRenderer *cell,
                      GtkWidget *widget,
                      GdkRectangle *cell_area,
                      PangoLayout *layout,
                      gint *x_offset,
                      gint *y_offset,
                      gint *width,
                      gint *height);

static void
ol_cell_renderer_button_init (OlCellRendererButton *celltext)
{
  OlCellRendererButtonPrivate *priv;

  priv = OL_CELL_RENDERER_BUTTON_GET_PRIVATE (celltext);

  GTK_CELL_RENDERER (celltext)->xalign = 0.0;
  GTK_CELL_RENDERER (celltext)->yalign = 0.5;
  GTK_CELL_RENDERER (celltext)->xpad = 2;
  GTK_CELL_RENDERER (celltext)->ypad = 2;
  GTK_CELL_RENDERER (celltext)->mode = GTK_CELL_RENDERER_MODE_ACTIVATABLE;
  /* celltext->font_scale = 1.0; */
  /* celltext->fixed_height_rows = -1; */
  celltext->font = pango_font_description_new ();
  celltext->stock_id = NULL;

  priv->width_chars = -1;
  priv->wrap_width = -1;
  priv->wrap_mode = PANGO_WRAP_CHAR;
  priv->align = PANGO_ALIGN_LEFT;
  /* priv->align_set = FALSE; */
}

static void
ol_cell_renderer_button_class_init (OlCellRendererButtonClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (class);

  object_class->finalize = ol_cell_renderer_button_finalize;

  object_class->get_property = ol_cell_renderer_button_get_property;
  object_class->set_property = ol_cell_renderer_button_set_property;

  cell_class->get_size = ol_cell_renderer_button_get_size;
  cell_class->render = ol_cell_renderer_button_render;
  cell_class->start_editing = ol_cell_renderer_button_start_editing;
  cell_class->activate = ol_cell_renderer_button_activate;

  g_object_class_install_property (object_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        ("Text"),
                                                        ("Text to render"),
                                                        NULL,
                                                        GTK_PARAM_READWRITE));


  cell_renderer_button_signals [EDITED] =
    g_signal_new (("edited"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (OlCellRendererButtonClass, edited),
                  NULL, NULL,
                  ol_marshal_VOID__STRING_STRING,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  G_TYPE_STRING);

  g_type_class_add_private (object_class, sizeof (OlCellRendererButtonPrivate));
}

static void
ol_cell_renderer_button_finalize (GObject *object)
{
  OlCellRendererButton *celltext = OL_CELL_RENDERER_BUTTON (object);
  OlCellRendererButtonPrivate *priv;

  priv = OL_CELL_RENDERER_BUTTON_GET_PRIVATE (object);

  pango_font_description_free (celltext->font);

  g_free (celltext->text);

  G_OBJECT_CLASS (ol_cell_renderer_button_parent_class)->finalize (object);
}


static void
ol_cell_renderer_button_get_property (GObject        *object,
                               guint           param_id,
                               GValue         *value,
                               GParamSpec     *pspec)
{
  OlCellRendererButton *celltext = OL_CELL_RENDERER_BUTTON (object);
  OlCellRendererButtonPrivate *priv;

  priv = OL_CELL_RENDERER_BUTTON_GET_PRIVATE (object);

  switch (param_id)
  {
  case PROP_TEXT:
    g_value_set_string (value, celltext->text);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
    break;
  }
}

static void
ol_cell_renderer_button_set_property (GObject *object,
                               guint param_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
  OlCellRendererButton *celltext = OL_CELL_RENDERER_BUTTON (object);
  OlCellRendererButtonPrivate *priv;

  priv = OL_CELL_RENDERER_BUTTON_GET_PRIVATE (object);

  switch (param_id)
  {
  case PROP_TEXT:
    g_free (celltext->text);

    /* if (priv->markup_set) */
    /* { */
    /*   if (celltext->extra_attrs) */
    /*     pango_attr_list_unref (celltext->extra_attrs); */
    /*   celltext->extra_attrs = NULL; */
    /*   priv->markup_set = FALSE; */
    /* } */

    celltext->text = g_strdup (g_value_get_string (value));
    g_object_notify (object, "text");
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
    break;
  }
}

GtkCellRenderer *
ol_cell_renderer_button_new (void)
{
  return g_object_new (OL_TYPE_CELL_RENDERER_BUTTON, NULL);
}

static void
get_icon_rect (GtkWidget *widget,
               GdkRectangle *cell_area,
               GdkPixbuf *icon,
               int index,
               int *x_offset, int *y_offset, int *width, int *height)
{
  if (icon == NULL)
    icon = gtk_widget_render_icon (widget,
                                   GTK_STOCK_REMOVE,
                                   GTK_ICON_SIZE_SMALL_TOOLBAR,
                                   NULL);
  else
    g_object_ref (icon);
  gint icon_width, icon_height;
  icon_width = gdk_pixbuf_get_width (icon);
  icon_height = gdk_pixbuf_get_height (icon);
  if (x_offset)
    *x_offset = cell_area->width - icon_width * (index + 1);
  if (y_offset)
    *y_offset = (cell_area->height - icon_height) / 2;
  if (width)
    *width = icon_width;
  if (height)
    *height = icon_height;
  g_object_unref (icon);
}

static void
ol_cell_renderer_button_render (GtkCellRenderer      *cell,
                         GdkDrawable          *window,
                         GtkWidget            *widget,
                         GdkRectangle         *background_area,
                         GdkRectangle         *cell_area,
                         GdkRectangle         *expose_area,
                         GtkCellRendererState  flags)

{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  OlCellRendererButton *celltext = (OlCellRendererButton *) cell;
  PangoLayout *layout;
  GtkStateType state;
  gint x_offset;
  gint y_offset;
  OlCellRendererButtonPrivate *priv;

  priv = OL_CELL_RENDERER_BUTTON_GET_PRIVATE (cell);

  layout = get_layout (celltext, widget, TRUE, flags);
  get_size (cell, widget, cell_area, layout, &x_offset, &y_offset, NULL, NULL);
  /* ol_logf (OL_DEBUG, "  offset: (%d,%d)\n", x_offset, y_offset); */
  if (!cell->sensitive) 
  {
    state = GTK_STATE_INSENSITIVE;
  }
  else if ((flags & GTK_CELL_RENDERER_SELECTED) == GTK_CELL_RENDERER_SELECTED)
  {
    if (GTK_WIDGET_HAS_FOCUS (widget))
      state = GTK_STATE_SELECTED;
    else
      state = GTK_STATE_ACTIVE;
  }
  else if ((flags & GTK_CELL_RENDERER_PRELIT) == GTK_CELL_RENDERER_PRELIT &&
	   GTK_WIDGET_STATE (widget) == GTK_STATE_PRELIGHT)
  {
    state = GTK_STATE_PRELIGHT;
  }
  else
  {
    if (GTK_WIDGET_STATE (widget) == GTK_STATE_INSENSITIVE)
      state = GTK_STATE_INSENSITIVE;
    else
      state = GTK_STATE_NORMAL;
  }

  pango_layout_set_width (layout, 
                          (cell_area->width - x_offset - 2 * cell->xpad) * PANGO_SCALE);

  gtk_paint_layout (widget->style,
                    window,
                    state,
        	    TRUE,
                    expose_area,
                    widget,
                    "cellrenderertext",
                    cell_area->x + x_offset + cell->xpad,
                    cell_area->y + y_offset + cell->ypad,
                    layout);
  g_object_unref (layout);
  /* if (state == GTK_STATE_SELECTED | */
  /*     state == GTK_STATE_PRELIGHT) */
  {
    GdkPixbuf *remove_icon = gtk_widget_render_icon (widget,
                                                     GTK_STOCK_REMOVE,
                                                     GTK_ICON_SIZE_SMALL_TOOLBAR,
                                                     NULL);
    gint icon_width, icon_height, icon_x, icon_y;
    get_icon_rect (widget,
                   cell_area,
                   remove_icon,
                   0,
                   &icon_x, &icon_y, &icon_width, &icon_height);
    gdk_draw_pixbuf (window,
                     widget->style->black_gc,
                     remove_icon,
                     0,           /* src_x */
                     0,           /* src_y */
                     cell_area->x + icon_x - cell->xpad,
                     cell_area->y + icon_y,
                     icon_width,
                     icon_height,
                     GDK_RGB_DITHER_NORMAL,
                     0, 0);

    g_object_unref (remove_icon);
  }
}

static void
add_attr (PangoAttrList  *attr_list,
          PangoAttribute *attr)
{
  attr->start_index = 0;
  attr->end_index = G_MAXINT;
  
  pango_attr_list_insert (attr_list, attr);
}

static PangoLayout*
get_layout (OlCellRendererButton *celltext,
            GtkWidget           *widget,
            gboolean             will_render,
            GtkCellRendererState flags)
{
  /* ol_logf (OL_DEBUG, "%s\n", __FUNCTION__); */
  PangoAttrList *attr_list;
  PangoLayout *layout;
  OlCellRendererButtonPrivate *priv;

  priv = OL_CELL_RENDERER_BUTTON_GET_PRIVATE (celltext);
  
  layout = gtk_widget_create_pango_layout (widget, celltext->text);
  attr_list = pango_attr_list_new ();
  pango_layout_set_single_paragraph_mode (layout, TRUE);

  if (will_render)
  {
    add_attr (attr_list, pango_attr_font_desc_new (celltext->font));
  }

  pango_layout_set_ellipsize (layout, priv->ellipsize);

  if (priv->wrap_width != -1)
  {
    pango_layout_set_width (layout, priv->wrap_width * PANGO_SCALE);
    /* ol_logf (OL_DEBUG, "  width: %d\n", (int)(priv->wrap_width * PANGO_SCALE)); */
    pango_layout_set_wrap (layout, priv->wrap_mode);
  }
  else
  {
    pango_layout_set_width (layout, -1);
    pango_layout_set_wrap (layout, PANGO_WRAP_CHAR);
  }

  PangoAlignment align;

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    align = PANGO_ALIGN_RIGHT;
  else
    align = PANGO_ALIGN_LEFT;

  pango_layout_set_attributes (layout, attr_list);

  pango_attr_list_unref (attr_list);
  
  return layout;
}

static void
get_size (GtkCellRenderer *cell,
	  GtkWidget       *widget,
	  GdkRectangle    *cell_area,
	  PangoLayout     *layout,
	  gint            *x_offset,
	  gint            *y_offset,
	  gint            *width,
	  gint            *height)
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  OlCellRendererButton *celltext = (OlCellRendererButton *) cell;
  PangoRectangle rect;
  OlCellRendererButtonPrivate *priv;

  priv = OL_CELL_RENDERER_BUTTON_GET_PRIVATE (cell);

  PangoContext *context;
  PangoFontMetrics *metrics;
  PangoFontDescription *font_desc;
  gint row_height;

  font_desc = pango_font_description_copy_static (widget->style->font_desc);
  pango_font_description_merge_static (font_desc, celltext->font, TRUE);

  context = gtk_widget_get_pango_context (widget);

  metrics = pango_context_get_metrics (context,
                                       font_desc,
                                       pango_context_get_language (context));
  row_height = (pango_font_metrics_get_ascent (metrics) +
                pango_font_metrics_get_descent (metrics));
  pango_font_metrics_unref (metrics);

  pango_font_description_free (font_desc);

  gtk_cell_renderer_set_fixed_size (cell,
                                    cell->width, 2*cell->ypad +
                                    PANGO_PIXELS (row_height));
  ol_logf (OL_DEBUG,
           "  size: %dx%d\n",
           cell->width,
           2*cell->ypad +
           PANGO_PIXELS (row_height));

  if (layout)
    g_object_ref (layout);
  else
    layout = get_layout (celltext, widget, FALSE, 0);

  pango_layout_get_pixel_extents (layout, NULL, &rect);
  if (height)
    *height = cell->ypad * 2 + rect.height;

  if (width)
    *width = cell->xpad * 2 + rect.x + rect.width;

  if (cell_area)
  {
    ol_logf (OL_DEBUG, "  cell_area:%dx%d\n", cell_area->width, cell_area->height);
    if (x_offset)
    {
      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        *x_offset = (1.0 - cell->xalign) * (cell_area->width - (rect.x + rect.width + (2 * cell->xpad)));
      else 
        *x_offset = cell->xalign * (cell_area->width - (rect.x + rect.width + (2 * cell->xpad)));

      *x_offset = MAX(*x_offset, 0);
    }
    if (y_offset)
    {
      *y_offset = cell->yalign * (cell_area->height - (rect.height + (2 * cell->ypad)));
      *y_offset = MAX (*y_offset, 0);
    }
  }
  else
  {
    if (x_offset) *x_offset = 0;
    if (y_offset) *y_offset = 0;
  }

  g_object_unref (layout);
}


static void
ol_cell_renderer_button_get_size (GtkCellRenderer *cell,
                           GtkWidget       *widget,
                           GdkRectangle    *cell_area,
                           gint            *x_offset,
                           gint            *y_offset,
                           gint            *width,
                           gint            *height)
{
  get_size (cell, widget, cell_area, NULL,
	    x_offset, y_offset, width, height);
}

static GtkCellEditable *
ol_cell_renderer_button_start_editing (GtkCellRenderer *cell,
                                GdkEvent *event,
                                GtkWidget *widget,
                                const gchar *path,
                                GdkRectangle *background_area,
                                GdkRectangle *cell_area,
                                GtkCellRendererState flags)
{
  ol_logf (OL_DEBUG,
           "%s\n",
           __FUNCTION__);
  return NULL;
}

static gint
ol_cell_renderer_button_activate (GtkCellRenderer      *cell,
                           GdkEvent             *event,
                           GtkWidget            *widget,
                           const gchar          *path,
                           GdkRectangle         *background_area,
                           GdkRectangle         *cell_area,
                           GtkCellRendererState  flags)
{
  ol_logf (OL_DEBUG,
           "%s\n"
           "  event type:%d\n",
           __FUNCTION__,
           event->type);
  ol_logf (OL_INFO,
           "  bg area: (%d,%d) %dx%d\n"
           "  cell area: (%d,%d) %dx%d\n",
           background_area->x, background_area->y,
           background_area->width, background_area->height,
           cell_area->x, cell_area->y,
           cell_area->width, cell_area->height);
  if (event->type == GDK_BUTTON_PRESS)
  {
    GdkEventButton button = event->button;
    ol_logf (OL_INFO,
             "  pointer (%d,%d)\n",
             (int)button.x, (int)button.y);
    GdkPixbuf *remove_icon = gtk_widget_render_icon (widget,
                                                     GTK_STOCK_REMOVE,
                                                     GTK_ICON_SIZE_SMALL_TOOLBAR,
                                                     NULL);
    gint icon_width, icon_height;
    icon_width = gdk_pixbuf_get_width (remove_icon);
    icon_height = gdk_pixbuf_get_height (remove_icon);
    g_object_unref (remove_icon);
  }
  return FALSE;
}
