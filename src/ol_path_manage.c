#include <stdio.h>
#include <glib.h>
#include "ol_path_manage.h"
#include "ol_utils.h"

#define BUFFER_SIZE 1024

/** 
 * @brief duplicates the string and replace all invalid characters to `_' 
 * 
 * @param str string needs to be replaced
 * 
 * @return replaced string, should be free with g_free
 */
static char* replace_invalid_str (const char *str);

/** 
 * @brief Gets the filename from a uri
 * 
 * @param dest The buffer to store the filename
 * @param dest_len The size of the buffer
 * @param uri The URI of a file
 * @param with_ext If FALSE, the returned filename will not contain extention (i.e. `.mp3')
 * 
 * @return The end of the destination if succeeded, or NULL if failed.
 */
static char* ol_uri_get_filename (char *dest,
                                  size_t dest_len,
                                  const char *uri,
                                  gboolean with_ext);

size_t
ol_path_get_lrc_pathname (const char *path_pattern,
                          const char *file_pattern,
                          OlMusicInfo *music_info,
                          char *pathname,
                          size_t len)
{
  if (path_pattern == NULL || file_pattern == NULL ||
      music_info == NULL || pathname == NULL || len <= 0)
    return -1;
  size_t current = ol_path_expand_path_pattern (path_pattern,
                                                music_info,
                                                pathname,
                                                len);
  if (current == -1)
    return -1;
  if (pathname[current - 1] != '/')
  {
    pathname[current] = '/';
    current++;
    pathname[current]  = '\0';
  }
  size_t offset = ol_path_expand_file_pattern (file_pattern,
                                               music_info,
                                               pathname + current,
                                               len - current);
  if (offset == -1)
    return -1;
  current += offset;
  if (ol_stricmp (&pathname[current - 4], ".lrc", 4) != 0)
  {
    char *end = ol_strnncpy (pathname + current, len - current,
                             ".lrc", 4);
    if (end == NULL)
      return -1;
    current = end - pathname;
  }
  return current;
}

static char*
replace_invalid_str (const char *str)
{
  if (str == NULL)
    return NULL;
  char *ret = g_strdup (str);
  char *rep = ret;
  while ((rep = strchr (rep, '/')) != NULL)
  {
    *rep = '_';
  }
  return ret;
}

size_t
ol_path_expand_file_pattern (const char *pattern,
                             OlMusicInfo *music_info,
                             char *filename,
                             size_t len)
{
  char buffer[BUFFER_SIZE];
  if (pattern == NULL)
    return -1;
  if (music_info == NULL)
    return -1;
  if (filename == NULL || len <= 0)
    return -1;
  const char *ptr1 = pattern;
  const char *pat_end = pattern + strlen (pattern);
  char *current = filename;
  char *end = filename + len;
  while (ptr1 < pat_end)
  {
    const char *ptr2 = strchr (ptr1, '%'); /* find place holder */
    if (ptr2 == NULL)                    /* no more place holders */
      ptr2 = pat_end; 
    current = ol_strnncpy (current, end - current, ptr1, ptr2 - ptr1);
    if (current == NULL)        /* The buffer is not big enough */
      return -1;
    if (ptr2 < pat_end)
    {
      char *append = NULL;
      size_t delta = 2;
      switch (*(ptr2 + 1))
      {
      case 't':                   /* title */
        append = music_info->title;
        break;
      case 'p':                   /* artist */
        append = music_info->artist;
        break;
      case 'a':                 /* album */
        append = music_info->album;
        break;
      case 'n':                 /* track number */
        snprintf (buffer, BUFFER_SIZE, "%d", music_info->track_number);
        append = buffer;
        break;
      case 'f':                 /* file name */
        if (music_info->uri == NULL)
          append = NULL;
        else
        {
          if (ol_uri_get_filename (buffer, BUFFER_SIZE, music_info->uri, FALSE))
            append = buffer;
        }
        break;
      case '%':
        append = "%";
        break;
      default:
        append = "%";
        delta = 1;
        break;
      } /* switch */
      if (append == NULL)
        return -1;
      current = ol_strnncpy (current, end - current,
                             append, strlen (append));
      if (current == NULL)
        return -1;
      ptr2 += delta;
    } /* if (ptr2 < pat_end) */
    ptr1 = ptr2;
  } /* while */
  return current - filename;
}

static const char*
ol_uri_get_query_start (const char* uri)
{
  const char *end = g_utf8_strrchr (uri, strlen (uri), '?');
  if (end == NULL)
    end = uri + strlen (uri);
  const char *begin = end;
  while (begin >= uri && *begin != '/') begin--;
  if (*begin == '/') begin++;
  if (begin >= end)
    return NULL;
  return begin;
}

static char*
ol_uri_get_filename (char *dest,
                     size_t dest_len,
                     const char *uri,
                     gboolean with_ext)
{
  if (dest == NULL || dest_len <= 0 || uri == NULL)
    return NULL;
  const char *end = g_utf8_strrchr (uri, strlen (uri), '?');
  if (end == NULL)
    end = uri + strlen (uri);
  const char *begin = ol_uri_get_query_start (uri);
  gchar *file_name = g_uri_unescape_segment (begin, end, NULL);
  if (file_name == NULL)
    return NULL;
  if (!with_ext)
  {
    gchar *ext = g_utf8_strrchr (file_name, strlen (uri), '.');
    if (ext != NULL)
      *ext = '\0';
  }
  char *ret = ol_strnncpy (dest, dest_len, file_name, strlen (file_name));
  g_free (file_name);
  return ret;
}

static char*
ol_uri_get_path (char *dest,
                 size_t dest_len,
                 const char *uri)
{
  if (dest == NULL || dest_len <= 0 || uri == NULL)
    return NULL;
  const char *end = ol_uri_get_query_start (uri);
  if (end == NULL)
    return NULL;
  const char *begin = strchr (uri, ':');
  if (begin == NULL)
    return NULL;
  if (begin[1] == '/' && begin[2] == '/')
    begin += 3;
  while (*begin != '\0' && *begin != '/')
    begin++;
  if (*begin != '/') return NULL;
  gchar *file_name = g_uri_unescape_segment (begin, end, NULL);
  if (file_name == NULL)
    return NULL;
  char *ret = ol_strnncpy (dest, dest_len, file_name, strlen (file_name));
  g_free (file_name);
  return ret;
}

size_t
ol_path_expand_path_pattern (const char *pattern,
                             OlMusicInfo *music_info,
                             char *filename,
                             size_t len)
{
  if (pattern == NULL || filename == NULL || len <= 0)
    return -1;
  if (strcmp (pattern, "%") == 0) /* use music's path */
  {
    if (music_info == NULL || music_info->uri == NULL)
      return -1;
    char *end = ol_uri_get_path (filename, len, music_info->uri);
    if (end == NULL)
      return -1;
    return end - filename;
  }
  if (pattern[0] == '~' && pattern[1] == '/') /* relative to home */
  {
    const char *home_dir;
    home_dir = g_get_home_dir ();
    char *end = ol_strnncpy (filename, len, home_dir, strlen (home_dir));
    if (end == NULL)
      return -1;
    end = ol_strnncpy (end, filename + len - end,
                       pattern + 1, strlen (pattern + 1));
    if (end == NULL)
      return -1;
    return end - filename;
  }
  /* Others, copy directly */
  char *end = ol_strnncpy (filename, len, pattern, strlen (pattern));
  if (end == NULL)
    return -1;
  else
    return end - filename;
}
