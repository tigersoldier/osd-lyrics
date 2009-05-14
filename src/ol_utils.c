#include <glib.h>
#include <glib-object.h>
#include <string.h>

#include "ol_utils.h"

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

gboolean
ol_is_string_empty (const char *str)
{
  if (str == NULL)
    return TRUE;
  int len = strlen (str);
  int i;
  for (i = 0; i < len; i++)
  {
    if (str[i] != ' ')
      return FALSE;
  }
  return TRUE;
}
