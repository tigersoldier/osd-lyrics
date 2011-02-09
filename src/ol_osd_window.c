/*
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
 */
#include <math.h>
#include "ol_osd_window.h"
#include "ol_utils.h"
#include "ol_debug.h"

#define OL_OSD_WINDOW_GET_PRIVATE(obj)   (G_TYPE_INSTANCE_GET_PRIVATE  \
                                          ((obj),                      \
                                           ol_osd_window_get_type (),  \
                                           OlOsdWindowPrivate))

enum {
  PROP_0,
  PROP_LOCKED,
};

static const OlColor DEFAULT_INACTIVE_COLORS[OL_LINEAR_COLOR_COUNT]= {
  {0.6, 1.0, 1.0},
  {0.0, 0.0, 1.0},
  {0.6, 1.0, 1.0},
};
static const OlColor DEFAULT_ACTIVE_COLORS[OL_LINEAR_COLOR_COUNT]= {
  {0.4, 0.15, 0.0},
  {1.0, 1.0, 0.0},
  {1.0, 0.5, 0.0},
};
static const int MOUSE_TIMER_INTERVAL = 100;

static const int DEFAULT_LINE_HEIGHT = 45;
static const int DEFAULT_HEIGHT = 100;
static const double LINE_PADDING = 0.1;
static const int BORDER_WIDTH = 5;
static const int DEFAULT_WIDTH = 1024;

enum DragState
{
  DRAG_NONE,
  DRAG_MOVE,
  DRAG_EAST,
  DRAG_WEST,
};

typedef struct __OlOsdWindowPrivate OlOsdWindowPrivate;
struct __OlOsdWindowPrivate
{
  gint width;
  gint osd_height;
  gint raw_x;
  gint raw_y;
  gboolean locked;
  gboolean pressed;
  gboolean composited;          /* whether the screen is composited */
  gulong composited_signal;
  gint mouse_x;
  gint mouse_y;
  gint old_x;
  gint old_y;
  gint old_width;
  gboolean visible;
  guint mouse_timer_id;
  guint drag_timer_id;
  gboolean mouse_over;
  gboolean mouse_over_lyrics;
  GtkRequisition child_requisition;
  enum OlOsdWindowMode mode;
  enum DragState drag_state;
};

struct OsdLrc
{
  gint id;
  gchar *lyric;
};

G_DEFINE_TYPE(OlOsdWindow, ol_osd_window, GTK_TYPE_WINDOW)
/** 
 * @brief Destroys an OSD Window
 * 
 * @param widget The OSD Window to destroy
 */
static void ol_osd_window_destroy (GtkObject *object);
static void ol_osd_window_set_property (GObject *object, guint prop_id,
                                        const GValue *value, GParamSpec *pspec);
static void ol_osd_window_get_property (GObject *object, guint prop_id,
                                        GValue *value, GParamSpec *pspec);
static void ol_osd_window_size_allocate (GtkWidget *widget,
                                         GtkAllocation *allocation);
static void ol_osd_window_size_request (GtkWidget *widget,
                                        GtkRequisition *requisition);
static gboolean ol_osd_window_panel_visible (OlOsdWindow *osd);
static gboolean ol_osd_window_expose_before (GtkWidget *widget, GdkEventExpose *event);
static gboolean ol_osd_window_expose_after (GtkWidget *widget, GdkEventExpose *event);
static gboolean ol_osd_window_paint_bg (OlOsdWindow *osd, cairo_t *cr);
static gboolean ol_osd_window_enter_notify (GtkWidget *widget, GdkEventCrossing *event);
static gboolean ol_osd_window_leave_notify (GtkWidget *widget, GdkEventCrossing *event);
static gboolean ol_osd_window_button_press (GtkWidget *widget, GdkEventButton *event);
static gboolean ol_osd_window_motion_notify (GtkWidget *widget, GdkEventMotion *event);
static gboolean ol_osd_window_button_release (GtkWidget *widget, GdkEventButton *event);
static gboolean ol_osd_window_configure_event (GtkWindow *window,
                                               GdkEventConfigure *event,
                                               gpointer user_data);
static gboolean ol_osd_window_map_cb (GtkWidget *widget,
                                      GdkEvent  *event,
                                      gpointer   user_data);
static gboolean ol_osd_window_unmap_cb (GtkWidget *widget,
                                        GdkEvent  *event,
                                        gpointer   user_data);

static gboolean ol_osd_window_has_lyrics (OlOsdWindow *osd);
static int ol_osd_window_compute_osd_height (OlOsdWindow *osd);
static int ol_osd_window_compute_window_height (OlOsdWindow *osd);
static double ol_osd_window_compute_lyric_xpos (OlOsdWindow *osd,
                                                int line,
                                                double percentage);
static int ol_osd_window_compute_lyric_ypos (OlOsdWindow *osd);
static void ol_osd_window_paint_lyrics (OlOsdWindow *osd, cairo_t *cr);
static void ol_osd_window_emit_move (OlOsdWindow *osd);
static void ol_osd_window_emit_resize (OlOsdWindow *osd);
static GdkWindowEdge ol_osd_window_get_edge_on_point (OlOsdWindow *osd,
                                                           gint x,
                                                           gint y);
static void ol_osd_window_update_child_size_requisition (OlOsdWindow *osd);
static void ol_osd_window_update_layout (OlOsdWindow *osd);
static void ol_osd_window_move_resize (OlOsdWindow *osd,
                                       int raw_x,
                                       int raw_y,
                                       int width,
                                       enum DragState drag_type);
/** 
 * @brief Paint the layout to be OSD style
 *
 * Paint the lyrics, the odd_lrc will be shown upper-left and the even_lrc will be shown bottom-right
 * @param osd An OlOsdWindow
 * @param cr A cairo_t to be painted
 */
static void ol_osd_window_paint (OlOsdWindow *osd);
static void ol_osd_window_clear_cairo (cairo_t *cr);
static void ol_osd_window_set_input_shape_mask (OlOsdWindow *osd,
                                                gboolean disable_input);
static void ol_osd_draw_lyric_pixmap (OlOsdWindow *osd,
                                      GdkPixmap **pixmap,
                                      const char *lyric);
static void ol_osd_window_update_lyric_pixmap (OlOsdWindow *osd, int line);
static void ol_osd_window_update_lyric_rect (OlOsdWindow *osd, int line);
static void ol_osd_window_screen_composited_changed (GdkScreen *screen,
                                                     gpointer userdata);
static gboolean ol_osd_window_mouse_timer (gpointer data);
static void ol_osd_window_update_child_allocation (OlOsdWindow *osd);
static void ol_osd_window_check_resize (GtkContainer *container);
static void ol_osd_window_check_mouse_leave (OlOsdWindow *osd);
static void ol_osd_window_update_child_visibility (OlOsdWindow *osd);

static void _paint_rect (cairo_t *cr, GdkPixbuf *source,
                         double src_x, double src_y,
                         double src_w, double src_h,
                         double des_x, double des_y,
                         double dex_w, double des_h);
