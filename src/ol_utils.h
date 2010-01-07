#ifndef __OL_UTILS_H__
#define __OL_UTILS_H__
#include <glib.h>

#define ol_get_array_len(arr) (sizeof (arr) / sizeof (arr[0]))

gchar* ol_get_string_from_hash_table (GHashTable *hash_table, const gchar *name);
gint ol_get_int_from_hash_table (GHashTable *hash_table, const gchar *name);

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
 * @return pointer to this memory if success, or NULL. Should be free with free()
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
int ol_stricmp(const char *str1, const char *str2, const size_t count);

/** 
 * @brief Copy the source string to the destination string
 * If the length to be coyped is larger than that of the destination string,
 * the copying will fail, and returns NULL.
 * @param dest Destination string
 * @param dest_len The length of destination string, including '\0'
 * @param src Source string
 * @param src_len The length of source string to be copyed
 * 
 * @return The end of the destination if suceeded, or NULL if failed.
 */
char* ol_strnncpy (char *dest,
                   size_t dest_len,
                   const char *src,
                   size_t src_len);

/** 
 * @Calculates the largest common substring for two strings
 * 
 * @param str1 
 * @param str2 
 * 
 * @return the length of the largest common substring
 */
size_t ol_lcs (const char *str1, const char *str2);

/** 
 * @Checks whether two strings are equale
 * Strings are different if one of them is NULL but another not,
 * or their content are different
 * 
 * @param str1 The first string or NULL
 * @param str2 The second string or NULL
 * 
 * @return TRUE if they are equal
 */
int ol_streq (const char *str1, const char *str2);

/** 
 * @Copy the content of a string pointer to another
 * 
 * @param dest The target string pointer. If it doesn't point to NULL, \
 * it will be free with g_free
 * @param src The source string. If it is not NULL, \
 * it will be copyed with g_strdup, the dupped string will be assigned to  \
 * dest. Otherwise the dest will be NULL after copied.
 * 
 * @return The string that dest points to after copying
 */
char *ol_strptrcpy (char **dest, const char *src);
#endif // __OL_UTILS_H__
