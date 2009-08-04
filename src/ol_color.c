#include "ol_color.h"
#include <stdio.h>
#include <string.h>

OlColor
ol_color_from_string (const char *color_str)
{
  OlColor ret = ol_color_black;
  if (color_str == NULL)
    return ret;
  if (strlen (color_str) != 7)
    return ret;
  if (color_str[0] != '#')
    return ret;
  int i = 1;
  char colors[3][3] = {0};
  for (i = 1; i < 7; i++)
  {
    if ((color_str[i] >= '0' && color_str[i] <= '9') ||
        (color_str[i] >= 'a' && color_str[i] <= 'f') ||
        (color_str[i] >= 'A' && color_str[i] <= 'F'))
    {
      colors[(i - 1) / 2][(i - 1) % 2] = color_str[i];
    }
    else
    {
      return ret;
    }
  }
  unsigned int color_int[3];
  for (i = 0; i < 3; i++)
  {
    sscanf (colors[i], "%x", &color_int[i]);
  }
  ret.r = color_int[0] / 255.0;
  ret.g = color_int[1] / 255.0;
  ret.b = color_int[2] / 255.0;
  return ret;
}

const char*
ol_color_to_string (OlColor color)
{
  static char ret[10] = "";
  unsigned int r, g, b;
  r = color.r * 255;
  g = color.g * 255;
  b = color.b * 255;
  snprintf (ret, 10, "#%02x%02x%02x", r, g, b);
  fprintf (stderr, "%s:%s\n", __FUNCTION__, ret);
  return ret;
}

const OlColor ol_color_black = {0.0, 0.0, 0.0};