static gboolean _point_in_rect (int x, int y, GdkRectangle *rect);

static void
_paint_rect (cairo_t *cr, GdkPixbuf *source,
             double src_x, double src_y,
             double src_w, double src_h,
             double des_x, double des_y,
             double des_w, double des_h)
{
  ol_assert (cr != NULL);
  ol_assert (source != NULL);
  cairo_save (cr);
  double sw = des_w / src_w;
  double sh = des_h / src_h;
  cairo_translate (cr, des_x, des_y);
  cairo_rectangle (cr, 0, 0,
                   des_w, des_h);
  cairo_scale (cr, sw, sh);
  cairo_clip (cr);
  gdk_cairo_set_source_pixbuf (cr,
                               source,
                               -src_x,
                               -src_y);
  cairo_paint (cr);
  cairo_restore (cr);
}

static gboolean
ol_osd_window_paint_bg (OlOsdWindow *osd, cairo_t *cr)
{
  ol_log_func ();
  ol_assert_ret (OL_IS_OSD_WINDOW (osd), FALSE);
  GtkWidget *widget = GTK_WIDGET (osd);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  int w, h;
  gdk_drawable_get_size (widget->window, &w, &h);
  ol_osd_window_clear_cairo (cr);
  if (ol_osd_window_panel_visible(osd))
  {
    if (osd->bg_pixbuf != NULL)
    {
      int sw, sh;
      if (priv->composited)
      {
        ol_osd_window_clear_cairo (cr);
      }
      cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
      sw = gdk_pixbuf_get_width (osd->bg_pixbuf);
      sh = gdk_pixbuf_get_height (osd->bg_pixbuf);
      _paint_rect (cr, osd->bg_pixbuf,
                   0, 0, BORDER_WIDTH, BORDER_WIDTH,
                   0, 0, BORDER_WIDTH, BORDER_WIDTH);
      _paint_rect (cr, osd->bg_pixbuf,
                   0, sh - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH,
                   0, h - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH);
      _paint_rect (cr, osd->bg_pixbuf,
                   sw - BORDER_WIDTH, 0, BORDER_WIDTH, BORDER_WIDTH,
                   w - BORDER_WIDTH, 0, BORDER_WIDTH, BORDER_WIDTH);
      _paint_rect (cr, osd->bg_pixbuf,
                   sw - BORDER_WIDTH, sh - BORDER_WIDTH,
                   BORDER_WIDTH, BORDER_WIDTH,
                   w - BORDER_WIDTH, h - BORDER_WIDTH,
                   BORDER_WIDTH, BORDER_WIDTH);
      _paint_rect (cr, osd->bg_pixbuf,
                   0, BORDER_WIDTH, BORDER_WIDTH, sh - BORDER_WIDTH * 2,
                   0, BORDER_WIDTH, BORDER_WIDTH, h - BORDER_WIDTH * 2);
      _paint_rect (cr, osd->bg_pixbuf,
                   sw - BORDER_WIDTH, BORDER_WIDTH,
                   BORDER_WIDTH, sh - BORDER_WIDTH * 2,
                   w - BORDER_WIDTH, BORDER_WIDTH,
                   BORDER_WIDTH, h - BORDER_WIDTH * 2);
      _paint_rect (cr, osd->bg_pixbuf,
                   BORDER_WIDTH, 0, sw - BORDER_WIDTH * 2, BORDER_WIDTH,
                   BORDER_WIDTH, 0, w - BORDER_WIDTH * 2, BORDER_WIDTH);
      _paint_rect (cr, osd->bg_pixbuf,
                   BORDER_WIDTH, sh - BORDER_WIDTH,
                   sw - BORDER_WIDTH * 2, BORDER_WIDTH,
                   BORDER_WIDTH, h - BORDER_WIDTH,
                   w - BORDER_WIDTH * 2, BORDER_WIDTH);
      _paint_rect (cr, osd->bg_pixbuf,
                   BORDER_WIDTH, BORDER_WIDTH,
                   sw - BORDER_WIDTH * 2, sh - BORDER_WIDTH * 2,
                   BORDER_WIDTH, BORDER_WIDTH,
                   w - BORDER_WIDTH * 2, h - BORDER_WIDTH * 2);
    
    }
    else
    {
      gtk_paint_box (widget->style,
                     widget->window,
                     GTK_STATE_NORMAL, GTK_SHADOW_IN,
                     NULL, widget, "buttondefault",
                     0, 0, w, h);
    
    }
  }
}

static void
ol_osd_window_screen_composited_changed (GdkScreen *screen, gpointer userdata)
{
  ol_log_func ();
  ol_assert (OL_IS_OSD_WINDOW (userdata));
  GtkWidget *widget = GTK_WIDGET (userdata);
  OlOsdWindow *osd = OL_OSD_WINDOW (userdata);
  gboolean mapped = GTK_WIDGET_MAPPED (widget);
  GdkColormap* colormap = gdk_screen_get_rgba_colormap (screen);
  if (colormap == NULL)
    colormap = gdk_screen_get_rgb_colormap (screen);
  gtk_widget_set_colormap (widget, colormap);
  if (mapped)
    gtk_widget_unmap (widget);
  if (GTK_WIDGET_REALIZED (widget))
  {
    gtk_widget_unrealize (widget);
    gtk_widget_realize (widget);
  }
  if (mapped)
    gtk_widget_map (widget);
}

static gboolean
ol_osd_window_use_system_drag (OlOsdWindow *osd)
{
  ol_assert_ret (OL_IS_OSD_WINDOW (osd), FALSE);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  return priv->mode == OL_OSD_WINDOW_NORMAL;
}

static gboolean
ol_osd_window_button_press (GtkWidget *widget, GdkEventButton *event)
{
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  if (GTK_WIDGET_CLASS (ol_osd_window_parent_class)->button_press_event &&
      GTK_WIDGET_CLASS (ol_osd_window_parent_class)->button_press_event (widget,
                                                                         event))
      return TRUE;
  ol_log_func ();
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  if (event->button == 1 && !priv->locked)
  {
    int edge = ol_osd_window_get_edge_on_point (osd, event->x, event->y);
    if (ol_osd_window_use_system_drag (osd)) {
      switch (edge) {
      case GDK_WINDOW_EDGE_EAST:
      case GDK_WINDOW_EDGE_WEST:
        gtk_window_begin_resize_drag (GTK_WINDOW (widget),
                                      edge,
                                      event->button,
                                      event->x_root,
                                      event->y_root,
                                      event->time);
        break;
      default:
        gtk_window_begin_move_drag (GTK_WINDOW (widget),
                                    event->button,
                                    event->x_root,
                                    event->y_root,
                                    event->time);
      }
    } else {
      priv->pressed = TRUE;
      priv->old_x = widget->allocation.x;
      priv->old_y = widget->allocation.y;
      priv->old_width = widget->allocation.width;
      priv->mouse_x = event->x_root;
      priv->mouse_y = event->y_root;
      switch (edge) {
      case GDK_WINDOW_EDGE_EAST:
        priv->drag_state = DRAG_EAST;
        break;
      case GDK_WINDOW_EDGE_WEST:
        priv->drag_state = DRAG_WEST;
        break;
      default:
        priv->drag_state = DRAG_MOVE;
        break;
      }
    }
  }
  return FALSE;
}

