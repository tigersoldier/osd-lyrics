/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009-2011  Tiger Soldier <tigersoldier@gmail.com>
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
#ifndef _OL_LRC_FETCH_MODULE_H_
#define _OL_LRC_FETCH_MODULE_H_

#include <glib.h>
#include "ol_lrc_fetch.h"

struct OlLrcFetchResult
{
  int count;
  int id;
  OlLrcCandidate *candidates;
  OlMusicInfo info;
  OlLrcFetchEngine *engine;
};

struct OlLrcDownloadResult
{
  int id;
  OlMusicInfo *info;
  const char *filepath;
  void *userdata;
};

typedef void (*OlLrcSearchCallback) (struct OlLrcFetchResult *result,
                                     void *userdata);

typedef void (*OlLrcDownloadCallback) (struct OlLrcDownloadResult *result);

struct OlLrcFetchResult* ol_lrc_fetch_result_new ();
void ol_lrc_fetch_result_free (struct OlLrcFetchResult *result);
int ol_lrc_fetch_result_serialize (struct OlLrcFetchResult *result,
                                   char *buffer,
                                   size_t count);

/** 
 * @brief Adds an callback function runs after search finished.
 * The userdata contains the pointer to a OlLrcFetchResult struct.
 * The userdata should not be freed.
 * The call back function will run in the main thread.
 * 
 * @param callbackFunc 
 */
void ol_lrc_fetch_add_async_search_callback (GSourceFunc callbackFunc);

/** 
 * @brief Adds an callback function runs after download finished.
 * The userdata contains the filename with full path of downloaded lrc file if succeed, or NULL if fail.
 * The userdata should be freed with g_free.
 * The call back function will run in the main thread
 * 
 * @param callbackFunc 
 */
void ol_lrc_fetch_add_async_download_callback (OlLrcDownloadCallback callbackFunc);

/** 
 * @brief Begin searching lyrics. Once finished, search callbacks will be invoked.
 * 
 * @param engine The engine to use for searching
 * @param music_info The music info for searching
 * @param callback The callback function for searching done
 * @return a unique id identifies the search result
 */
int ol_lrc_fetch_begin_search (OlLrcFetchEngine *engine, 
                               OlMusicInfo *music_info, 
                               OlLrcSearchCallback callback,
                               void *userdata);

/** 
 * @brief Begin downloading lyrics. Once finished, download callbacks will be invoked.
 * 
 * @param engine The engine to be used.
 * @param candidate The candidate to be downloaded. The function will keep a copy of it.
 * @param pathname The filename with full path of the target lrc file. The function will keep a copy of it
 */
void ol_lrc_fetch_begin_download (OlLrcFetchEngine *engine,
                                  OlLrcCandidate *candidate,
                                  const OlMusicInfo *info,
                                  const char *pathname,
                                  void *userdata);

void ol_lrc_fetch_module_init ();

#endif /* _OL_LRC_FETCH_MODULE_H_ */
