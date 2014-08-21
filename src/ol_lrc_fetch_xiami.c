/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009-2011  Tiger Soldier <tigersoldi@gmail.com>
 *
 * This file is part of OSD Lyrics.
 * 
 * OSD Lyrics is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OSD Lyrics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OSD Lyrics.  If not, see <http://www.gnu.org/licenses/>. 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <glib.h>
#include "ol_lrc_fetch.h"
#include "ol_lrc_fetch_xiami.h"
#include "ol_utils.h"
#include "ol_intl.h"
#include "ol_debug.h"

static const char *PREFIX_SEARCH_URL = "http://www.xiami.com/search?key=";
static const char *PREFIX_LRC_URL = "http://www.xiami.com/song/playlist/id/";
static const char *TITLE_PATTERN = "href=\"http://www.xiami.com/song/";
static const char *ARTIST_PATTERN = "href=\"http://www.xiami.com/artist/";
/* static const char *ALBUM_PATTERN = "href=\"/album/"; */
/* #define PREFIX_LRC_XIAMI "http://mp3.xiami.com/" */
/* #define LRC_CLUE_XIAMI "downlrc" */
#define TRY_MATCH_MAX 5

static char* _get_title_attr (char *content, char **end);

static char*
_get_title_attr (char *content, char **end)
{
  ol_assert_ret (content != NULL, NULL);
  ol_assert_ret (end != NULL, NULL);
  *end = content;
  char *p = strstr (content, "title=\"");
  if (p == NULL)
  {
    return NULL;
  }
  content = p + strlen ("title=\"");
  char *q = strchr (content, '"');
  if (q == NULL)
  {
    return NULL;
  }
  *end = q + 1;
  return g_strndup (content, q - content);
}

static char*
_get_lyric_url (const char *song_id)
{
  ol_assert_ret (song_id != NULL, NULL);
  char *url = g_strdup_printf ("%s%s", PREFIX_LRC_URL, song_id);
  struct memo content;
  int result;
  content.mem_base = NULL;
  content.mem_len = 0;
  if ((result = fetch_into_memory (url,
                                   NULL, /* refer */
                                   NULL, /* user-agent */
                                   NULL, /* postdata */
                                   0,    /* postdata len */
                                   &content)) < 0)
  {
    if (content.mem_base != NULL)
      free (content.mem_base);
    return NULL;
  }
  ol_info ("result: %d\nurl: %s\ncontent: %s\n", result, url, content.mem_base);
  g_free (url);
  char *start = strstr ((char*)content.mem_base,
                        "<lyric>");
  if (start == NULL)
    return NULL;
  start += strlen ("<lyric>");
  while (*start && isspace (*start))
    start++;
  char *end = strstr (start, "</lyric>");
  if (end == NULL)
    return NULL;
  while (end > start && isspace(*(end - 1)))
    end--;
  if (end == start)
    return NULL;
  char *ret = g_strndup (start, end - start);
  if (!g_str_has_suffix (ret, ".lrc"))
  {
    g_free (ret);
    ret = NULL;
  }
  free (content.mem_base);
  return ret;
}

static OlLrcCandidate *
ol_lrc_fetch_xiami_search(const OlMusicInfo *info, int *size, const char* charset)
{
  ol_assert_ret (info != NULL, NULL);
  ol_debugf ("  title: %s\n"
             "  artist: %s\n"
             "  album: %s\n",
             info->title,
             info->artist,
             info->album);
  OlLrcCandidate *ret = NULL;
  static OlLrcCandidate candidate_list[TRY_MATCH_MAX];
  OlLrcCandidate candidate;
  int result;
  int count = 0;
  if (info->title == NULL && info->artist == NULL)
    return NULL;

  GString *url = g_string_new (PREFIX_SEARCH_URL);
  if (ol_music_info_get_title (info) != NULL)
    g_string_append_uri_escaped (url,
                                 ol_music_info_get_title (info),
                                 NULL,
                                 FALSE);
  if (ol_music_info_get_artist (info) != NULL)
  {
    g_string_append_c (url, '+');
    g_string_append_uri_escaped (url,
                                 ol_music_info_get_artist (info),
                                 NULL,
                                 FALSE);
  }

  char *search_url = g_string_free (url, FALSE);
  ol_debugf ("Search url: %s\n", search_url);

  struct memo content;
  content.mem_base = NULL;
  content.mem_len = 0;

  if((result = fetch_into_memory(search_url,
                                 NULL, /* refer */
                                 NULL, /* user-agent */
                                 NULL, /* postdata */
                                 0,    /* postdata len */
                                 &content)) >= 0)
  {
    char *current = (char*)content.mem_base;
    char *p;
    while ((p = strstr (current, TITLE_PATTERN)) != NULL)
    {
      current = p + strlen (TITLE_PATTERN);
      if (!isdigit(*current))
        continue;
      p = strchr (current, '\"');
      if (p == NULL)
        break;
      *p = '\0';
      ol_debugf ("url id: %s\n", current);
      char *id = g_strdup (current);
      current = p + 1;
      char *title = _get_title_attr (current, &current);
      if (title == NULL)
        break;
      ol_lrc_candidate_set_title (&candidate, title);
      ol_debugf ("title: %s\n", title);
      g_free (title);
      p = strstr (current, ARTIST_PATTERN);
      if (p != NULL)
      {
        char *artist = _get_title_attr (p, &current);
        if (artist != NULL)
        {
          ol_lrc_candidate_set_artist (&candidate, artist);
          ol_debugf ("artist: %s\n", artist);
          g_free (artist);
        }
      }

      if (ol_lrc_fetch_calc_rank (info, &candidate) >= LRC_RANK_THRESHOLD)
      {
        char *url = _get_lyric_url (id);
        ol_debugf ("lyric url: %s\n", url);
        if (url != NULL)
        {
          ol_lrc_candidate_set_url (&candidate, url);
          count = ol_lrc_fetch_add_candidate (info,
                                              candidate_list,
                                              count,
                                              TRY_MATCH_MAX,
                                              &candidate);
          g_free (url);
        }
      }
      g_free (id);
    }
    free(content.mem_base);
    if (count > 0)
      ret = candidate_list;
  }
  *size = count;
  g_free (search_url);
  return ret;
}

int 
ol_lrc_fetch_xiami_download(OlLrcCandidate *candidate,
                            const char *pathname,
                            const char *charset)
{
  ol_log_func ();
  ol_assert_ret (candidate != NULL, -1);
  ol_assert_ret (pathname != NULL, -1);
  FILE *fp;
  int ret = 0;

  if ((fp = fopen(pathname, "w")) == NULL)
  {
    ret = -1;
  }
  else
  {
    fetch_into_file (ol_lrc_candidate_get_url (candidate),
                     NULL,
                     fp);
    fclose(fp);
    ret = 0;
  }
  return ret;
}

static OlLrcFetchEngine xiami = {
  N_("Xiami"),
  ol_lrc_fetch_xiami_search,
  ol_lrc_fetch_xiami_download,
};

OlLrcFetchEngine *ol_lrc_fetch_xiami_engine ()
{
  return &xiami;
}