static void
ol_osd_window_emit_move (OlOsdWindow *osd)
{
  GValue params[1] = {0};
  g_value_init (&params[0], G_OBJECT_TYPE (osd));
  g_value_set_object (&params[0], G_OBJECT (osd));
  g_signal_emitv (params,
                  OL_OSD_WINDOW_GET_CLASS (osd)->signals[OSD_MOVED],
                  0,
                  NULL);
}

static void
ol_osd_window_emit_resize (OlOsdWindow *osd)
{
  ol_errorf ("resize: %d\n", ol_osd_window_get_width (osd));
  GValue params[1] = {0};
  g_value_init (&params[0], G_OBJECT_TYPE (osd));
  g_value_set_object (&params[0], G_OBJECT (osd));
  g_signal_emitv (params,
                  OL_OSD_WINDOW_GET_CLASS (osd)->signals[OSD_RESIZE],
                  0,
                  NULL);
}

static gboolean
ol_osd_window_button_release (GtkWidget *widget, GdkEventButton *event)
{
  /* ol_log_func (); */
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  if (priv->drag_state != DRAG_NONE)
  {
    ol_osd_window_emit_move (osd);
    ol_osd_window_emit_resize (osd);
  }
  priv->drag_state = DRAG_NONE;
  return FALSE;
}

static gboolean ol_osd_window_configure_event (GtkWindow *window,
                                               GdkEventConfigure *event,
                                               gpointer user_data) {
  ol_assert_ret (OL_IS_OSD_WINDOW (window), FALSE);
  GtkWidget *widget = GTK_WIDGET (window);
  OlOsdWindow *osd = OL_OSD_WINDOW (window);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  gboolean width_changed = FALSE;
  widget->allocation.x = event->x;
  widget->allocation.y = event->y;
  widget->allocation.height = event->height;
  widget->allocation.width = event->width;
  if (priv->width != event->width) {
    priv->width = event->width;
    width_changed = TRUE;
    if (priv->drag_state == DRAG_NONE)
      ol_osd_window_emit_resize (osd);
    /* ol_osd_window_set_width (osd, event->width); */
  }
  if (priv->raw_x != event->x || priv->raw_y != event->y)
  {
    priv->raw_x = event->x;
    priv->raw_y = event->y;
    if (priv->drag_state == DRAG_NONE)
      ol_osd_window_emit_move (osd);
  }
  return FALSE;
}

static GdkWindowEdge
ol_osd_window_get_edge_on_point (OlOsdWindow *osd,
                                 gint x,
                                 gint y) {
  ol_assert (OL_IS_OSD_WINDOW (osd));
  GtkWidget *widget = GTK_WIDGET (osd);
  gint width = widget->allocation.width;
  gint height = widget->allocation.height;
  if (y >= 0 && y <= height) {
    if (x >= 0 && x < BORDER_WIDTH)
      return GDK_WINDOW_EDGE_WEST;
    if (x >= width - BORDER_WIDTH && x < width)
      return GDK_WINDOW_EDGE_EAST;
  }
  return -1;
}

static gboolean
ol_osd_window_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
  ol_log_func ();
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  int x = priv->old_x + (event->x_root - priv->mouse_x);
  int y = priv->old_y + (event->y_root - priv->mouse_y);
  switch (priv->drag_state)
  {
  case DRAG_MOVE:
    ol_osd_window_move (osd, x, y);
    break;
  case DRAG_EAST:
    ol_osd_window_move_resize (osd, priv->old_x, priv->old_y,
                               priv->old_width + (event->x_root - priv->mouse_x),
                               DRAG_EAST);
    break;
  case DRAG_WEST:
    ol_osd_window_move_resize (osd, x, priv->old_y,
                               priv->old_width + priv->old_x - x, DRAG_WEST);
                               
    break;
  case DRAG_NONE:
  {
    int edge = ol_osd_window_get_edge_on_point (osd,
                                                event->x,
                                                event->y);
    GdkCursor *cursor = NULL;
    switch (edge) {
    case GDK_WINDOW_EDGE_EAST:
        cursor = gdk_cursor_new (GDK_RIGHT_SIDE);
        break;
    case GDK_WINDOW_EDGE_WEST:
      cursor = gdk_cursor_new (GDK_LEFT_SIDE);
      break;
    }
    gdk_window_set_cursor (widget->window,
                           cursor);
    if (cursor)
      gdk_cursor_unref (cursor);
    break;
  }
  default:
    break;
  }
  return FALSE;
}

static gboolean ol_osd_window_panel_visible (OlOsdWindow *osd) {
  ol_assert (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  return priv->mode == OL_OSD_WINDOW_NORMAL ||
    (priv->locked == FALSE && priv->mouse_over);
}

static gboolean
ol_osd_window_expose_before (GtkWidget *widget, GdkEventExpose *event) {
  ol_assert (OL_IS_OSD_WINDOW (widget));
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  if (ol_osd_window_panel_visible (osd))
    ol_osd_window_paint (osd);
  return FALSE;
}

static gboolean
ol_osd_window_expose_after (GtkWidget *widget, GdkEventExpose *event)
{
  ol_assert_ret (OL_IS_OSD_WINDOW (widget), FALSE);
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  if (!ol_osd_window_panel_visible (osd))
    ol_osd_window_paint (osd);
  return FALSE;
}

static void
ol_osd_window_paint (OlOsdWindow *osd) {
  ol_assert (OL_IS_OSD_WINDOW (osd));
  cairo_t *cr;
  cr = gdk_cairo_create (GTK_WIDGET (osd)->window);
  ol_osd_window_paint_bg (osd, cr);
  ol_osd_window_paint_lyrics (osd, cr);
  cairo_destroy (cr);
}

static gboolean
ol_osd_window_enter_notify (GtkWidget *widget, GdkEventCrossing *event)
{
  ol_assert_ret (OL_IS_OSD_WINDOW (widget), FALSE);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  priv->mouse_over = TRUE;
  gtk_widget_queue_draw (widget);
  /* ol_osd_window_update_child_visibility (OL_OSD_WINDOW (widget)); */
  return FALSE;
}

static void
ol_osd_window_check_mouse_leave (OlOsdWindow *osd)
{
  ol_assert (osd != NULL);
  GtkWidget *widget = GTK_WIDGET (osd);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  if (!gdk_window_is_visible (widget->window))
    return;
  gint rel_x, rel_y;
  gdk_window_get_pointer (widget->window, &rel_x, &rel_y, NULL);
  GdkRectangle rect;
  rect.x = 0; rect.y = 0;
  rect.width = widget->allocation.width;
  rect.height = widget->allocation.height;
  if ((priv->mode != OL_OSD_WINDOW_DOCK ||
       !priv->pressed) &&
      !_point_in_rect (rel_x, rel_y, &rect))
  {
    priv->mouse_over = FALSE;
    gtk_widget_queue_draw (widget);
  }
}

static gboolean
ol_osd_window_leave_notify (GtkWidget *widget, GdkEventCrossing *event)
{
  ol_assert_ret (OL_IS_OSD_WINDOW (widget), FALSE);
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  ol_osd_window_check_mouse_leave (osd);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  return FALSE;
}

static gboolean
ol_osd_window_map_cb (GtkWidget *widget,
                      GdkEvent  *event,
                      gpointer   user_data) {
  ol_assert_ret (OL_IS_OSD_WINDOW (widget), FALSE);
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  if (priv->mouse_timer_id == 0)
  {
    priv->mouse_timer_id = g_timeout_add (MOUSE_TIMER_INTERVAL,
                                          (GSourceFunc) ol_osd_window_mouse_timer,
                                          widget);
  }
  return FALSE;
}

static gboolean
ol_osd_window_unmap_cb (GtkWidget *widget,
                      GdkEvent  *event,
                      gpointer   user_data) {
  ol_assert_ret (OL_IS_OSD_WINDOW (widget), FALSE);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (widget);
  if (priv->mouse_timer_id != 0)
  {
    g_source_remove (priv->mouse_timer_id);
  }
  return FALSE;
}

static void
ol_osd_window_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  widget->allocation = *allocation;
  int h = ol_osd_window_compute_window_height (osd);
  widget->allocation.height = h;

  /* priv->width = allocation->width; */
  if (GTK_WIDGET_REALIZED (widget))
  {
    gdk_window_move_resize (widget->window,
                            widget->allocation.x,
                            widget->allocation.y,
                            widget->allocation.width,
                            widget->allocation.height);
  }
  ol_osd_window_update_child_allocation (osd);
}

