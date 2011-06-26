/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2011  Tiger Soldier <tigersoldier@gmail.com>
 *
 * This file is part of OSD Lyrics.
 * 
 * OSD Lyrics is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OSD Lyrics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OSD Lyrics.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include <math.h>
#include <string.h>
#include <glib.h>
#include <ol_debug.h>
#include "ol_gussian_blur.h"

struct _pixel
{
  double alpha;
  double red;
  double green;
  double blue;
};

static double *_calc_kernel (double sigma, int *size);
static void _apply_kernel (cairo_surface_t *surface,
                           const double *kernel,
                           int kernel_size);
static inline int _pos_to_index (int x, int y, int width, int height);
static inline struct _pixel _num_to_pixel_with_factor (guint32 value,
                                                       double factor);
static inline guint32 _pixel_to_num_with_factor (struct _pixel *pixel,
                                                 double factor);
static inline void _pixel_plus (struct _pixel *adder_sum,
                                const struct _pixel *adder2);

static inline int
_pos_to_index (int x, int y, int width, int height)
{
  if (x >= width || y >= height || x < 0 || y < 0)
    return -1;
  return y * width + x;
}

static inline struct _pixel
_num_to_pixel_with_factor (guint32 value,
                           double factor)
{
  /* This only works for type CAIRO_FORMAT_ARGB32 */
  struct _pixel pixel;
  pixel.alpha = ((value >> 24) & 0xff) * factor;
  pixel.red = ((value >> 16) & 0xff) * factor;
  pixel.green = ((value >> 8) & 0xff) * factor;
  pixel.blue = (value & 0xff) * factor;
  return pixel;
}

static inline guint32
_pixel_to_num_with_factor (struct _pixel *pixel,
                           double factor)
{
  guint32 alpha = pixel->alpha * factor;
  if (alpha > 0xff) alpha = 0xff;
  guint32 red = pixel->red * factor;
  if (red > 0xff) red = 0xff;
  guint32 green = pixel->green * factor;
  if (green > 0xff) green = 0xff;
  guint32 blue = pixel->blue * factor;
  if (blue > 0xff) blue = 0xff;
  return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

static inline void
_pixel_plus (struct _pixel *adder_sum,
             const struct _pixel *adder2)
{
  adder_sum->alpha += adder2->alpha;
  adder_sum->red += adder2->red;
  adder_sum->green += adder2->green;
  adder_sum->blue += adder2->blue;
}

static double *
_calc_kernel (double sigma, int *size)
{
  int kernel_size = ceil (sigma * 6);
  if (kernel_size % 2 == 0)
    kernel_size++;
  int orig = kernel_size / 2;
  if (size) *size = kernel_size;
  double *kernel = g_new (double, kernel_size);
  int i;
  double factor = 1.0 / sqrt (2.0 * M_PI * sigma * sigma);
  double denom = 1.0 / (2.0 * sigma * sigma);
  for (i = 0; i < kernel_size; i++)
    kernel[i] = factor * exp (- (i - orig) * (i - orig) * denom);
  return kernel;
}

static void _apply_kernel (cairo_surface_t *surface,
                           const double *kernel,
                           int kernel_size)
{
  ol_assert (kernel_size > 0 && kernel_size % 2 == 1);
  ol_assert (kernel != NULL);
  static const int DIR[2][2] = {{0, 1}, {1, 0}};
  guint32 *pixels = (guint32*) cairo_image_surface_get_data (surface);
  int width = cairo_image_surface_get_width (surface);
  int height = cairo_image_surface_get_height (surface);
  if (pixels == NULL || width <= 0 || height <= 0)
  {
    ol_errorf ("Invalid image surface");
    return;
  }
  int kernel_orig = kernel_size / 2;
  int d, i, x, y;
  for (d = 0; d < 2; d++)
  {
    guint32 *old_pixels = g_new (guint32, width * height);
    memcpy (old_pixels, pixels, sizeof (guint32) * width * height);
    for (x = 0; x < width; x++)
      for (y = 0; y < height; y++)
      {
        struct _pixel final_value = {0};
        double factor = 0.0;
        for (i = 0; i < kernel_size; i++)
        {
          int x1 = x + (i - kernel_orig) * DIR[d][0];
          int y1 = y + (i - kernel_orig) * DIR[d][1];
          int index1 = _pos_to_index (x1, y1, width, height);
          if (index > 0)
          {
            factor += kernel[i];
            struct _pixel value = _num_to_pixel_with_factor (old_pixels[index1],
                                                             kernel[i]);
            _pixel_plus (&final_value, &value);
          }
        }
        int index = _pos_to_index (x, y, width, height);
        pixels[index] = _pixel_to_num_with_factor (&final_value, 1 / factor);
      }
    g_free (old_pixels);
  }
}

void
ol_gussian_blur (cairo_surface_t *surface,
                 double sigma)
{
  ol_assert (surface != NULL);
  ol_assert (sigma > 0);
  cairo_format_t format = cairo_image_surface_get_format (surface);
  if (format != CAIRO_FORMAT_ARGB32)
  {
    ol_errorf ("The surface format is %d, only ARGB32 is supported\n",
               format);
    return;
  }
  int kernel_size;
  double *kernel = _calc_kernel (sigma, &kernel_size);
  _apply_kernel (surface, kernel, kernel_size);
  g_free (kernel);
}

