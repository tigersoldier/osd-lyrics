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

int 
ignore_case_strcmp(const char *str1, const char *str2, const size_t count)
{
  const char *ptr1 = str1;
  const char *ptr2 = str2;
  int len1 = strlen(str1);
  int len2 = strlen(str2);
  int min = len1 > len2 ? len2 : len1;
  min = min > count ? count : min;

  while((ptr1 < str1+min) && (ptr2 < str2+min)) {
    if(isalpha(*ptr1) && isalpha(*ptr2)) {
      if(tolower(*ptr1) != tolower(*ptr2))
        return *ptr1 > *ptr2 ? 1 : -1;
    } else {
      if(*ptr1 != *ptr2)
        return *ptr1 > *ptr2 ? 1 : -1;
    }
    ptr1++;
    ptr2++;
  }
  return 0;
}
