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
static gboolean initialized = FALSE;
static GPtrArray *search_listeners;
static GPtrArray *download_listeners;
static GHashTable *search_contexts = NULL;

struct SearchData 
{
  int id;
  guint child_watch;
  guint delay_timer;
  GList *engines;
  OlMetadata *metadata;
  OlLrcSearchMsgCallback msg_callback;
  OlLrcSearchCallback result_callback;
  void *userdata;
};

static struct SearchData *_search_data_new (int id,
                                            GList *engine_list,
                                            OlMetadata *metadata,
                                            OlLrcSearchMsgCallback msg_callback,
                                            OlLrcSearchCallback result_callback,
                                            void *userdata);
static void _search_data_free (struct SearchData *search_data);
                              
static void _begin_search (struct SearchData *search_data);
static gboolean _begin_search_delay_cb (struct SearchData *search_data);
static void internal_search (struct SearchData *search_data);
static void internal_run_func (GSourceFunc func,
                               gpointer data);
static void internal_invoke_callback (GPtrArray *func_array,
                                      gpointer userdata);
static void internal_search_callback (void *ret_data,
                                      size_t ret_size,
                                      int status,
                                      void *userdata);
static void _remove_search_data (int search_id);
static GList *_get_engine_list_from_strv (char **engine_list, int count);

static struct SearchData *
_search_data_new (int id,
                  GList *engines,
                  OlMetadata *metadata,
                  OlLrcSearchMsgCallback msg_callback,
                  OlLrcSearchCallback result_callback,
                  void *userdata)
{
  struct SearchData *data = g_new (struct SearchData, 1);
  data->id = id;
  data->engines = engines;
  data->metadata = ol_metadata_new ();
  ol_metadata_copy (data->metadata, metadata);
  data->msg_callback = msg_callback;
  data->result_callback = result_callback;
  data->delay_timer = 0;
  data->userdata = userdata;
  data->child_watch = 0;
  return data;
}

static void
_search_data_free (struct SearchData *search_data)
{
  if (search_data->delay_timer > 0)
    g_source_remove (search_data->delay_timer);
  if (search_data->engines != NULL)
    g_list_free (search_data->engines);
  if (search_data->child_watch > 0)
    g_source_remove (search_data->child_watch);
  ol_metadata_free (search_data->metadata);
  g_free (search_data);
}

