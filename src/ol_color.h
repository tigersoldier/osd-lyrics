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

/** 
 * @brief Converts a string list to color array
 * 
 * @param str_list String array, should be terminated with NULL
 * @param len Return location of the count of colors
 * 
 * @return Color array, should be free with g_free, or NULL if failed
 */
OlColor* ol_color_from_str_list (const char **str_list, int *len);

/** 
 * @brief Converts a color array to NULL terminated string array
 * 
 * @param colors An array of OlColor
 * @param len The length of colors
 * 
 * @return NULL terminated string array, free with g_strfreev
 */
char** ol_color_to_str_list (const OlColor *colors, int len);

extern const OlColor ol_color_black;

#endif /* _OL_COLOR_H_ */