static void
ol_osd_window_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
  ol_error ("req");
  OlOsdWindow *osd = OL_OSD_WINDOW (widget);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  ol_osd_window_update_layout (osd);
  requisition->width = priv->width;
  requisition->height = ol_osd_window_compute_window_height (osd);
}

static void
ol_osd_window_update_layout (OlOsdWindow *osd)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  priv->osd_height = ol_osd_window_compute_osd_height (osd);
  ol_osd_window_update_child_size_requisition (osd);
}

static gboolean
_point_in_rect (int x, int y, GdkRectangle *rect)
{
  ol_assert_ret (rect != NULL, FALSE);
  return x < rect->x + rect->width &&
    y < rect->y + rect->height &&
    x >= rect->x &&
    y >= rect->y;
}

static gboolean
ol_osd_window_mouse_timer (gpointer data)
{
  /* ol_log_func (); */
  ol_assert_ret (data != NULL, FALSE);
  ol_assert_ret (OL_IS_OSD_WINDOW (data), FALSE);
  OlOsdWindow *osd = OL_OSD_WINDOW (data);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  GtkWidget *widget = GTK_WIDGET (osd);
  if (GTK_WIDGET_REALIZED (osd))
  {
    gint rel_x, rel_y;
    gint w, h;
    gboolean mouse_over = FALSE;
    gdk_window_get_pointer (widget->window, &rel_x, &rel_y, NULL);
    rel_x -= BORDER_WIDTH; rel_y -= BORDER_WIDTH;
    int i;
    for (i = 0; i < osd->line_count; i++)
    {
      if (_point_in_rect (rel_x, rel_y, &osd->lyric_rects[i]))
      {
        mouse_over = TRUE;
        break;
      }
    }
    if (priv->mouse_over_lyrics != mouse_over)
    {
      priv->mouse_over_lyrics = mouse_over;
      gtk_widget_queue_draw (GTK_WIDGET (osd));
    }
    ol_osd_window_check_mouse_leave (osd);
  }
  return TRUE;
}

void
ol_osd_window_set_width (OlOsdWindow *osd, gint width)
{
  ol_errorf ("set width: %d\n", width);
  ol_assert (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  priv->width = width;
  gtk_widget_queue_resize (GTK_WIDGET (osd));
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

int
ol_osd_window_get_width (OlOsdWindow *osd)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  return priv->width;
}

void
ol_osd_window_get_osd_size (OlOsdWindow *osd, gint *width, gint *height)
{
  /* ol_log_func (); */
  ol_assert (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  if (width)
    *width = priv->width - BORDER_WIDTH * 2;
  if (height)
    *height = ol_osd_window_compute_osd_height (osd);
}

static void
ol_osd_window_update_child_visibility (OlOsdWindow *osd)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  GtkWidget *child = gtk_bin_get_child (GTK_BIN (osd));
  if (child == NULL)
    return;
  if (priv->locked)
    gtk_widget_hide (child);
  else
    gtk_widget_show (child);
}

void
ol_osd_window_set_locked (OlOsdWindow *osd, gboolean locked)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  if (priv->locked == locked)
    return;
  priv->locked = locked;
  if (GTK_WIDGET_REALIZED (GTK_WIDGET (osd))) {
    ol_osd_window_set_input_shape_mask (osd, locked);
  }
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

static void
ol_osd_window_update_child_size_requisition (OlOsdWindow *osd)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  GtkBin *bin = GTK_BIN (osd);
  if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
  {
    gtk_widget_size_request (bin->child, &priv->child_requisition);
  }
  else
  {
    priv->child_requisition.width = 0;
    priv->child_requisition.height = 0;
  }
}

/** 
 * Computes the height of the OSD Window
 *
 * The computation relies on the height of OSD text and child widget. It should be
 * called after ol_osd_window_update_child_size_requisition priv->osd_height
 * updated. 
 * @param osd 
 * 
 * @return The height of the widget
 */
static int
ol_osd_window_compute_window_height (OlOsdWindow *osd)
{
  ol_assert_ret (OL_IS_OSD_WINDOW (osd), 0);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (osd));
  int sh = gdk_screen_get_height (screen);
  int height = BORDER_WIDTH * 2;
  height += priv->osd_height;
  height = MAX (height, MIN (height + priv->child_requisition.height,
                             sh - priv->raw_y));
  return height;
}

static int
ol_osd_window_compute_osd_height (OlOsdWindow *osd)
{
  ol_assert_ret (OL_IS_OSD_WINDOW (osd) && osd->render_context != NULL,
                 DEFAULT_HEIGHT);
  int font_height = ol_osd_render_get_font_height (osd->render_context);
  int height = font_height * (osd->line_count + (osd->line_count - 1) * LINE_PADDING);
  return height;
}

