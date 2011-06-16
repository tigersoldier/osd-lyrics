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
#ifndef _OL_LRC_PARSER_H_
#define _OL_LRC_PARSER_H_

/** 
 * @brief LRC Parser struct
 * 
 * To user the struct, you need to create an instance with
 * ol_lrc_parser_new or ol_lrc_parser_new_from file, and frees with
 * ol_lrc_parser_free
 */
struct OlLrcParser;

/** 
 * @brief Types of OlLrcToken
 * 
 * 
 */
enum OlLrcTokenType {
  OL_LRC_TOKEN_TEXT = 0,        /**< Normal text token */
  OL_LRC_TOKEN_ATTR,            /**< Attribute tag token */
  OL_LRC_TOKEN_TIME,            /**< Time tag token */
  OL_LRC_TOKEN_INVALID,         /**< Invalid token */
};

struct OlLrcTextToken
{
  enum OlLrcTokenType type;
  char *text;
};

struct OlLrcAttrToken
{
  enum OlLrcTokenType type;
  char *attr;
  char *value;
};

struct OlLrcTimeToken
{
  enum OlLrcTokenType type;
  int time;
};

/** 
 * @brief Token parsed from LRC files
 *
 * You can use the type member to get the type of the token
 */
union OlLrcToken
{
  enum OlLrcTokenType type;
  struct OlLrcTextToken text;
  struct OlLrcAttrToken attr;
  struct OlLrcTimeToken time;
};

/** 
 * @brief Creates a new LRC parser instance
 * 
 * You need to call ol_lrc_parser_set_buffer to set the contents
 * @return A new OlLrcParser instance. You should use ol_lrc_parser_free to free it
 */
struct OlLrcParser *ol_lrc_parser_new ();

/** 
 * @brief Create a new LRC parser and loads a file as its buffer
 * 
 * @param filename The file name of the LRC file to be loaded
 * 
 * @return A new OlLrcParser instance, or NULL if the file fail to load.
 *         You should use ol_lrc_parser_free to free it
 */
struct OlLrcParser *ol_lrc_parser_new_from_file (const char *filename);

/** 
 * @brief Set the buffer of the parser.
 *
 * The parser will be reset to parse from the beginning.
 * The filename of the parser will be set to NULL
 *
 * @param parser The LRC parser
 * @param buffer The content of the lyric
 */
void ol_lrc_parser_set_buffer (struct OlLrcParser *parser,
                               const char *buffer);

/** 
 * @brief Reset the current location of the parser
 *
 * The parser will parse from the begging of its buffer after reset
 * @param parser The parser to reset
 */
void ol_lrc_parser_reset (struct OlLrcParser *parser);

/** 
 * @brief Get the token from the current position of parser
 *
 * You can use ol_lrc_token_get_type to get the type of the token
 * @param parser An LRC parser
 * 
 * @return The next token return. If there is no more tokens, return NULL.
 */
union OlLrcToken *ol_lrc_parser_next_token (struct OlLrcParser *parser);

/** 
 * @brief Frees a parser
 *
 * All the buffers from the parser is freed
 * @param parser 
 */
void ol_lrc_parser_free (struct OlLrcParser *parser);

/** 
 * @brief Gets the type of a token
 *
 * @param token 
 * 
 * @return The type of token. If the token is NULL or invalid, return OL_LRC_TOKEN_INVALID
 */
enum OlLrcTokenType ol_lrc_token_get_type (union OlLrcToken *token);

/** 
 * @brief Frees the token, include its strings
 * 
 * @param token 
 */
void ol_lrc_token_free (union OlLrcToken *token);

/** 
 * @brief Get the filename of the LRC file the parser uses
 * 
 * @param parser 
 * 
 * @return The full filename, may be NULL
 */
const char *ol_lrc_parser_get_filename (struct OlLrcParser *parser);

#endif /* _OL_LRC_PARSER_H_ */
