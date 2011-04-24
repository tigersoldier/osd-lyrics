#include <string.h>
#include <glib.h>
#include "ol_music_info.h"
#include "ol_utils.h"
#include "ol_debug.h"

const int DEFAULT_TRACK_NUM = -1;

static void internal_set_string (char **string,
                                 const char *val);
static int internal_streq (const char *lhs,
                           const char *rhs);
static int internal_snprint (void *buffer,
                             size_t count,
                             char *val);

static void
internal_set_string (char **string,
                     const char *val)
{
  ol_assert (string != NULL);
  if (*string != NULL)
  {
    g_free (*string);
    *string = NULL;
  }
  if (val != NULL)
    *string = g_strdup (val);
}

OlMusicInfo *
ol_music_info_new ()
{
  OlMusicInfo *info = g_new (OlMusicInfo, 1);
  ol_music_info_init (info);
  return info;
}

void
ol_music_info_init (OlMusicInfo *info)
{
  ol_assert (info != NULL);
  info->artist = NULL;
  info->title = NULL;
  info->album = NULL;
  info->uri = NULL;
  info->track_number = DEFAULT_TRACK_NUM;
}

void
ol_music_info_clear (OlMusicInfo *info)
{
  ol_assert (info != NULL);
  ol_music_info_set_title (info, NULL);
  ol_music_info_set_artist (info, NULL);
  ol_music_info_set_album (info, NULL);
  ol_music_info_set_track_number (info, DEFAULT_TRACK_NUM);
  ol_music_info_set_uri (info, NULL);
}

void
ol_music_info_copy (OlMusicInfo *dest, const OlMusicInfo *src)
{
  ol_assert (dest != NULL);
  ol_assert (src != NULL);
  if (dest == src)
    return;
  ol_music_info_set_title (dest, src->title);
  ol_music_info_set_artist (dest, src->artist);
  ol_music_info_set_album (dest, src->album);
  ol_music_info_set_track_number (dest, src->track_number);
  ol_music_info_set_uri (dest, src->uri);
}

void
ol_music_info_set_title (OlMusicInfo *music_info,
                         const char *title)
{
  ol_assert (music_info != NULL);
  internal_set_string (&(music_info->title), title);
}

const char *
ol_music_info_get_title (const OlMusicInfo *music_info)
{
  ol_assert_ret (music_info != NULL, NULL);
  return music_info->title;
}

void
ol_music_info_set_artist (OlMusicInfo *music_info,
                          const char *artist)
{
  ol_assert (music_info != NULL);
  internal_set_string (&(music_info->artist), artist);
}

const char *
ol_music_info_get_artist (const OlMusicInfo *music_info)
{
  ol_assert_ret (music_info != NULL, NULL);
  return music_info->artist;
}

void
ol_music_info_set_album (OlMusicInfo *music_info,
                         const char *album)
{
  ol_assert (music_info != NULL);
  internal_set_string (&(music_info->album), album);
}

const char *
ol_music_info_get_album (const OlMusicInfo *music_info)
{
  ol_assert_ret (music_info != NULL, NULL);
  return music_info->album;
}

void
ol_music_info_set_track_number (OlMusicInfo *music_info,
                                int track_number)
{
  ol_assert (music_info != NULL);
  music_info->track_number = track_number;
}

int
ol_music_info_get_track_number (const OlMusicInfo *music_info)
{
  ol_assert_ret (music_info != NULL, DEFAULT_TRACK_NUM);
  return music_info->track_number;
}

void
ol_music_info_set_uri (OlMusicInfo *music_info,
                       const char *uri)
{
  ol_assert (music_info != NULL);
  internal_set_string (&(music_info->uri), uri);
}

const char *
ol_music_info_get_uri (const OlMusicInfo *music_info)
{
  ol_assert_ret (music_info != NULL, NULL);
  return music_info->uri;
}

void
ol_music_info_destroy (OlMusicInfo *music_info)
{
  ol_assert (music_info != NULL);
  ol_music_info_clear (music_info);
  g_free (music_info);
}

static int
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
    if (cnt < count)
      buffer[cnt] = '\0';
    else if (count > 0)
      buffer[count - 1] = '\0';
  }
  return cnt;
}

int
ol_music_info_deserialize (OlMusicInfo *music_info,
                           const char *data)
{
  ol_assert_ret (music_info != NULL, 0);
  ol_assert_ret (data != NULL, 0);
  int ret = 1;
  char *buffer = g_strdup (data);
  char *title, *artist, *album, *track_number, *uri;
  title = artist = album = track_number = uri = NULL;
  title = buffer;
  if ((artist = ol_split_a_line (title)) == NULL)
    ret = 0;
  else if ((album = ol_split_a_line (artist)) == NULL)
    ret = 0;
  else if ((track_number = ol_split_a_line (album)) == NULL)
    ret = 0;
  else if ((uri = ol_split_a_line (track_number)) == NULL)
    ret = 0;
  if ((ol_split_a_line (uri)) == NULL)
    ret = 0;
  if (ret)
  {
    int tn = 0;
    sscanf (track_number, "%d", &tn);
    ol_music_info_set_title (music_info, title);
    ol_music_info_set_artist (music_info, artist);
    ol_music_info_set_album (music_info, album);
    ol_music_info_set_track_number (music_info, tn);
    ol_music_info_set_uri (music_info, uri);
  }
  g_free (buffer);
  return ret;
}

static int
internal_streq (const char *lhs,
                const char *rhs)
{
  if (lhs == rhs)
    return 1;
  if (lhs == NULL || rhs == NULL)
    return 0;
  return strcmp (lhs, rhs) == 0;
}

int
ol_music_info_equal (const OlMusicInfo *lhs,
                     const OlMusicInfo *rhs)
{
  if (lhs == rhs)
    return 1;
  if (lhs == NULL || rhs == NULL)
    return 0;
  if (!internal_streq (lhs->title, rhs->title))
  {
    return 0;
  }
  if (!internal_streq (lhs->artist, rhs->artist))
  {
    return 0;
  }
  if (!internal_streq (lhs->album, rhs->album))
  {
    return 0;
  }
  if (lhs->track_number != rhs->track_number)
  {
    return 0;
  }
  if (!internal_streq (lhs->uri, rhs->uri))
  {
    return 0;
  }
  return 1;
}
