#ifndef __OL_OSD_RENDER_H__
#define __OL_OSD_RENDER_H__

#include <gtk/gtk.h>
#include "ol_color.h"

enum {
  OL_LINEAR_COLOR_COUNT = 3,
};

typedef struct
{
  char *font_family;
  double font_size;
  OlColor linear_colors[OL_LINEAR_COLOR_COUNT];
  double linear_pos[OL_LINEAR_COLOR_COUNT];
  double outline_width;
  PangoContext *pango_context;
  PangoLayout *pango_layout;
  char *text;
} OlOsdRenderContext;

/** 
 * @brief Creates a new context
 * The new context should be destroyed by ol_osd_render_context_destroy
 * 
 * @return The new context
 */
OlOsdRenderContext* ol_osd_render_context_new ();
/** 
 * @brief Destroys an OlOsdRenderContext
 * 
 * @param context The context to be destroyed
 */
void ol_osd_render_context_destroy (OlOsdRenderContext *context);

/** 
 * @brief Sets the font family for a context
 * 
 * @param context An OlOsdRenderContext;
 * @param font_family Font family, must not be NULL
 */
void ol_osd_render_set_font_family (OlOsdRenderContext *context,
                                    const char *font_family);
/** 
 * @brief Gets the font family for a context
 * 
 * @param context An OlOsdRenderContext
 * @return The font family of the context, must be freed by g_free
 */
char* ol_osd_render_get_font_family (OlOsdRenderContext *context);

/** 
 * @brief Sets the font size for a contexnt
 * 
 * @param context An OlOsdRenderContext;
 * @param font_size Font size, must be positive
 */
void ol_osd_render_set_font_size (OlOsdRenderContext *context,
                                  const double font_size);

/** 
 * @brief Gets the font size for a context
 * 
 * @param context An OlOsdRenderContext;
 * 
 * @return The font size for the context
 */
double ol_osd_render_get_font_size (OlOsdRenderContext *context);

/** 
 * @brief Sets linear color
 * 
 * @param context An OlOsdRenderContext
 * @param index The index of the color
 * @param color The color to be set
 */

void ol_osd_render_set_linear_color (OlOsdRenderContext *context,
                                     int index,
                                     OlColor color);

/** 
 * @brief Paints text to pixmap
 * 
 * @param context The context of the renderer
 * @param canvas The GdkDrawable to be drawn
 * @param text The text to be painted
 * @param x The horizontal position
 * @param y The vertectical position
 */
void ol_osd_render_paint_text (OlOsdRenderContext *context,
                               cairo_t *canvas,
                               const char *text,
                               double x,
                               double y);

/** 
 * @brief Gets the width and height of the text
 * 
 * @param context The context of the renderer, must not be NULL
 * @param text The text to be calculated, must not be NULL
 * @param width The width of the text
 * @param height The height of the text
 */
void ol_osd_render_get_pixel_size (OlOsdRenderContext *context,
                                   const char *text,
                                   int *width,
                                   int *height);
/** 
 * @brief Sets text to the context
 * 
 * @param context An OlOsdRenderContext
 * @param text Text to be set
 */
void ol_osd_render_set_text (OlOsdRenderContext* context,
                             const char *text);
#endif /* __OL_OSD_RENDER_H__ */