static void
ol_osd_window_check_resize (GtkContainer *container)
{
  ol_assert (OL_IS_OSD_WINDOW (container));
  GtkWidget *widget = GTK_WIDGET (container);
  OlOsdWindow *osd = OL_OSD_WINDOW (container);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  ol_osd_window_update_layout (osd);
  GtkAllocation alloc = {
    .x = widget->allocation.x,
    .y = widget->allocation.y,
    .width = priv->width,
    .height = ol_osd_window_compute_osd_height (osd),
  };
  gtk_widget_size_allocate (widget, &alloc);
  gtk_widget_queue_draw (widget);
}

static void
ol_osd_window_update_child_allocation (OlOsdWindow *osd)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  GtkWidget *widget = GTK_WIDGET (osd);
  GtkBin *bin = GTK_BIN (osd);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
  {
    ol_debugf ("set child allocation\n");
    GtkAllocation alloc;
    alloc.x = (widget->allocation.width - priv->child_requisition.width) / 2;
    alloc.width = priv->child_requisition.width;
    alloc.height = priv->child_requisition.height;
    alloc.y = widget->allocation.height
      - BORDER_WIDTH
      - priv->child_requisition.height;
    gtk_widget_size_allocate (bin->child, &alloc);
  }
}

static double
ol_osd_window_compute_lyric_xpos (OlOsdWindow *osd, int line, double percentage)
{
  /* ol_log_func (); */
  GtkWidget *widget = GTK_WIDGET (osd);
  gint w, h;
  int width, height;
  /* gdk_drawable_get_size (widget->window, &w, &h); */
  ol_osd_window_get_osd_size (osd, &w, &h);
  if (osd->active_lyric_pixmap[line] != NULL)
  {
    gdk_drawable_get_size (osd->active_lyric_pixmap[line], &width, &height);
  }
  else
  {
    width = height = 0;
  }
  double xpos;
  if (w >= width)
  {
    xpos = (w - width) * osd->line_alignment[line];
  }
  else
  {
    OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
    if (priv->composited)
    {
      if (percentage * width < w / 2.0)
        xpos = 0;
      else if ((1.0 - percentage) * width < w / 2.0)
        xpos = w - width;
      else
        xpos = w / 2.0 - width * percentage;
    }
    else
    {
      if (percentage * width < w)
      {
        xpos = 0;
      }
      else
      {
        int half_count = (percentage * width - w) / w + 1;
        xpos = -half_count * w;
        if (xpos < w - width)
          xpos = w - width;
      }
    }
  }
  return xpos;
}

static int
ol_osd_window_compute_lyric_ypos (OlOsdWindow *osd)
{
  ol_assert_ret (OL_IS_OSD_WINDOW (osd), 0);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  int y = BORDER_WIDTH;
  return y;
}

static void
ol_osd_window_paint_lyrics (OlOsdWindow *osd, cairo_t *cr)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  GtkWidget *widget = GTK_WIDGET (osd);
  if (!gdk_window_is_visible (widget->window))
    return;
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  double alpha = 1.0;
  int font_height = ol_osd_render_get_font_height (osd->render_context);
  if (priv->composited && priv->locked && priv->mouse_over_lyrics &&
      osd->translucent_on_mouse_over)
  {
    alpha = 0.3;
  }
  if (!GTK_WIDGET_REALIZED (widget))
    gtk_widget_realize (widget);
  gint w, h;
  int width, height;
  ol_osd_window_get_osd_size (osd, &w, &h);
  int line;
  gdouble ypos, xpos;
  ypos = ol_osd_window_compute_lyric_ypos (osd);
  int start, end;
  if (osd->line_count == 1)
  {
    start = osd->current_line;
    end = start + 1;
  }
  else
  {
    start = 0;
    end = OL_OSD_WINDOW_MAX_LINE_COUNT;
  }
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  for (line = start; line < end; line++)
  {
    double percentage = osd->percentage[line];
    if (osd->active_lyric_pixmap[line] != NULL && osd->inactive_lyric_pixmap[line])
    {
      gdk_drawable_get_size (osd->active_lyric_pixmap[line], &width, &height);
      xpos = ol_osd_window_compute_lyric_xpos (osd, line, osd->percentage[line]);
      xpos += BORDER_WIDTH;
      cairo_save (cr);
      cairo_rectangle (cr, xpos, ypos, (double)width * percentage, height);
      cairo_clip (cr);
      gdk_cairo_set_source_pixmap (cr, osd->active_lyric_pixmap[line], xpos, ypos);
      cairo_paint_with_alpha (cr, alpha);
      cairo_restore (cr);
      cairo_save (cr);
      cairo_rectangle (cr, xpos + width * percentage, ypos, (double)width * (1.0 - percentage), height);
      cairo_clip (cr);
      gdk_cairo_set_source_pixmap (cr, osd->inactive_lyric_pixmap[line], xpos, ypos);
      cairo_paint_with_alpha (cr, alpha);
      cairo_restore (cr);
    }
    ypos += font_height * (1 + LINE_PADDING);
  }
}

static void
ol_osd_window_clear_cairo (cairo_t *cr)
{
  ol_assert (cr != NULL);
  cairo_save (cr);
  cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE); // set drawing compositing operator
  // SOURCE -> replace destination
  cairo_paint(cr); // paint source
  cairo_restore (cr);
}

void
ol_osd_window_set_percentage (OlOsdWindow *osd, gint line, double percentage)
{
  if (line < 0 || line >= OL_OSD_WINDOW_MAX_LINE_COUNT)
    return;
  ol_assert (OL_IS_OSD_WINDOW (osd));
  if (percentage == osd->percentage[line])
    return;
  double old_percentage = osd->percentage[line];
  osd->percentage[line] = percentage;
  /* update shape if line is too long */
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  if (!priv->composited && percentage != old_percentage)
  {
    int old_x = ol_osd_window_compute_lyric_xpos (osd, line, old_percentage);
    int new_x = ol_osd_window_compute_lyric_xpos (osd, line, percentage);
  }
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

void
ol_osd_window_set_current_percentage (OlOsdWindow *osd, double percentage)
{
  ol_osd_window_set_percentage (osd, osd->current_line, percentage);
}

double
ol_osd_window_get_current_percentage (OlOsdWindow *osd)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  return osd->percentage[osd->current_line];
}

void
ol_osd_window_set_current_line (OlOsdWindow *osd, gint line)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  ol_assert (line < ol_osd_window_get_line_count (osd));
  osd->current_line = line;
}

gint ol_osd_window_get_current_line (OlOsdWindow *osd)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  return osd->current_line;
}

static void
ol_osd_draw_lyric_pixmap (OlOsdWindow *osd, GdkPixmap **pixmap, const char *lyric)
{
  if (*pixmap != NULL)
  {
    g_object_unref (*pixmap);
    *pixmap = NULL;
  }
  if (!ol_is_string_empty (lyric) && *pixmap == NULL)
  {
    int w, h;
    if (!GTK_WIDGET_REALIZED (GTK_WIDGET (osd)))
      gtk_widget_realize (GTK_WIDGET (osd));
    ol_osd_render_get_pixel_size (osd->render_context,
                                  lyric,
                                  &w, &h);
    *pixmap = gdk_pixmap_new (GTK_WIDGET (osd)->window, w, h, -1);
    cairo_t *cr = gdk_cairo_create (*pixmap);
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    ol_osd_render_paint_text (osd->render_context,
                              cr,
                              lyric,
                              0,
                              0);
    cairo_destroy (cr);
  }
}

