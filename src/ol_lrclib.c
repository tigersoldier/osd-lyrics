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
#include "ol_lrclib.h"
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <glib.h>
#include "ol_debug.h"

#define LRC_TABLE "lyrics"
#define FIELD_BUFLEN 1024
#define QUERY_BUFLEN 2048

static sqlite3 *db = NULL;
static char *errmsg = NULL;
static const char *CREATE_TABLE = 
  "CREATE TABLE IF NOT EXISTS " LRC_TABLE " ("
  "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
  "  title TEXT, artist TEXT, album TEXT, tracknum INTEGER,"
  "  uri TEXT UNIQUE ON CONFLICT REPLACE, lrcpath TEXT"
  ")";
static const char *ASSIGN_LYRIC =
  "INSERT OR REPLACE INTO " LRC_TABLE
  "  (title, artist, album, tracknum, uri, lrcpath) "
  "  VALUES (%s, %s, %s, %d, %s, %s)";
static const char *UPDATE_LYRIC =
  "UPDATE " LRC_TABLE
  "  SET lrcpath=%s"
  "  WHERE %s";
static const char *FIND_LYRIC = 
  "SELECT lrcpath FROM " LRC_TABLE " WHERE %s";

#define _show_error()            do {ol_error (sqlite3_errmsg (db));} while (0)

static int _set_where_with_info (const OlMetadata *info, char *where,
                                 size_t size);
/** 
 * @brief Copies an string and transformed as an SQL string form.
 *
 * If the string is NULL, it will be transformed to NULL
 * Otherwise, the string will be escaped and quoted in the new string.
 * 
 * @param dest The string to store the copied string.
 * @param src The string to be copied.
 * @param size The space left in dest.
 * 
 * @return The number of bytes returned in dest, or -1 if failed
 */
static int _copy_str (char *dest, const char *src, size_t size);

/** 
 * @brief Find the lrc file according to URI
 * 
 * @param uri The URI of the music, cannot be NULL
 * @param ret Return location of the LRC file
 * 
 * @return 1 if found, 0 if not found, -1 if error occurs
 */
static int _find_by_uri (const char *uri, char **lrcpath);

/** 
 * @brief Find the lrc file according to title, artist and album
 * 
 * @param info The music
 * @param lrcpath Return location of the LRC file
 * 
 * @return 1 if found, 0 if not found, -1 if error occurs
 */
static int _find_by_info (const OlMetadata *info, char **lrcpath);

static int
_set_where_with_info (const OlMetadata *metadata,
                      char *where,
                      size_t size)
{
  ol_assert_ret (ol_metadata_get_title (metadata) != NULL, -1);
  ol_assert_ret (where != NULL, -1);
  int cnt = 0;
  cnt += snprintf (where + cnt, FIELD_BUFLEN - cnt, "title = ");
  cnt += _copy_str (where + cnt,
                    ol_metadata_get_title (metadata),
                    FIELD_BUFLEN - cnt);
  if (ol_metadata_get_artist (metadata) == NULL)
  {
    cnt += snprintf (where + cnt, FIELD_BUFLEN - cnt, " AND artist is NULL");
  }
  else
  {
    cnt += snprintf (where + cnt, FIELD_BUFLEN - cnt, " AND artist = ");
    cnt += _copy_str (where + cnt,
                      ol_metadata_get_artist(metadata),
                      FIELD_BUFLEN - cnt);
  }
  if (ol_metadata_get_album (metadata) == NULL)
  {
    cnt += snprintf (where + cnt, FIELD_BUFLEN - cnt, " AND album is NULL");
  }
  else
  {
    cnt += snprintf (where + cnt, FIELD_BUFLEN - cnt, " AND album = ");
    cnt += _copy_str (where + cnt,
                      ol_metadata_get_album (metadata),
                      FIELD_BUFLEN - cnt);
  }
  cnt += snprintf (where + cnt, FIELD_BUFLEN - cnt, " AND uri is NULL");
  return cnt;
}

