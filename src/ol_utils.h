#ifndef __OL_UTILS_H__
#define __OL_UTILS_H__
#include <glib.h>

#define ol_get_array_len(arr) (sizeof (arr) / sizeof (arr[0]))

gchar* ol_get_string_from_hash_table (GHashTable *hash_table, gchar *name);
gint ol_get_int_from_hash_table (GHashTable *hash_table, gchar *name);

/** 
 * @brief Checks if a string is empty
 * A string is empty if it is NULL or contains only white spaces
 * @param str The string to be checked
 * 
 * @return If the string is empty, returns true
 */
gboolean ol_is_string_empty (const char *str);

#endif // __OL_UTILS_H__