static void
_remove_search_data (int search_id)
{
  g_hash_table_remove (search_contexts, &search_id);
}

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
internal_search (struct SearchData *search_data)
{
  ol_assert (search_data != NULL);
  ol_assert (search_data->engines != NULL);
  int count = 0;
  OlLrcCandidate *candidates = NULL;
  OlLrcFetchEngine *engine = search_data->engines->data;
  candidates = engine->search (search_data->metadata,
                               &count,
                               "UTF-8");
  /* Output search result */
  fprintf (fret, "%d\n", count);
  int i;
  for (i = 0; i < count; i++)
  {
    int size = ol_lrc_candidate_serialize (&candidates[i], 
                                           NULL, 
                                           0);
    int buf_size = size + 10;
    char *buffer = g_new0 (char, buf_size);
    ol_lrc_candidate_serialize (&candidates[i], buffer, buf_size);
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
  struct SearchData *search_data = (struct SearchData *) userdata;
  OlLrcFetchEngine *engine = search_data->engines->data;
  int count = 0;
  search_data->engines = g_list_delete_link (search_data->engines,
                                             search_data->engines);
  search_data->child_watch = 0;
  char *current = (char *) ret_data;
  ol_debugf ("returned data:\n%s", current);
  char *count_str = ret_data;
  sscanf (count_str, "%d", &count);
  
  if (count == 0 && search_data->engines != NULL)
  {
    _begin_search (search_data);
  }
  else
  {
    struct OlLrcFetchResult *result = ol_lrc_fetch_result_new ();
    current = ol_split_a_line (current);
    ol_assert (current != NULL);
    result->engine = engine;
    result->count = count;
    ol_metadata_copy (&result->metadata, search_data->metadata);
    result->id = search_data->id;
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
    if (search_data->result_callback)
    {
      search_data->result_callback (result, search_data->userdata);
    }
    internal_invoke_callback (search_listeners, result);
    _remove_search_data (search_data->id);
    ol_lrc_fetch_result_free (result);
  }
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
  g_free (result);
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
  if (initialized)
  {
    ol_error ("Lrc fetch module has been initialized");
    return;
  }
  initialized = TRUE;
  ol_lrc_fetch_init ();
  search_listeners = g_ptr_array_new ();
  download_listeners = g_ptr_array_new ();
  search_contexts = g_hash_table_new_full (g_int_hash,
                                           g_int_equal,
                                           g_free,
                                           (GDestroyNotify) _search_data_free);
}

void
ol_lrc_fetch_module_unload ()
{
  if (!initialized)
  {
    ol_error ("Lrc fetch module is not initialized");
    return;
  }
  initialized = FALSE;
  g_hash_table_destroy (search_contexts);
  g_ptr_array_unref (search_listeners);
  g_ptr_array_unref (download_listeners);
}

static void
_begin_search (struct SearchData *search_data)
{
  ol_assert (search_data != NULL);
  ol_assert (search_data->engines != NULL);
  if (search_data->msg_callback)
  {
    OlLrcFetchEngine *engine = (OlLrcFetchEngine*)(search_data->engines->data);
    search_data->msg_callback (search_data->id,
                               OL_LRC_SEARCH_MSG_ENGINE,
                               ol_lrc_fetch_engine_get_name (engine),
                               search_data->userdata);
  }
  if (ol_fork (internal_search_callback,
               search_data, &search_data->child_watch) == 0)
  {
    internal_search (search_data);
    fflush (fret);
    fclose (fret);
    exit (0);
  }
}

static gboolean
_begin_search_delay_cb (struct SearchData *search_data)
{
  search_data->delay_timer = 0;
  _begin_search (search_data);
  return FALSE;
}

static GList *
_get_engine_list_from_strv (char **engine_list, int count)
{
  ol_assert_ret (engine_list != NULL, NULL);
  GList *ret = NULL;
  int real_count = count;
  if (real_count < 0)
    real_count = g_strv_length (engine_list);
  int i;
  for (i = real_count - 1; i >= 0; i--)
  {
    OlLrcFetchEngine *engine = ol_lrc_fetch_get_engine (engine_list[i]);
    if (engine)
      ret = g_list_prepend (ret, engine);
  }
  if (ret == NULL && count < 0)
  {
    char **default_engines = (char**)ol_lrc_fetch_get_engine_list (&count);
    return _get_engine_list_from_strv (default_engines, count);
  }
  else
  {
    return ret;
  }
}

int
ol_lrc_fetch_begin_search (char **engine_list,
                           OlMetadata *metadata,
                           OlLrcSearchMsgCallback msg_callback,
                           OlLrcSearchCallback result_callback,
                           void *userdata)
{
  ol_log_func ();
  ol_assert_ret (engine_list != NULL, -1);
  ol_assert_ret (metadata != NULL, -1);
  ol_debugf ("  title: %s\n"
             "  artist: %s\n"
             "  album: %s\n",
             ol_metadata_get_title (metadata),
             ol_metadata_get_artist (metadata),
             ol_metadata_get_album (metadata));
  GList *engines = _get_engine_list_from_strv (engine_list, -1);
  if (engines == NULL)
  {
    ol_error ("Invoke searching with empty engine list");
    return -1;
  }
  search_id++;
  struct SearchData *data = _search_data_new (search_id,
                                              engines,
                                              metadata,
                                              msg_callback,
                                              result_callback,
                                              userdata);
  /* We need to delay the first search procedure so that
     the msg_callback will be called after this function returned */
  data->delay_timer = g_timeout_add (0,
                                     (GSourceFunc)_begin_search_delay_cb,
                                     data);
  gint *p_search_id = g_new (gint, 1);
  *p_search_id = search_id;
  g_hash_table_insert (search_contexts,
                       p_search_id,
                       data);
  return search_id;
}

void ol_lrc_fetch_cancel_search (int search_id)
{
  _remove_search_data (search_id);
}

void
ol_lrc_fetch_begin_download (OlLrcFetchEngine *engine,
                             OlLrcCandidate *candidate,
                             const OlMetadata *metadata,
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
  if (metadata != NULL)
  {
    result->metadata = ol_metadata_new ();
    result->userdata = userdata;
    ol_metadata_copy (result->metadata, metadata);
  }
  result->userdata = userdata;
  
  if (ol_fork (internal_download_callback, result, NULL) == 0)
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
  ol_metadata_init (&ret->metadata);
  return ret;
}

void
ol_lrc_fetch_result_free (struct OlLrcFetchResult *result)
{
  g_free (result->candidates);
  ol_metadata_clear (&result->metadata);
  g_free (result);
}