static int
_find_by_uri (const char *uri, char **lrcpath)
{
  int code;
  sqlite3_stmt *stmt;
  static char query[QUERY_BUFLEN] = "";
  static char where[FIELD_BUFLEN] = "";
  int cnt = 0;
  int ret = 0;
  ol_assert_ret (uri != NULL, -1);
  ol_assert_ret (lrcpath != NULL, -1);
  cnt += snprintf (where + cnt, FIELD_BUFLEN - cnt, "uri = ");
  cnt += _copy_str (where + cnt, uri, FIELD_BUFLEN - cnt);
  snprintf (query, FIELD_BUFLEN, FIND_LYRIC, where);
  ol_debugf ("%s\n", query);
  code = sqlite3_prepare_v2 (db, query, -1, &stmt, NULL);
  if (code != SQLITE_OK)
  {
    _show_error ();
    return -1;
  }
  code = sqlite3_step (stmt);
  if (code != SQLITE_ROW)
  {
    if (code == SQLITE_DONE)
    {
      *lrcpath = NULL;
      ret = 0;
    }
    else
    {
      ret = -1;
      _show_error ();
    }
  }
  else
  {
    const char *path = (const char*)sqlite3_column_text (stmt, 0);
    ol_debugf ("Path is: %s\n", path);
    *lrcpath = g_strdup (path);
    ret = 1;
  }
  sqlite3_finalize (stmt);
  return ret;
}

static int
_find_by_info (const OlMetadata *metadata, char **lrcpath)
{
  int code;
  sqlite3_stmt *stmt;
  static char query[QUERY_BUFLEN] = "";
  static char where[FIELD_BUFLEN] = "";
  int ret = 0;
  ol_assert_ret (ol_metadata_get_title (metadata) != NULL, -1);
  ol_assert_ret (lrcpath != NULL, -1);
  _set_where_with_info (metadata, where, FIELD_BUFLEN);
  snprintf (query, FIELD_BUFLEN, FIND_LYRIC, where);
  ol_debugf ("%s\n", query);
  code = sqlite3_prepare_v2 (db, query, -1, &stmt, NULL);
  if (code != SQLITE_OK)
  {
    _show_error ();
    return -1;
  }
  code = sqlite3_step (stmt);
  if (code != SQLITE_ROW)
  {
    if (code == SQLITE_DONE)
    {
      *lrcpath = NULL;
      ret = 0;
    }
    else
    {
      ret = -1;
      _show_error ();
    }
  }
  else
  {
    const char *path = (const char*)sqlite3_column_text (stmt, 0);
    ol_debugf ("Path is: %s\n", path);
    *lrcpath = g_strdup (path);
    ret = 1;
  }
  sqlite3_finalize (stmt);
  return ret;
}

static int
_copy_str (char *dest, const char *src, size_t size)
{
  ol_assert_ret (dest != NULL, -1);
  if (src == NULL)
  {
    return snprintf (dest, size, "NULL");
  }
  else
  {
    int ret = 0;
    ret += snprintf (dest + ret, size - ret, "'");
    const char *ptr = src;
    while (ptr != NULL)
    {
      const char *next = strchr (ptr, '\'');
      if (next == NULL)
      {
        ret += snprintf (dest + ret, size - ret, "%s", ptr);
        break;
      }
      else
      {
        int maxsize = size - ret < next - ptr + 1 ? size - ret: next - ptr + 1;
        snprintf (dest + ret, maxsize, "%s", ptr);
        ret += maxsize - 1;
        ret += snprintf (dest + ret, size - ret, "''");
        ptr = next + 1;
      }
    }
    ret += snprintf (dest + ret, size - ret, "'");
    /* ol_debugf ("_copy_str: %s -> %s, ret: %d\n", src, dest, ret); */
    return ret;
  }
}

int 
ol_lrclib_init (const char *filename)
{
  ol_log_func ();
  if (db != NULL)
  {
    ol_error ("Lrclib has been initialized.");
    return 1;
  }
  int code = 0;
  code = sqlite3_open (filename, &db);
  if (code != SQLITE_OK)
  {
    _show_error ();
  }
  else
  {
    code = sqlite3_exec (db, CREATE_TABLE, NULL,
                         NULL, &errmsg);
    if (code != SQLITE_OK)
    {
      ol_error (errmsg);
      sqlite3_free (errmsg);
    }
  }
  return db != NULL;
}

