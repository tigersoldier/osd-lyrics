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
#ifndef _OL_LRCLIB_H_
#define _OL_LRCLIB_H_

#include "ol_metadata.h"

/** 
 * @brief Initializes the Lrclib module
 * 
 * @param filename The lrclib filename. 
 *                 If not exists, a new one will be created
 * @return Non-zero if succeded. Otherwise returns 0.
 */
int ol_lrclib_init (const char *filename);

/** 
 * @brief Unload the Lrclib module.
 *
 * It will close the database. You should unload it before exit
 * 
 */
void ol_lrclib_unload ();

/** 
 * @brief Assign an LRC file to a music
 * 
 * @param metadata The music info to be assigned
 * @param lrcpath The LRC file, or NULL if no lyric should be assigned
 * 
 * @return Non-zero if succeeded.
 */
int ol_lrclib_assign_lyric (const OlMetadata *metadata, 
                            const char *lrcpath);

/** 
 * @brief Find the lyric assigned to a music
 * 
 * The lyric will be searched according to the file uri, then to the
 * combination of title, artist and album
 * 
 * @param metadata The music info to be assigned
 * @param lrcpath The return loaction to the LRC File. This may be set to 
 *                NULL. If not NULL, it should be freed with 
 * 
 * @return Non-zero if succeeded. 0 if not found or error occured
 */
int ol_lrclib_find (const OlMetadata *metadata,
                    char **lrcpath);
#endif /* _OL_LRCLIB_H_ */
