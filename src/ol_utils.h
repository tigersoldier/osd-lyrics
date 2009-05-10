#ifndef __OL_UTILS_H__
#define __OL_UTILS_H__
#include <glib.h>

gchar* ol_get_string_from_hash_table (GHashTable *hash_table, gchar *name);
gint ol_get_int_from_hash_table (GHashTable *hash_table, gchar *name);

#endif // __OL_UTILS_H__
