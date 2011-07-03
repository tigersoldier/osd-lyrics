/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2011  Tiger Soldier
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
#ifndef _OL_PLAYER_UTILS_H_
#define _OL_PLAYER_UTILS_H_

#include <glib.h>

struct OlPlayer;

/** 
 * Prepend an GAppInfo to list according to the player commandline and name
 * 
 * @param list The original list of pointer to GAppInfo
 * 
 * @return The new list of pointer to GAppInfo
 */
GList *ol_player_get_app_info_list (struct OlPlayer *player,
                                    GList *list);

/** 
 * Prepend an GAppInfo to list according to name and cmd
 * 
 * This will try to get from .desktop file first, then from the command line
 *
 * @param list The original list of pointer to GAppInfo
 * @param cmd The command line to append
 * 
 * @return The new list of pointer to GAppInfo
 */
GList *ol_player_app_info_list_from_cmdline (GList *list,
                                             const char *name,
                                             const char *cmd);

#endif /* _OL_PLAYER_UTILS_H_ */
