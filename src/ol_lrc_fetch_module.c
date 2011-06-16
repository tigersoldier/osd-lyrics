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
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "ol_lrc_fetch_module.h"
#include "ol_fork.h"
#include "ol_utils.h"
#include "ol_debug.h"

static int search_id = 0;
static int download_id = 0;
static GPtrArray *search_listeners;
static GPtrArray *download_listeners;

struct SearchData 
{
  struct OlLrcFetchResult *result;
  OlLrcSearchCallback callback;
  void *userdata;
};

static void internal_search (struct OlLrcFetchResult *search_result);
static void internal_run_func (GSourceFunc func,
                               gpointer data);
static void internal_invoke_callback (GPtrArray *func_array,
                                      gpointer userdata);
static void internal_search_callback (void *ret_data,
                                      size_t ret_size,
                                      int status,
                                      void *userdata);

static void
internal_run_func (GSourceFunc func,
                   gpointer data)
{
  ol_log_func ();
  func (data);
}

static void
internal_invoke_callback (GPtrArray *func_array,
                 gpointer userdata)
{
  g_ptr_array_foreach (func_array,
                       (GFunc)internal_run_func,
                       userdata);
}

static void
internal_search (struct OlLrcFetchResult *search_result)
{
  ol_assert (search_result != NULL);
  search_result->candidates = NULL;
  search_result->candidates = search_result->engine->search (&search_result->info,
                                                             &search_result->count,
                                                             "UTF-8");
  /* Output search result */
  fprintf (fret, "%d\n", search_result->count);
  int i;
  for (i = 0; i < search_result->count; i++)
  {
    int size = ol_lrc_candidate_serialize (&search_result->candidates[i], 
                                           NULL, 
                                           0);
    int buf_size = size + 10;
    char *buffer = g_new0 (char, buf_size);
    ol_lrc_candidate_serialize (&search_result->candidates[i], buffer, buf_size);
    fprintf (fret, "%s\n", buffer);
    g_free (buffer);
  }
}

static void
internal_search_callback (void *ret_data,
                          size_t ret_size,
                          int status,
                          void *userdata)
{
  ol_assert (ret_data != NULL);
  ol_assert (userdata != NULL);
  struct SearchData *data = (struct SearchData *) userdata;
  struct OlLrcFetchResult *result = data->result;
  char *current = (char *) ret_data;
  ol_debugf ("returned data:\n%s", current);
  char *count_str = ret_data;
  current = ol_split_a_line (current);
  ol_assert (current != NULL);
  sscanf (count_str, "%d", &result->count);
  int i;
  result->candidates = g_new0 (OlLrcCandidate, result->count);
  for (i = 0; i < result->count; i++)
  {
    current = ol_lrc_candidate_deserialize (&result->candidates[i],
                                            current);
    ol_assert (current != NULL);
    current = ol_split_a_line (current);
    ol_assert (current != NULL);
  }
  if (data->callback)
    data->callback (result, data->userdata);
  internal_invoke_callback (search_listeners, result);
  ol_lrc_fetch_result_free (result);
}

static int
internal_download (OlLrcFetchEngine *engine,
                   OlLrcCandidate *candidate,
                   const char *filepath)
{
  ol_log_func ();
  ol_assert_ret (engine != NULL, 1);
  ol_assert_ret (candidate != NULL, 1);
  ol_assert_ret (filepath != NULL, 1);
  if (engine->download (candidate,
                        filepath,
                        "UTF-8") >= 0)
  {
    ol_debugf ("download %s success\n", filepath);
    fprintf (fret, "%s", filepath);
    return 0;
  }
  else
  {
    return 1;
  }
}

static void
internal_download_callback (void *ret_data,
                            size_t ret_size,
                            int status,
                            void *userdata)
{
  struct OlLrcDownloadResult *result = userdata;
  if (ret_size == 0)
  {
    result->filepath = NULL;
    internal_invoke_callback (download_listeners, result);
  }
  else
  {
    char *filepath = g_new0 (char, ret_size + 1);
    strncpy (filepath, (char *)ret_data, ret_size);
    result->filepath = filepath;
    internal_invoke_callback (download_listeners, result);
  }
}

void
ol_lrc_fetch_add_async_search_callback (GSourceFunc callbackFunc)
{
  g_ptr_array_add (search_listeners, callbackFunc);
}

void
ol_lrc_fetch_add_async_download_callback (OlLrcDownloadCallback callbackFunc)
{
  g_ptr_array_add (download_listeners, callbackFunc);
}

void
ol_lrc_fetch_module_init ()
{
  ol_lrc_fetch_init ();
  search_listeners = g_ptr_array_new ();
  download_listeners = g_ptr_array_new ();
}

int
ol_lrc_fetch_begin_search (OlLrcFetchEngine* _engine, 
                           OlMusicInfo *_music_info,
                           OlLrcSearchCallback callback,
                           void *userdata)
{
  ol_log_func ();
  ol_assert_ret (_engine != NULL, -1);
  ol_assert_ret (_music_info != NULL, -1);
  ol_debugf ("  title: %s\n"
             "  artist: %s\n"
             "  album: %s\n",
             ol_music_info_get_title (_music_info),
             ol_music_info_get_artist (_music_info),
             ol_music_info_get_album (_music_info));
  search_id++;
  struct OlLrcFetchResult *search_result = ol_lrc_fetch_result_new ();
  search_result->engine = _engine;
  ol_music_info_copy (&search_result->info, _music_info);
  search_id++;
  search_result->id = search_id;
  struct SearchData *data = g_new (struct SearchData, 1);
  data->result = search_result;
  data->callback = callback;
  data->userdata = userdata;
  if (ol_fork (internal_search_callback,
               data) == 0)
  {
    internal_search (search_result);
    fflush (fret);
    fclose (fret);
    exit (0);
  }
  return search_id;
}

void
ol_lrc_fetch_begin_download (OlLrcFetchEngine *engine,
                             OlLrcCandidate *candidate,
                             const OlMusicInfo *info,
                             const char *pathname,
                             void *userdata)
{
  ol_log_func ();
  ol_assert (engine != NULL);
  ol_assert (candidate != NULL);
  ol_assert (pathname != NULL);
  ol_debugf ("  pathname: %s\n", pathname);
  struct OlLrcDownloadResult *result = g_new0 (struct OlLrcDownloadResult, 1);
  result->id = ++download_id;
  result->filepath = NULL;
  if (info != NULL)
  {
    result->info = ol_music_info_new ();
    result->userdata = userdata;
    ol_music_info_copy (result->info, info);
  }
  result->userdata = userdata;
  
  if (ol_fork (internal_download_callback, result) == 0)
  {
    int ret = internal_download (engine, candidate, pathname);
    fflush (fret);
    fclose (fret);
    exit (ret);
  }
}

struct OlLrcFetchResult*
ol_lrc_fetch_result_new ()
{
  struct OlLrcFetchResult *ret = g_new0 (struct OlLrcFetchResult, 1);
  ol_music_info_init (&ret->info);
  return ret;
}

void
ol_lrc_fetch_result_free (struct OlLrcFetchResult *result)
{
  g_free (result->candidates);
  ol_music_info_clear (&result->info);
  g_free (result);
}
