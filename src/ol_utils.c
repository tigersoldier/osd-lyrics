#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>
#include <glib-object.h>

#include "ol_utils.h"

gchar*
ol_get_string_from_hash_table (GHashTable *hash_table, const gchar *key)
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
ol_get_int_from_hash_table (GHashTable *hash_table, const gchar *key)
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
ol_stricmp(const char *str1, const char *str2, const size_t count)
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

size_t
ol_lcs (const char *str1, const char *str2)
{
  if (str1 == NULL ||
      str2 == NULL)
    return 0;
  size_t len[2];
  len[0] = strlen (str1);
  len[1] = strlen (str2);
  if (len[0] == 0 || len[1] == 0)
    return 0;
  size_t i, j;
  int **data;
  data = g_new (int*, len[0]);
  for (i = 0; i < len[0]; i++)
  {
    data[i] = g_new (int, len[1]);
    for (j = 0; j < len[1]; j++)
    {
      data[i][j] = 0;
      if (i > 0 && j > 0)
      {
        data[i][j] = data[i - 1][j - 1];
      }
      if ((tolower (str1[i]) == tolower (str2[j])))
        data[i][j]++;
      if (i > 0 && data[i - 1][j] > data[i][j])
        data[i][j] = data[i - 1][j];
      if (j > 0 && data[i][j - 1] > data[i][j])
        data[i][j] = data[i][j - 1];
    }
  }
  int ret = data[len[0] - 1][len[1] - 1];
  for (i = 0; i < len[0]; i++)
    g_free (data[i]);
  g_free (data);
  /* fprintf (stderr, "LCS (%s, %s) = %d\n", str1, str2, ret); */
  return ret;
}

char*
ol_strnncpy (char *dest,
             size_t dest_len,
             const char *src,
             size_t src_len)
{
  if (dest == NULL || dest_len <= 0 || src == NULL || src_len < 0)
    return NULL;
  char *dest_end = dest + dest_len - 1;
  size_t src_real_len = strlen (src);
  if (src_len > src_real_len)
    src_len = src_real_len;
  const char *src_end = src + src_len;
  if (dest_len < src_len + 1)   /* The space in dest is not enough */
  {
    dest[0] = '\0';
    return NULL;
  }
  strncpy (dest, src, src_len);
  dest[src_len] = '\0';
  return dest + src_len;
}

