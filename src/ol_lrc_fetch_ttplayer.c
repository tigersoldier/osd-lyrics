/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009-2011  Tiger Soldier <tigersoldi@gmail.com>
 *
 * This file is part of OSD Lyrics.
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
#include <string.h>
#include <glib.h>
#include "ol_intl.h"
#include "ol_lrc_fetch_ttplayer.h"
#include "ol_lrc_fetch_utils.h"
#include "ol_utils.h"
#include "ol_debug.h"

#define MAX_CANDIDATE 5

const char *PREFIX_SEARCH_URL = "http://ttlrcct.qianqian.com/dll/lyricsvr.dll?sh?";
const char *DOWNLOAD_URL = "http://ttlrcct.qianqian.com/dll/lyricsvr.dll?dl?Id=%d&Code=%d&uid=01&mac=%012x";
const char *FIELD_DELIMIT = "\'";

struct CandidateParserData
{
  const OlMusicInfo *info;
  OlLrcCandidate *candidate_list;
  int count;
};

static OlLrcCandidate* _search(const OlMusicInfo *info,
                               int *size,
                               const char* charset);
static int _download (OlLrcCandidate *candidate,
                      const char *pathname,
                      const char *charset);
static char* _encode_request_field (const char *value);

static void _parse_candidate (GMarkupParseContext *context,
                              const gchar *element_name,
                              const gchar **attribute_names,
                              const gchar **attribute_values,
                              gpointer user_data,
                              GError **error);
static size_t _remove_whitespace (char *str);
static gint32 _calc_download_code (int id, const char *data);

static gint32
_calc_download_code (int id, const char *data)
{
  guint32 id1 = id & 0xff;
  guint32 id2 = (id >> 8) & 0xff;
  guint32 id3 = (id >> 16) & 0xff;
  guint32 id4 = (id >> 24) & 0xff;

  if (id3 == 0) id3 = ~id2 & 0xff;
  if (id4 == 0) id4 = ~id1 & 0xff;
  gint32 encoded_id = (id1 << 24) | (id3 << 16) | (id2 << 8) | id4;
  gint32 encoded_data = 0;
  size_t len = strlen(data);
  int i;
  for (i = len - 1; i >= 0; i--)
    encoded_data = encoded_data + data[i] + (encoded_data << (i % 2 + 4));
  gint32 encoded_data2 = 0;
  for (i = 0; i < len; i++)
    encoded_data2 = encoded_data2 + data[i] + (encoded_data2 << (i % 2 + 3));
  gint32 ret = ((encoded_data ^ encoded_id) + (encoded_data2 | id)) * (encoded_data2 | encoded_id);
  ret = ret * (encoded_data ^ id);
  return ret;
}

static void
_parse_candidate (GMarkupParseContext *context,
                  const gchar *element_name,
                  const gchar **attribute_names,
                  const gchar **attribute_values,
                  gpointer user_data,
                  GError **error)
{
  ol_debugf ("Got element: %s\n", element_name);
  if (strcmp (element_name, "lrc") == 0)
  {
    const char **attr;
    const char **value;
    const char *url = NULL;
    const char *title = NULL;
    const char *artist = NULL;
    struct CandidateParserData *data = (struct CandidateParserData*) user_data;
    for (attr = attribute_names, value = attribute_values;
         *attr != NULL && *value != NULL;
         attr++, value++)
    {
      if (strcmp (*attr, "id") == 0)
        url = *value;
      else if (strcmp (*attr, "title") == 0)
        title = *value;
      else if (strcmp (*attr, "artist") == 0)
        artist = *value;
      else
        ol_debugf ("Unknown attribute: %s=%s\n", *attr, *value);
    }
    if (url != NULL && (title != NULL || artist != NULL))
    {
      OlLrcCandidate *candidate = ol_lrc_candidate_new ();
      ol_lrc_candidate_set_url (candidate, url);
      if (title != NULL)
        ol_lrc_candidate_set_title (candidate, title);
      if (artist != NULL)
        ol_lrc_candidate_set_artist (candidate, artist);
      data->count = ol_lrc_fetch_add_candidate (data->info,
                                                data->candidate_list,
                                                data->count,
                                                MAX_CANDIDATE,
                                                candidate);
      ol_lrc_candidate_free (candidate);
    }
  }
}

static size_t
_remove_whitespace (char *str)
{
  size_t len = 0;
  size_t tail = 0;
  for (; str[len] != '\0'; len++)
  {
    if (str[len] != ' ')
    {
      if (tail != len)
        str[tail] = str[len];
      tail++;
    }
  }
  str[tail] = '\0';
  return tail;
}

