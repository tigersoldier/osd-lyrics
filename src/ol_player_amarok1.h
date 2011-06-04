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
 * @file   ol_player_amarok1.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Mon May 18 13:55:29 2009
 * 
 * @brief  Provides support for Amarok1.4
 * 
 * 
 */
#ifndef _OL_PLAYER_AMAROK1_H_
#define _OL_PLAYER_AMAROK1_H_

#include "ol_player.h"

/** 
 * @brief Creates a controller of AmarOK1.4
 * 
 * @return The controller of AmarOK1.4. It's allocated by g_new, so use g_free to free the memory
 */
struct OlPlayer* ol_player_amarok1_get ();


#endif /* _OL_PLAYER_AMAROK1_H_ */