void
ol_lrclib_unload ()
{
  ol_log_func ();
  if (db == NULL)
  {
    ol_error ("Lrclib database is not open");
  }
  else
  {
    sqlite3_close (db);
    db = NULL;
  }
}

int 
ol_lrclib_assign_lyric (const OlMetadata *metadata, 
                        const char *lrcpath)
{
  ol_log_func ();
  static char title_value[FIELD_BUFLEN] = "";
  static char artist_value[FIELD_BUFLEN] = "";
  static char album_value[FIELD_BUFLEN] = "";
  static char uri_value[FIELD_BUFLEN] = "";
  static char lrcpath_value[FIELD_BUFLEN] = "";
  static char where[FIELD_BUFLEN] = "";
  static char query[QUERY_BUFLEN] = "";
  if (db == NULL)
  {
    ol_error ("LrcLib is no initialized.");
    return 0;
  }
  ol_assert_ret (metadata != NULL, 0);
  if (ol_metadata_get_title (metadata) == NULL && 
      ol_metadata_get_uri (metadata) == NULL)
  {
    ol_error ("Require either title or uri be set.");
    return 0;
  }

  _copy_str (lrcpath_value, lrcpath, FIELD_BUFLEN);
  char *oldpath = NULL;
  int find_ret = -1;
  if (ol_metadata_get_uri (metadata) != NULL ||
      (find_ret = ol_lrclib_find (metadata, &oldpath)) == 0)
  {
    _copy_str (title_value, ol_metadata_get_title (metadata), FIELD_BUFLEN);
    _copy_str (artist_value, ol_metadata_get_artist (metadata), FIELD_BUFLEN);
    _copy_str (album_value, ol_metadata_get_album (metadata), FIELD_BUFLEN);
    _copy_str (uri_value, ol_metadata_get_uri (metadata), FIELD_BUFLEN);
    snprintf (query, QUERY_BUFLEN, ASSIGN_LYRIC,
              title_value, artist_value, album_value,
              ol_metadata_get_track_number (metadata),
              uri_value, lrcpath_value);
    ol_debugf ("query: %s\n", query);
    int code = sqlite3_exec (db, query, NULL, NULL, &errmsg);
    if (code != SQLITE_OK)
    {
      ol_errorf ("Assign Lyric Failed: %s\n", errmsg);
      sqlite3_free (errmsg);
    }
    return code == SQLITE_OK;
  }
  else
  {
    if (oldpath != NULL) g_free (oldpath);
    int retval = _set_where_with_info (metadata, where, FIELD_BUFLEN);
    if (retval == -1)
      return 0;
    snprintf (query, QUERY_BUFLEN, UPDATE_LYRIC, lrcpath_value, where);
    ol_debugf ("update query: %s\n", query);
    int code = sqlite3_exec (db, query, NULL, NULL, &errmsg);
    if (code != SQLITE_OK)
    {
      ol_errorf ("Update Lyric Failed %s\n", errmsg);
      sqlite3_free (errmsg);
    }
    return code == SQLITE_OK;
  }
}

int 
ol_lrclib_find (const OlMetadata *metadata,
                char **lrcpath)
{
  ol_log_func ();
  ol_assert_ret (metadata != NULL, 0);
  ol_assert_ret (lrcpath != NULL, 0);
  ol_assert_ret (db != NULL, 0);

  int found = 0;
  if (ol_metadata_get_uri (metadata) != NULL)
  {
    found = _find_by_uri (ol_metadata_get_uri (metadata), lrcpath);
  }
  ol_debugf ("found = %d\n", found);
  if (found == 0 && ol_metadata_get_title (metadata) != NULL)
  {
    found = _find_by_info (metadata, lrcpath);
  }
  return found > 0;
}
