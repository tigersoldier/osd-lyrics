#ifndef _OL_COLOR_H_
#define _OL_COLOR_H_

typedef struct __OlColor
{
  double r;
  double g;
  double b;
} OlColor;

/** 
 * @brief Parse a string in the form of #RRGGBB and return the corresponding OlColor
 * 
 * @param color_str The color string
 * 
 * @return The corresponding color, if color_str is not a valid color string,
 *         return black (result of #000000)
 */
OlColor ol_color_from_string (const char *color_str);

/** 
 * @brief Convert a color to a string in the form of #RRGGBB
 * 
 * @param color The color to be converted
 * 
 * @return The converted string. It belongs to the function and should not be
 *         freed. It will be changed after another ol_color_to_string is called.
 */
const char* ol_color_to_string (OlColor color);

extern const OlColor ol_color_black;

#endif /* _OL_COLOR_H_ */
