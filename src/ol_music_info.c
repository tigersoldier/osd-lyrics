#include <string.h>
#include <glib.h>
#include "ol_music_info.h"
#include "ol_debug.h"

void
ol_music_info_init (OlMusicInfo *info)
{
  g_return_if_fail (info != NULL);
  info->artist = NULL;
  info->title = NULL;
  info->album = NULL;
  info->uri = NULL;
  info->track_number = -1;
}

void
ol_music_info_clear (OlMusicInfo *info)
{
  g_return_if_fail (info != NULL);
  if (info->title)
  {
    g_free (info->title);
    info->title = NULL;
  }
  if (info->artist)
  {
    g_free (info->artist);
    info->artist = NULL;
  }
  if (info->album)
  {
    g_free (info->album);
    info->album = NULL;
  }
  if (info->uri)
  {
    g_free (info->uri);
    info->uri = NULL;
  }
  info->track_number = 0;
}

void
ol_music_info_copy (OlMusicInfo *dest, const OlMusicInfo *src)
{
  g_return_if_fail (dest != NULL);
  g_return_if_fail (src != NULL);
  if (dest == src)
    return;
  gchar *temp;
  if (src->artist != NULL)
    temp = g_strdup (src->artist);
  else
    temp = NULL;
  if (dest->artist != NULL)
  {
    g_free (dest->artist);
  }
  dest->artist = temp;

  if (src->title != NULL)
    temp = g_strdup (src->title);
  else
    temp = NULL;
  if (dest->title != NULL)
  {
    g_free (dest->title);
  }
  dest->title = temp;

  if (src->album != NULL)
    temp = g_strdup (src->album);
  else
    temp = NULL;
  if (dest->album != NULL)
  {
    g_free (dest->album);
  }
  dest->album = temp;
  
  if (src->uri != NULL)
    temp = g_strdup (src->uri);
  else
    temp = NULL;
  if (dest->uri != NULL)
  {
    g_free (dest->uri);
  }
  dest->uri = temp;
}

void
ol_music_info_destroy (OlMusicInfo *music_info)
{
  g_return_if_fail (music_info != NULL);
  ol_music_info_clear (music_info);
  g_free (music_info);
}

int
internal_snprint (void *buffer,
                  size_t count,
                  char *val)
{
  if (val == NULL)
    val = "";
  return snprintf (buffer, count, "%s\n", val);
}

int
ol_music_info_serialize (OlMusicInfo *music_info,
                         char *buffer,
                         size_t count)
{
  ol_assert_ret (music_info != NULL, 0);
  static const char empty_string[] = "";
  int cnt = 0;
  if (buffer == NULL)
  {
    if (music_info->title != NULL)
      cnt += strlen (music_info->title);
    cnt++;
    if (music_info->artist != NULL)
      cnt += strlen (music_info->artist);
    cnt++;
    if (music_info->album != NULL)
      cnt += strlen (music_info->album);
    cnt++;
    static char tmpbuf[100];
    cnt += snprintf (tmpbuf, 100, "%d\n", music_info->track_number);
    if (music_info->uri != NULL)
      cnt += strlen (music_info->uri);
    cnt++;
  }
  else
  {
    cnt += internal_snprint (buffer + cnt,
                             count - cnt,
                             music_info->title);
    cnt += internal_snprint (buffer + cnt,
                             count - cnt,
                             music_info->artist);
    cnt += internal_snprint (buffer + cnt,
                             count - cnt,
                             music_info->album);
    cnt += snprintf (buffer + cnt,
                     count - cnt,
                     "%d\n",
                     music_info->track_number);
    cnt += internal_snprint (buffer + cnt,
                             count - cnt,
                             music_info->uri);
  }
  return cnt;
}
