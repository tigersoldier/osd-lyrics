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
#ifndef _LRC_FETCH_H
#define _LRC_FETCH_H
#include "ol_lrc_fetch_utils.h"
#include "ol_metadata.h"

#define OL_URL_LEN_MAX 1024
#define OL_TS_LEN_MAX 100 /* max length for title and singer */
#define BUFSIZE 512 

typedef struct _OlLrcCandidate OlLrcCandidate;
struct _OlLrcCandidate
{
  char title[OL_TS_LEN_MAX];
  char artist[OL_TS_LEN_MAX];
  char url[OL_URL_LEN_MAX];
  int rank;
};

OlLrcCandidate *ol_lrc_candidate_new ();
void ol_lrc_candidate_copy (OlLrcCandidate *dest, OlLrcCandidate *src);
void ol_lrc_candidate_free (OlLrcCandidate *ptr);

void ol_lrc_candidate_set_title (OlLrcCandidate *candidate,
                                 const char *title);
char* ol_lrc_candidate_get_title (OlLrcCandidate *candidate);

void ol_lrc_candidate_set_artist (OlLrcCandidate *candidate,
                                  const char *artist);
char* ol_lrc_candidate_get_artist (OlLrcCandidate *candidate);

void ol_lrc_candidate_set_url (OlLrcCandidate *candidate,
                               const char *url);
char* ol_lrc_candidate_get_url (OlLrcCandidate *candidate);

void ol_lrc_candidate_set_rank (OlLrcCandidate *candidate,
                                int rank);
int ol_lrc_candidate_get_rank (OlLrcCandidate *candidate);

/** 
 * @brief Converts an LRC Candidate to a string
 * The returned buffer is NUL-terminated
 * @param candidate An OlLrcCandidate
 * @param buffer Buffer of serialzed string, or NULL.
 *               If not NULL, the serialzed string is terminated with NUL.
 * @param count The size of the buffer. 
 * 
 * @return The length of the serialized string, regardless of the size of buffer.
 */
int ol_lrc_candidate_serialize (OlLrcCandidate *candidate,
                                char *buffer,
                                size_t count);

/** 
 * @brief Converts a string to an OlLrcCandidate
 * The serialized part of the string will be modified.
 * @param candidate An OlLrcCandidate
 * @param data The serialized string from an OlLrcCandidate
 * 
 * @return Pointer to the remaining data, or NULL if failed
 */
char * ol_lrc_candidate_deserialize (OlLrcCandidate *candidate,
                                     char *data);

/** 
 * @brief fetch the candidate title-singer-url list;
 *        strongly depending on the web page structure.
 */
typedef OlLrcCandidate *(*Lrc_Search) (const OlMetadata *metadata,
                                       int *size,
                                       const char *local_charset);

/** 
 * @brief download the lrc and store it in the file system
 */
typedef int (*Lrc_Download) (OlLrcCandidate *candidates,
                             const char *pathname,
                             const char *charset);

typedef struct _OlLrcFetchEngine OlLrcFetchEngine;
struct _OlLrcFetchEngine
{
  char *name;
  Lrc_Search search;
  Lrc_Download download;
};

const char *ol_lrc_fetch_engine_get_name (OlLrcFetchEngine *engine);

/** 
 * @brief Get LRC Fetch engine with given name.
 * The engine should NOT be freed.
 * @param name The name of the engine
 * 
 * @return The engine with the given name. If not exists, return default engine.
 */
OlLrcFetchEngine *ol_lrc_fetch_get_engine (const char *name);

/** 
 * @brief Initialize LRC fetch module
 * 
 */
void ol_lrc_fetch_init ();

const char** ol_lrc_fetch_get_engine_list (int *count);

#endif /* _LRC_FETCH_H */ 
