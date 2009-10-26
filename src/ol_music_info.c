#include "ol_music_info.h"
#include <glib.h>

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
}

void
ol_music_info_finalize (OlMusicInfo *music_info)
{
  g_return_if_fail (music_info != NULL);
  if (music_info->album != NULL)
    g_free (music_info->album);
  music_info->album = NULL;
  if (music_info->title != NULL)
    g_free (music_info->title);
  music_info->title = NULL;
  if (music_info->artist != NULL)
    g_free (music_info->artist);
  music_info->artist = NULL;
}

void
ol_music_info_destroy (OlMusicInfo *music_info)
{
  g_return_if_fail (music_info != NULL);
  ol_music_info_finalize (music_info);
  g_free (music_info);
}
