#include "ol_utils.h"
#include <glib.h>
#include <glib-object.h>

gchar*
ol_get_string_from_hash_table (GHashTable *hash_table, gchar *key)
{
  if (!hash_table)
    return NULL;
  GValue *value;
  value = (GValue *) g_hash_table_lookup(hash_table, key);
  if (value != NULL && G_VALUE_HOLDS_STRING(value))
    return (gchar*) g_value_get_string (value);
  else
    return NULL;
}

gint
ol_get_int_from_hash_table (GHashTable *hash_table, gchar *key)
{
  if (!hash_table)
    return -1;
  GValue *value;
  value = (GValue *) g_hash_table_lookup(hash_table, key);
  if (value != NULL && G_VALUE_HOLDS_INT(value))
    return  g_value_get_int (value);
  else
    return -1;
}