static void
ol_osd_window_update_lyric_pixmap (OlOsdWindow *osd, int line)
{
  int i;
  if (!GTK_WIDGET_REALIZED (GTK_WIDGET (osd)))
    return;
  /* draws the inactive pixmaps */
  for (i = 0; i < OL_LINEAR_COLOR_COUNT; i++)
  {
    ol_osd_render_set_linear_color (osd->render_context,
                                    i,
                                    osd->inactive_colors[i]);
  }
  ol_osd_draw_lyric_pixmap (osd, &osd->inactive_lyric_pixmap[line], osd->lyrics[line]);
  /* draws the active pixmaps */
  for (i = 0; i < OL_LINEAR_COLOR_COUNT; i++)
  {
    ol_osd_render_set_linear_color (osd->render_context,
                                    i,
                                    osd->active_colors[i]);
  }
  ol_osd_draw_lyric_pixmap (osd, &osd->active_lyric_pixmap[line], osd->lyrics[line]);
  ol_osd_window_update_lyric_rect (osd, line);
}

static void
ol_osd_window_update_lyric_rect (OlOsdWindow *osd, int line)
{
  ol_log_func ();
  int w, h;
  w = h = 0;
  if (osd->active_lyric_pixmap[line] != NULL)
    gdk_drawable_get_size (osd->active_lyric_pixmap[line], &w, &h);
  int font_height = ol_osd_render_get_font_height (osd->render_context);
  osd->lyric_rects[line].x = ol_osd_window_compute_lyric_xpos (osd,
                                                               line,
                                                               osd->percentage[line]);
  osd->lyric_rects[line].y = font_height * line * (1 + LINE_PADDING);
  osd->lyric_rects[line].width = w;
  osd->lyric_rects[line].height = h;
}

static gboolean
ol_osd_window_has_lyrics (OlOsdWindow *osd)
{
  int i;
  gboolean ret = FALSE;
  for (i = 0; i < ol_osd_window_get_line_count (osd); i++)
  {
    if (!ol_is_string_empty (osd->lyrics[i]))
    {
      ret = TRUE;
      break;
    }
  }
  return ret;
}

void
ol_osd_window_set_lyric (OlOsdWindow *osd, gint line, const char *lyric)
{
  ol_log_func ();
  ol_debugf ("  lyric:%s\n", lyric);
  ol_assert (OL_IS_OSD_WINDOW (osd));
  ol_assert (line >= 0 && line < OL_OSD_WINDOW_MAX_LINE_COUNT);
  if (osd->lyrics[line] != NULL)
    g_free (osd->lyrics[line]);
  if (lyric != NULL)
    osd->lyrics[line] = g_strdup (lyric);
  else
    osd->lyrics[line] = g_strdup ("");
  ol_osd_window_update_lyric_pixmap (osd, line);
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

void
ol_osd_window_set_line_alignment (OlOsdWindow *osd, gint line, double alignment)
{
  ol_debugf ("%s:%d-%lf\n", __FUNCTION__, line, alignment);
  ol_assert (OL_IS_OSD_WINDOW (osd));
  if (line < 0 || line >= OL_OSD_WINDOW_MAX_LINE_COUNT)
    return;
  if (alignment < 0.0)
    alignment = 0.0;
  else if (alignment > 1.0)
    alignment = 1.0;
  osd->line_alignment[line] = alignment;
  ol_osd_window_update_lyric_rect (osd, line);
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

static void
ol_osd_window_set_input_shape_mask (OlOsdWindow *osd, gboolean disable_input)
{
  gint w, h;
  ol_assert (OL_IS_OSD_WINDOW (osd));
  GtkWidget *widget = GTK_WIDGET (osd);
  ol_assert (GTK_WIDGET_REALIZED (widget));
  if (disable_input)
  {    
    GdkRegion *region = gdk_region_new ();
    gdk_window_input_shape_combine_region (GTK_WIDGET (osd)->window, region, 0, 0);
    gdk_region_destroy (region);
  }
  else
  {
    gdk_window_input_shape_combine_region (GTK_WIDGET (osd)->window, NULL, 0, 0);
  }
}

/** 
 * Move and resize the OSD Window.
 *
 * The coordinations and size are raw values, which means does not honor the size
 * of the child widget.
 * 
 * @param osd 
 * @param raw_x 
 * @param raw_y 
 * @param width
 */
static void
ol_osd_window_move_resize (OlOsdWindow *osd,
                           int raw_x,
                           int raw_y,
                           int width,
                           enum DragState drag_type)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  GtkWidget *widget = GTK_WIDGET (osd);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  int old_raw_x = priv->raw_x, old_raw_y = priv->raw_y;
  int x, y;
  x = raw_x;
  y = raw_y;
  priv->raw_x = raw_x;
  priv->raw_y = raw_y;
  if (ol_osd_window_get_mode (osd) == OL_OSD_WINDOW_DOCK)
  {
    GdkScreen *screen = gtk_widget_get_screen (widget);
    int sw = gdk_screen_get_width (screen);
    int sh = gdk_screen_get_height (screen);
    /* Since the height might change when getting close to the bottom of
       screen, we need to compute it. */
    int h = ol_osd_window_compute_window_height (osd);
    int w = width;
    if (drag_type == DRAG_MOVE)
      w = widget->allocation.width;
    if (w < priv->child_requisition.width)
      w = priv->child_requisition.width;
    if (drag_type == DRAG_EAST)
    {
      x = MAX (0, x);
      w = MIN (w, sw - x);
    }
    else if (drag_type == DRAG_WEST)
    {
      w = MIN (w, x + w);
      x = MAX (0, MIN (x, width + raw_x - w));
    }
    else
    {
      x = MAX (0, MIN (x, sw - w));
    }
    y = MAX (0, MIN (y, sh- h));
    if (h != widget->allocation.height || w != widget->allocation.width)
    {
      /* gtk_window_resize and gtk_widget_queue_resize don't work,
         so we have to specify allocation explicitly*/
      GtkAllocation alloc = {
        .x = widget->allocation.x,
        .y = widget->allocation.y,
        .width = w,
        .height = h,
      };
      gtk_widget_size_allocate (widget, &alloc);
      gtk_widget_queue_draw (widget);
    }
  }
  priv->raw_x = x;
  priv->raw_y = y;
  if (priv->raw_x != old_raw_x || priv->raw_y != old_raw_y)
  {
    gtk_window_move (GTK_WINDOW (osd), x, y);
  }
}

void
ol_osd_window_move (OlOsdWindow *osd, int x, int y)
{
  ol_osd_window_move_resize (osd, x, y, -1, DRAG_MOVE);
}

void
ol_osd_window_get_pos (OlOsdWindow *osd, int *x, int *y)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  if (x)
    *x = priv->raw_x;
  if (y)
    *y = priv->raw_y;
}

