#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>
#include <glib-object.h>

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

#ifdef PATH_MAX
static int pathmax = PATH_MAX;
#else
static int pathmax = 0;
#endif
#define PATH_MAX_GUESS 1024

char *
ol_path_alloc(void)
{
  char *ptr;
  int size;

  if(pathmax == 0)
  {
    errno = 0;
    if((pathmax = pathconf("/", _PC_PATH_MAX)) < 0)
    {
      if(errno == 0)
      {
        pathmax = PATH_MAX_GUESS;
      }
      else
      {
        fprintf(stderr, "pathconf error for _PC_PATH_MAX\n");
        return NULL;
      }
    }
    else
    {
      pathmax++;
    }
  }

  if((ptr = calloc(pathmax, sizeof(char))) == NULL) {
    fprintf(stderr, "malloc error for pathname\n");
    return NULL;
  }

  return ptr;
}
