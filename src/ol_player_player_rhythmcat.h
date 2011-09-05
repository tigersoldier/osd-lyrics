/*
 * Copyright (C) 2009-2011  Mike Ma <zhtx10@gmail.com>
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
#ifndef __OL_PLAYER_RHYTHMCAT_H__
#define __OL_PLAYER_RHYTHMCAT_H__

/* 对不起啊 tiger 兄，我编辑器只能是 4 空格缩进的 */
/** 
 * @brief Creates a controller of RhythmCat
 * 
 * @return The controller of RhythmCat. It's allocated by g_new, so use g_free to free the memory
 */
struct OlPlayer* ol_player_rhythmcat_get ();

#endif /* __OL_PLAYER_RHYTHMCAT_H__ */
