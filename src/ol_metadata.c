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
#include <string.h>
#include <glib.h>
#include "ol_metadata.h"
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

OlMetadata *
ol_metadata_new ()
{
  OlMetadata *info = g_new (OlMetadata, 1);
  ol_metadata_init (info);
  return info;
}

void
ol_metadata_free (OlMetadata *metadata)
{
  if (metadata == NULL)
    return;
  ol_metadata_clear (metadata);
  g_free (metadata);
}

void
ol_metadata_init (OlMetadata *info)
{
  ol_assert (info != NULL);
  info->artist = NULL;
  info->title = NULL;
  info->album = NULL;
  info->uri = NULL;
  info->track_number = DEFAULT_TRACK_NUM;
}

void
ol_metadata_clear (OlMetadata *info)
{
  ol_assert (info != NULL);
  ol_metadata_set_title (info, NULL);
  ol_metadata_set_artist (info, NULL);
  ol_metadata_set_album (info, NULL);
  ol_metadata_set_track_number (info, DEFAULT_TRACK_NUM);
  ol_metadata_set_uri (info, NULL);
}

void
ol_metadata_copy (OlMetadata *dest, const OlMetadata *src)
{
  ol_assert (dest != NULL);
  ol_assert (src != NULL);
  if (dest == src)
    return;
  ol_metadata_set_title (dest, src->title);
  ol_metadata_set_artist (dest, src->artist);
  ol_metadata_set_album (dest, src->album);
  ol_metadata_set_track_number (dest, src->track_number);
  ol_metadata_set_uri (dest, src->uri);
}

void
ol_metadata_set_title (OlMetadata *metadata,
                       const char *title)
{
  ol_assert (metadata != NULL);
  internal_set_string (&(metadata->title), title);
}

const char *
ol_metadata_get_title (const OlMetadata *metadata)
{
  ol_assert_ret (metadata != NULL, NULL);
  return metadata->title;
}

void
ol_metadata_set_artist (OlMetadata *metadata,
                        const char *artist)
{
  ol_assert (metadata != NULL);
  internal_set_string (&(metadata->artist), artist);
}

const char *
ol_metadata_get_artist (const OlMetadata *metadata)
{
  ol_assert_ret (metadata != NULL, NULL);
  return metadata->artist;
}

void
ol_metadata_set_album (OlMetadata *metadata,
                       const char *album)
{
  ol_assert (metadata != NULL);
  internal_set_string (&(metadata->album), album);
}

const char *
ol_metadata_get_album (const OlMetadata *metadata)
{
  ol_assert_ret (metadata != NULL, NULL);
  return metadata->album;
}

void
ol_metadata_set_track_number (OlMetadata *metadata,
                              int track_number)
{
  ol_assert (metadata != NULL);
  metadata->track_number = track_number;
}

void
ol_metadata_set_track_number_from_string (OlMetadata *metadata,
                                          const char *track_number)
{
  ol_assert (metadata != NULL);
  if (track_number == NULL)
    metadata->track_number = DEFAULT_TRACK_NUM;
  else
    sscanf (track_number, "%d", &metadata->track_number);
}

int
ol_metadata_get_track_number (const OlMetadata *metadata)
{
  ol_assert_ret (metadata != NULL, DEFAULT_TRACK_NUM);
  return metadata->track_number;
}

void
ol_metadata_set_uri (OlMetadata *metadata,
                     const char *uri)
{
  ol_assert (metadata != NULL);
  internal_set_string (&(metadata->uri), uri);
}

const char *
ol_metadata_get_uri (const OlMetadata *metadata)
{
  ol_assert_ret (metadata != NULL, NULL);
  return metadata->uri;
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
ol_metadata_serialize (OlMetadata *metadata,
                       char *buffer,
                       size_t count)
{
  ol_assert_ret (metadata != NULL, 0);
  int cnt = 0;
  if (buffer == NULL)
  {
    if (metadata->title != NULL)
      cnt += strlen (metadata->title);
    cnt++;
    if (metadata->artist != NULL)
      cnt += strlen (metadata->artist);
    cnt++;
    if (metadata->album != NULL)
      cnt += strlen (metadata->album);
    cnt++;
    static char tmpbuf[100];
    cnt += snprintf (tmpbuf, 100, "%d\n", metadata->track_number);
    if (metadata->uri != NULL)
      cnt += strlen (metadata->uri);
    cnt++;
  }
  else
  {
    cnt += internal_snprint (buffer + cnt,
                             count - cnt,
                             metadata->title);
    cnt += internal_snprint (buffer + cnt,
                             count - cnt,
                             metadata->artist);
    cnt += internal_snprint (buffer + cnt,
                             count - cnt,
                             metadata->album);
    cnt += snprintf (buffer + cnt,
                     count - cnt,
                     "%d\n",
                     metadata->track_number);
    cnt += internal_snprint (buffer + cnt,
                             count - cnt,
                             metadata->uri);
    if (cnt < count)
      buffer[cnt] = '\0';
    else if (count > 0)
      buffer[count - 1] = '\0';
  }
  return cnt;
}

int
ol_metadata_deserialize (OlMetadata *metadata,
                         const char *data)
{
  ol_assert_ret (metadata != NULL, 0);
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
    ol_metadata_set_title (metadata, title);
    ol_metadata_set_artist (metadata, artist);
    ol_metadata_set_album (metadata, album);
    ol_metadata_set_track_number (metadata, tn);
    ol_metadata_set_uri (metadata, uri);
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
ol_metadata_equal (const OlMetadata *lhs,
                   const OlMetadata *rhs)
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

