#include <cairo.h>

#include "ol_gussian_blur.h"

static void
banchmark ()
{
  cairo_surface_t *img = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                    1000, 1000);
  ol_gussian_blur (img, 10);
  cairo_surface_destroy (img);
}

int
main ()
{
  banchmark ();
  return 0;
}
