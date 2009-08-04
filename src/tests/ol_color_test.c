#include "ol_color.h"
#include <stdio.h>

void
print_color (OlColor color)
{
  printf ("OlColor color = {%lf, %lf, %lf}\n", color.r, color.g, color.b);
}

void
test_from_str ()
{
  print_color (ol_color_from_string (NULL));
  print_color (ol_color_from_string (""));
  print_color (ol_color_from_string ("AABBCC"));
  print_color (ol_color_from_string ("#AAEEGG"));
  print_color (ol_color_from_string ("#000000"));
  print_color (ol_color_from_string ("#FFFFFF"));
  print_color (ol_color_from_string ("#ffffff"));
  print_color (ol_color_from_string ("#3377aA"));
  print_color (ol_color_from_string ("#abcdef"));
  print_color (ol_color_from_string ("#ABCDEF"));
  print_color (ol_color_from_string ("#123456"));
  print_color (ol_color_from_string ("#789aBc"));
  print_color (ol_color_from_string ("#0a0B0c"));
}

void
test_to_str ()
{
  OlColor color1 = {0.000000, 0.000000, 0.000000};
  ol_color_to_string (color1);
  OlColor color2 = {1.000000, 1.000000, 1.000000};
  ol_color_to_string (color2);
  OlColor color3 = {0.200000, 0.466667, 0.666667};
  ol_color_to_string (color3);
  OlColor color4 = {0.670588, 0.803922, 0.937255};
  ol_color_to_string (color4);
  OlColor color5 = {0.039216, 0.043137, 0.047059};
  ol_color_to_string (color5);
  OlColor color6 = {0.070588, 0.203922, 0.337255};
  ol_color_to_string (color6);
  OlColor color7 = {0.470588, 0.603922, 0.737255};
  ol_color_to_string (color7);
}

int
main (int argc, char **argv)
{
  test_from_str ();
  test_to_str ();
  return 0;
}
