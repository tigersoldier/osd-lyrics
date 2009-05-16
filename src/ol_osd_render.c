#include "ol_osd_render.h"

static const OlColor OL_BLACK = {0.0, 0.0, 0.0};
static const double OUTLINE_WIDTH = 3.0;
static const char *DEFAULT_FONT_FAMILY = "AR PL UKai CN";
static const double DEFAULT_FONT_SIZE = 30.0;

OlOsdRenderContext *
ol_osd_render_context_new ()
{
  OlOsdRenderContext *context = g_new (OlOsdRenderContext, 1);
  context->font_family = g_strdup (DEFAULT_FONT_FAMILY);
  context->font_size = DEFAULT_FONT_SIZE;
  int i;
  for (i = 0; i < OL_LINEAR_COLOR_COUNT; i++)
  {
    context->linear_colors[i] = OL_BLACK;
  }
  context->linear_pos[0] = 0.0;
  context->linear_pos[1] = 0.5;
  context->linear_pos[2] = 1.0;
  context->pango_context = gdk_pango_context_get ();
  context->pango_layout = pango_layout_new (context->pango_context);
  gchar *font_string = g_strdup_printf ("%s %0.0lf", context->font_family, context->font_size);
  PangoFontDescription *font_desc = pango_font_description_from_string (font_string);
  printf ("%s\n", font_string);
  g_free (font_string);
  pango_layout_set_font_description (context->pango_layout, font_desc);
  return context;
}

void
ol_osd_render_context_free (OlOsdRenderContext *context)
{
  g_return_if_fail (context != NULL);
  if (context->font_family != NULL)
    g_free (context->font_family);
  if (context->pango_layout != NULL)
    g_object_unref (context->pango_layout);
  if (context->pango_context != NULL)
    g_object_unref (context->pango_context);
  g_free (context);
}

void
ol_osd_render_paint_text (OlOsdRenderContext *context,
                          cairo_t *cr,
                          const char *text,
                          double xpos,
                          double ypos)
{
  g_return_if_fail (context != NULL);
  g_return_if_fail (cr != NULL);
  g_return_if_fail (text != NULL);
  pango_layout_set_text (context->pango_layout, text, -1);
  int width, height;
  pango_layout_get_pixel_size (context->pango_layout, &width, &height);
  /* draws the outline of the text */
  cairo_move_to (cr, xpos, ypos);
  cairo_save (cr);
  pango_cairo_layout_path(cr, context->pango_layout);
  cairo_set_source_rgb (cr, OL_BLACK.r, OL_BLACK.g, OL_BLACK.b);
  cairo_set_line_width (cr, OUTLINE_WIDTH);
  cairo_stroke (cr);
  cairo_restore (cr);
  cairo_new_path (cr);
  /* creates the linear pattern */
  cairo_pattern_t *pattern = cairo_pattern_create_linear (xpos, ypos, xpos, ypos + height);
  int i;
  for (i = 0; i < OL_LINEAR_COLOR_COUNT; i++)
  {
    cairo_pattern_add_color_stop_rgb(pattern,
                                     context->linear_pos[i],
                                     context->linear_colors[i].r,
                                     context->linear_colors[i].g,
                                     context->linear_colors[i].b);
  }
  cairo_set_source (cr, pattern);
  /* draws the text */
  cairo_move_to (cr, xpos, ypos);
  pango_cairo_show_layout (cr, context->pango_layout);
  cairo_pattern_destroy (pattern);
}

void
ol_osd_render_get_pixel_size (OlOsdRenderContext *context,
                              const char *text,
                              int *width,
                              int *height)
{
  g_return_if_fail (context != NULL);
  g_return_if_fail (text != NULL);
  g_return_if_fail (width != NULL || height != NULL);
  pango_layout_set_text (context->pango_layout, text, -1);
  int w, h;
  pango_layout_get_pixel_size (context->pango_layout, &w, &h);
  if (width != NULL)
    *width = w;
  if (height != NULL)
    *height = h;
}

void
ol_osd_render_set_linear_color (OlOsdRenderContext *context,
                                int index,
                                OlColor color)
{
  g_return_if_fail (context != NULL);
  g_return_if_fail (index >= 0 && index < OL_LINEAR_COLOR_COUNT);
  context->linear_colors[index] = color;
}