static char*
_encode_request_field (const char *value)
{
  size_t len = strlen (value);
  size_t utf16_len = len * 2 + 1;
  char *utf16 = g_new0 (char, utf16_len);
  char *hex = NULL;
  char *lower_str = g_utf8_strdown (value, -1);
  g_strdelimit (lower_str, FIELD_DELIMIT, ' ');
  len = _remove_whitespace (lower_str);
  utf16_len = convert ("UTF-8", "UTF16LE",
                       (char*)lower_str, len,
                       utf16, utf16_len);
  if ((int)utf16_len >= 0)
    hex = ol_encode_hex (utf16, utf16_len);
  g_free (lower_str);
  g_free (utf16);
  return hex;
}

static OlLrcCandidate *
_search(const OlMusicInfo *info,
        int *size,
        const char* charset)
{
  ol_assert_ret (info != NULL, NULL);
  ol_assert_ret (size != NULL, NULL);
  if (ol_music_info_get_title (info) == NULL ||
      ol_music_info_get_artist (info) == NULL)
    return NULL;
  static OlLrcCandidate candidate_list[MAX_CANDIDATE];
  OlLrcCandidate *retval = NULL;
  GString *surl = g_string_new (PREFIX_SEARCH_URL);
  char *encoded_title = _encode_request_field (ol_music_info_get_title (info));
  char *encoded_artist = _encode_request_field (ol_music_info_get_artist (info));
  g_string_append_printf (surl,
                          "Artist=%s&Title=%s&Flags=2",
                          encoded_artist, encoded_title);
  g_free (encoded_title);
  g_free (encoded_artist);
  char *url = g_string_free (surl, FALSE);
  ol_debugf ("url: %s\n", url);
  struct memo content = {.mem_base = NULL, .mem_len = 0};
  if (fetch_into_memory (url, NULL, NULL, NULL, 0, &content) != 0)
  {
    ol_debugf ("Search lyrics failed\n");
    if (content.mem_base != NULL)
      free (content.mem_base);
  }
  else
  {
    GMarkupParser parser = {.start_element = _parse_candidate};
    GError *error = NULL;
    struct CandidateParserData data = {
      .info = info,
      .candidate_list = candidate_list,
      .count = 0,
    };
    GMarkupParseContext *context = g_markup_parse_context_new (&parser,
                                                               0,
                                                               &data,
                                                               NULL);
    ol_debugf ("Search result: %s\n", content.mem_base);
    if (!g_markup_parse_context_parse (context,
                                       (const char*)content.mem_base,
                                       content.mem_len,
                                       &error))
    {
      ol_errorf ("failed to parse: %s\n", error->message);
      g_error_free (error);
    }
    else if (!g_markup_parse_context_end_parse (context, &error))
    {
      ol_errorf ("failed to parse: %s\n", error->message);
      g_error_free (error);
    }
    g_markup_parse_context_free (context);
    retval = candidate_list;
    *size = data.count;
    free (content.mem_base);
  }
  g_free (url);
  return retval;
}

static int
_download(OlLrcCandidate *candidate,
          const char *pathname,
          const char *charset)
{
  ol_assert_ret (candidate != NULL, -1);
  ol_assert_ret (pathname != NULL, -1);
  int ret = 0;
  int id = atoi (candidate->url);
  char *data = g_strdup_printf ("%s%s", candidate->artist, candidate->title);
  int code = _calc_download_code (id, data);
  int mac = random ();
  char *url = g_strdup_printf (DOWNLOAD_URL, id, code, mac);
  ol_debugf ("Download url: %s\n", url);
  struct memo content = {.mem_base = NULL, .mem_len = 0};
  if (fetch_into_memory (url, NULL, NULL, NULL, 0, &content) < 0 ||
      content.mem_len == 0)
  {
    ol_errorf ("Download lyrics (%s, %s, %s) failed\n",
               candidate->url, candidate->title, candidate->artist);
    ret = -1;
  }
  else
  {
    GError *error = NULL;
    if (!g_file_set_contents (pathname,
                              content.mem_base,
                              content.mem_len,
                              &error))
    {
      ol_errorf ("Cannot save lyric file %s: %s\n",
                 error->message);
      g_error_free (error);
      ret = -1;
    }
  }
  if (content.mem_base != NULL)
    free (content.mem_base);
  g_free (data);
  g_free (url);
  return ret;
}

static OlLrcFetchEngine ttplayer = {
  N_("ttPlayer"),
  _search,
  _download,
};

OlLrcFetchEngine *
ol_lrc_fetch_ttplayer_engine ()
{
  return &ttplayer;
}
