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

/** 
 * @brief allocate memory for pathname, from APUE
 * 
 * @return pointer to this memory if success, or NULL
 */
char* ol_path_alloc(void);

/** 
 * @brief comparing str1 with str2 case insensitive
 * 
 * @param str1
 * @param str2
 * @param count
 * 
 * @return the same with the function: strcmp in <string.h>
 */
int ignore_case_strcmp(const char *str1, const char *str2, const size_t count);
#endif // __OL_UTILS_H__
