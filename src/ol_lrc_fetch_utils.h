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
/** 
 * @file util_lrc_fetch.h
 * @brief utility functions for fetching the lrc from Internet using libcurl
 * @author simplyzhao
 * @version 0.0.0
 * @date 2009-05-02
 */

#ifndef _UTIL_LRC_FETCH
#define _UTIL_LRC_FETCH

/* libiconv */
/* http://www.gnu.org/software/libiconv */
#include<iconv.h>

/* libcurl */
/* http://curl.haxx.se/libcurl/c */
#include<curl/curl.h>
#include<curl/easy.h>

#include<limits.h>

/** 
 * @brief converts text from one encoding to another using libiconv
 * 
 * @param src text to be converted
 * @param srclen at most #srclen bytes will be read
 * @param dest buffer to store the convert result
 * @param destlen at most #destlen bytes will be written
 * 
 * @return The converted length. In case of error, it sets errno and returns (size_t)(−1). 
 */
size_t convert(const char *from_charset, const char *to_charset, char *src, size_t srclen, char *dest, size_t destlen);


typedef size_t (*WriteCallback)(void *ptr, size_t size, size_t nmemb, void *data);

struct memo {
	char *mem_base;
	size_t mem_len;
};
/** 
 * @brief fetch the content of url into memery
 *
 * @param url target Url
 * @param user_agent Useragent, can be NULL
 * @param dest a bunch of memory identified by the struct memo
 *
 * @return 0 if success, or negative number
 */
int fetch_into_memory(const char *url,
                      const char *refer,
                      const char *user_agent,
                      const char *post_data,
                      size_t post_len,
                      struct memo *dest);

/** 
 * @brief fetch the content of url into file
 * 
 * @param url target Url
 * @param fp file descriptor
 * 
 * @return 0 if success, or negative number
 */
int fetch_into_file(const char *url, const char *refer, FILE *fp);

/** 
 * @brief URL encoding;
 *        especially, 
 *        ' ' will be turned to '+';
 *        "."、"-"、"*"、"_" will keep no change 
 * 
 * @return 0 if success, or negative number
 */
int url_encoding(const char *src, const int srclen, char *dest, int destlen, int space_cat); 

/** 
 * @brief URL decoding
 */
int url_decoding(const char *src, const int srclen, char *dest, int destlen);


/** 
 * @brief URL encoding using libcurl which all input characters that are not a-z, A-Z or 0-9 
 *        are converted to their "URL escaped" version (%NN where NN is a two-digit hexadecimal number)
 * 
 * @return 0 if success, or negative number
 */
int curl_url_encoding(CURL *curl, char *input, char *output, size_t size);

/** 
 * @brief URL decoding using libcurl
 */
int curl_url_decoding(CURL *curl, char *input, char *output, size_t size);

struct _OlLrcCandidate;
struct _OlMusicInfo;
/** 
 * @brief Adds a new candidate into candidate list at the right position
 *
 * @param info The music info used to sort candidate list
 * @param candidate_list The candidate list to be inserted, should be sorted
 * @param count The number of candidates in the list before adding
 * @param size The size of the candidate list
 * @param new_candidate The new candidate to be inserted
 * 
 * @return The number of candidates after adding
 */
int ol_lrc_fetch_add_candidate (const struct _OlMusicInfo *info,
                                struct _OlLrcCandidate *candidate_list,
                                size_t count,
                                size_t size,
                                struct _OlLrcCandidate *new_candidate);

#endif /* _UTIL_LRC_FETCH */ 