static void
ol_osd_window_class_init (OlOsdWindowClass *klass)
{
  ol_log_func ();
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;

  gobject_class = G_OBJECT_CLASS (klass);
  object_class = (GtkObjectClass*)klass;
  widget_class = (GtkWidgetClass*)klass;
  container_class = (GtkContainerClass*)klass;
  ol_osd_window_parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = ol_osd_window_set_property;
  gobject_class->get_property = ol_osd_window_get_property;

  object_class->destroy = ol_osd_window_destroy;
  
  widget_class->size_allocate = ol_osd_window_size_allocate;
  widget_class->size_request = ol_osd_window_size_request;
  widget_class->enter_notify_event = ol_osd_window_enter_notify;
  widget_class->leave_notify_event = ol_osd_window_leave_notify;

  container_class->check_resize = ol_osd_window_check_resize;
  
  /* set up properties */
  g_object_class_install_property (gobject_class,
                                   PROP_LOCKED,
                                   g_param_spec_boolean ("locked",
                                                         ("Whether the it is possible to move the OSD window"),
                                                         ("If TRUE, there will be a window displayed under the"
                                                          "OSD indicating that it can be moved, and so it is"),
                                                         FALSE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));
  /* install signals */
  klass->signals[OSD_MOVED] =
    g_signal_newv ("moved",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  NULL,         /* closure */
                  NULL,         /* accumulator */
                  NULL,         /* accumulator data */
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0,
                  NULL);
  klass->signals[OSD_RESIZE] =
    g_signal_newv ("resize",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  NULL,         /* closure */
                  NULL,         /* accumulator */
                  NULL,         /* accumulator data */
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0,
                  NULL);
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
  case PROP_LOCKED:
    g_value_set_boolean (value, priv->locked);
  }
}
  
static void
ol_osd_window_init (OlOsdWindow *osd)
{
  ol_log_func ();
  GtkWindow *window = GTK_WINDOW (osd);
  window->type = GTK_WINDOW_TOPLEVEL;
  gtk_window_set_decorated (window, FALSE);
  gtk_widget_set_app_paintable (GTK_WIDGET (osd), TRUE);
  osd->current_line = 0;
  osd->bg_pixbuf = NULL;
  osd->line_count = 1;
  int i;
  for (i = 0; i < OL_OSD_WINDOW_MAX_LINE_COUNT; i++)
  {
    osd->lyrics[i] = NULL;
    osd->line_alignment[i] = 0.5;
    osd->percentage[i] = 0.0;
    osd->active_lyric_pixmap[i] = NULL;
    osd->inactive_lyric_pixmap[i] = NULL;
    osd->lyric_rects[i].x = 0;
    osd->lyric_rects[i].y = 0;
    osd->lyric_rects[i].width = 0;
    osd->lyric_rects[i].height = 0;
  }
  for (i = 0; i < OL_LINEAR_COLOR_COUNT; i++)
  {
    osd->active_colors[i] = DEFAULT_ACTIVE_COLORS[i];
    osd->inactive_colors[i] = DEFAULT_INACTIVE_COLORS[i];
  }
  osd->render_context = ol_osd_render_context_new ();
  osd->shape_pixmap = NULL;
  osd->translucent_on_mouse_over = FALSE;
  /* initilaize private data */
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  priv->width = DEFAULT_WIDTH;
  priv->pressed = FALSE;
  priv->locked = TRUE;
  priv->composited = FALSE;
  priv->visible = FALSE;
  priv->mouse_timer_id = 0;
  priv->drag_timer_id = 0;
  priv->child_requisition.width = 0;
  priv->child_requisition.height = 0;
  priv->mouse_over_lyrics = FALSE;
  priv->composited_signal =  g_signal_connect (gtk_widget_get_screen (GTK_WIDGET (osd)),
                                               "composited-changed",
                                               G_CALLBACK (ol_osd_window_screen_composited_changed),
                                               osd);
  priv->raw_x = 0;
  priv->raw_y = 0;
  priv->drag_state = DRAG_NONE;
  /* ol_osd_window_set_mode (osd, OL_OSD_WINDOW_DOCK); */
  ol_osd_window_set_mode (osd, OL_OSD_WINDOW_NORMAL);
  gtk_widget_add_events(GTK_WIDGET(osd),
                        GDK_BUTTON_PRESS_MASK |
                        GDK_BUTTON_RELEASE_MASK |
                        GDK_POINTER_MOTION_MASK);
  g_signal_connect (G_OBJECT (osd), "button-press-event",
                    G_CALLBACK (ol_osd_window_button_press), osd);
  g_signal_connect (G_OBJECT (osd), "button-release-event",
                    G_CALLBACK (ol_osd_window_button_release), osd);
  g_signal_connect (G_OBJECT (osd), "motion-notify-event",
                    G_CALLBACK (ol_osd_window_motion_notify), osd);
  g_signal_connect (G_OBJECT (osd), "expose-event",
                    G_CALLBACK (ol_osd_window_expose_before), osd);
  g_signal_connect (G_OBJECT (osd), "configure-event",
                    G_CALLBACK (ol_osd_window_configure_event), osd);
  g_signal_connect_after (G_OBJECT (osd), "expose-event",
                          G_CALLBACK (ol_osd_window_expose_after), osd);
  g_signal_connect (G_OBJECT (osd), "map-event",
                    G_CALLBACK (ol_osd_window_map_cb), osd);
  g_signal_connect (G_OBJECT (osd), "unmap-event",
                    G_CALLBACK (ol_osd_window_unmap_cb), osd);
  ol_osd_window_screen_composited_changed (gtk_widget_get_screen (GTK_WIDGET (osd)),
                                           osd);
}

GtkWidget*
ol_osd_window_new ()
{
  ol_log_func ();
  OlOsdWindow *osd;
  osd = g_object_new (ol_osd_window_get_type (), NULL);
  return GTK_WIDGET (osd);
}

static void
ol_osd_window_destroy (GtkObject *object)
{
  ol_log_func ();
  ol_assert (OL_IS_OSD_WINDOW (object));
  GtkWidget *widget = GTK_WIDGET (object);
  OlOsdWindow *osd = OL_OSD_WINDOW (object);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (object);
  int i;
  for (i = 0; i < OL_OSD_WINDOW_MAX_LINE_COUNT; i++)
  {
    if (osd->lyrics[i] != NULL)
    {
      g_free (osd->lyrics[i]);
      osd->lyrics[i] = NULL;
    }
    if (osd->active_lyric_pixmap[i] != NULL)
    {
      g_object_unref (osd->active_lyric_pixmap[i]);
      osd->active_lyric_pixmap[i] = NULL;
    }
    if (osd->inactive_lyric_pixmap[i] != NULL)
    {
      g_object_unref (osd->inactive_lyric_pixmap[i]);
      osd->inactive_lyric_pixmap[i] = NULL;
    }
  }
  if (osd->shape_pixmap != NULL)
  {
    g_object_unref (osd->shape_pixmap);
    osd->shape_pixmap = NULL;
  }
  if (osd->render_context != NULL)
  {
    ol_osd_render_context_destroy (osd->render_context);
    osd->render_context = NULL;
  }
  if (osd->bg_pixbuf != NULL)
    g_object_unref (osd->bg_pixbuf);
  if (priv->composited_signal != 0)
  {
    g_signal_handler_disconnect (gtk_widget_get_screen (widget),
                                 priv->composited_signal);
    priv->composited_signal = 0;
  }
  GTK_OBJECT_CLASS (ol_osd_window_parent_class)->destroy (object);
}

void
ol_osd_window_set_font_family (OlOsdWindow *osd,
                               const char *font_family)
{
  if (osd == NULL || osd->render_context == NULL || font_family == NULL)
    return;
  ol_osd_render_set_font_family (osd->render_context,
                                 font_family);
  int i;
  for (i = 0; i < osd->line_count; i++)
    ol_osd_window_update_lyric_pixmap (osd, i);
  gtk_widget_queue_resize (GTK_WIDGET (osd));
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

char*
ol_osd_window_get_font_family (OlOsdWindow *osd)
{
  if (osd == NULL || osd->render_context == NULL)
    return NULL;
  return ol_osd_render_get_font_family (osd->render_context);
}

void
ol_osd_window_set_font_size (OlOsdWindow *osd,
                             const double font_size)
{
  if (osd == NULL || osd->render_context == NULL)
    return;
  ol_osd_render_set_font_size (osd->render_context, font_size);
  int i;
  for (i = 0; i < osd->line_count; i++)
    ol_osd_window_update_lyric_pixmap (osd, i);
  gtk_widget_queue_resize (GTK_WIDGET (osd));
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

double
ol_osd_window_get_font_size (OlOsdWindow *osd)
{
  ol_assert_ret (osd != NULL, 0.0);
  ol_assert_ret (osd->render_context != NULL, 0.0);
  return ol_osd_render_get_font_size (osd->render_context);
}

void
ol_osd_window_set_outline_width (OlOsdWindow *osd,
                                 const int width)
{
  ol_assert (osd != NULL);
  ol_assert (osd->render_context != NULL);
  ol_osd_render_set_outline_width (osd->render_context, width);
  int i;
  for (i = 0; i < osd->line_count; i++)
    ol_osd_window_update_lyric_pixmap (osd, i);
  gtk_widget_queue_resize (GTK_WIDGET (osd));
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

int
ol_osd_window_get_outline_width (OlOsdWindow *osd)
{
  ol_assert_ret (osd != NULL, 0);
  ol_assert_ret (osd->render_context != NULL, 0);
  return ol_osd_render_get_outline_width (osd->render_context);

}

void
ol_osd_window_set_active_colors (OlOsdWindow *osd,
                                 OlColor top_color,
                                 OlColor middle_color,
                                 OlColor bottom_color)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  osd->active_colors[0] = top_color;
  osd->active_colors[1] = middle_color;
  osd->active_colors[2] = bottom_color;
  int i;
  for (i = 0; i < osd->line_count; i++)
    ol_osd_window_update_lyric_pixmap (osd, i);
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

void
ol_osd_window_set_inactive_colors (OlOsdWindow *osd,
                                   OlColor top_color,
                                   OlColor middle_color,
                                   OlColor bottom_color)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  osd->inactive_colors[0] = top_color;
  osd->inactive_colors[1] = middle_color;
  osd->inactive_colors[2] = bottom_color;
  int i;
  for (i = 0; i < osd->line_count; i++)
    ol_osd_window_update_lyric_pixmap (osd, i);
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

void
ol_osd_window_set_line_count (OlOsdWindow *osd,
                              guint line_count)
{
  ol_assert (OL_IS_OSD_WINDOW (osd)); 
  ol_assert (line_count >= 1 && line_count <= 2);
  osd->line_count = line_count;
  gtk_widget_queue_resize (GTK_WIDGET (osd));
}

guint
ol_osd_window_get_line_count (OlOsdWindow *osd)
{
  ol_assert_ret (OL_IS_OSD_WINDOW (osd), 1);
  return osd->line_count;
}

void
ol_osd_window_set_translucent_on_mouse_over (OlOsdWindow *osd,
                                             gboolean value)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  osd->translucent_on_mouse_over = value;
}

gboolean
ol_osd_window_get_translucent_on_mouse_over (OlOsdWindow *osd)
{
  ol_assert_ret (OL_IS_OSD_WINDOW (osd), FALSE);
  return osd->translucent_on_mouse_over;
}

void
ol_osd_window_set_bg (OlOsdWindow *osd, GdkPixbuf *bg)
{
  ol_log_func ();
  ol_debugf ("bg: %p\n", bg);
  ol_assert (OL_IS_OSD_WINDOW (osd));
  if (bg != NULL)
    g_object_ref (bg);
  if (osd->bg_pixbuf != NULL)
    g_object_unref (osd->bg_pixbuf);
  osd->bg_pixbuf = bg;
  gtk_widget_queue_draw (GTK_WIDGET (osd));
}

void
ol_osd_window_set_mode (OlOsdWindow *osd, enum OlOsdWindowMode mode)
{
  ol_assert (OL_IS_OSD_WINDOW (osd));
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  GtkWindow *window = GTK_WINDOW (osd);
  GtkWidget *widget = GTK_WIDGET (osd);
  if (mode == priv->mode)
    return;
  gboolean realized = GTK_WIDGET_REALIZED (widget);
  gboolean mapped = GTK_WIDGET_MAPPED (widget);
  if (mapped)
    gtk_widget_unmap (widget);
  if (realized)
    gtk_widget_unrealize (widget);
  switch (mode)
  {
  case OL_OSD_WINDOW_NORMAL:
    gtk_window_set_type_hint (window, GDK_WINDOW_TYPE_HINT_NORMAL);
    window->type = GTK_WINDOW_TOPLEVEL;
    break;
  case OL_OSD_WINDOW_DOCK:
    gtk_window_set_type_hint (window, GDK_WINDOW_TYPE_HINT_DOCK);
    window->type = GTK_WINDOW_POPUP;
    break;
  default:
    ol_errorf ("Invalid OSD Window type %d\n", mode);
    mode = priv->mode;
  }
  priv->mode = mode;
  if (realized)
    gtk_widget_realize (widget);
  if (mapped)
    gtk_widget_map (widget);
}

enum OlOsdWindowMode
ol_osd_window_get_mode (OlOsdWindow *osd)
{
  ol_assert_ret (OL_IS_OSD_WINDOW (osd), OL_OSD_WINDOW_NORMAL);
  OlOsdWindowPrivate *priv = OL_OSD_WINDOW_GET_PRIVATE (osd);
  return priv->mode;
}
