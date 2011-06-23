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
#ifndef _OL_LRC_ENGINE_LIST_H_
#define _OL_LRC_ENGINE_LIST_H_

#include <gtk/gtk.h>
#include "ol_lrc_fetch.h"

void ol_lrc_engine_list_init (GtkTreeView *list);

/** 
 * Sets enabled engines with a list of name of engines.
 *
 * The enabled engines will be moved above disabled engines and sorted
 * according their position in the engine_list
 * @param list 
 * @param engine_list A NULL-terminated string list with names of engines.
 *                    The caller is responsible for freeing it.
 */
void ol_lrc_engine_list_set_engine_names (GtkTreeView *list, 
                                          char **engine_list);
/** 
 * Gets a list of name of enabled engines
 *
 * The returned list will be sorted according to their position in
 * the widget
 * 
 * @param list The engine list widget
 * 
 * @return A NULL-terminated string list with names of engines.
 * Should be freed with g_strfreev
 */
char **ol_lrc_engine_list_get_engine_names (GtkTreeView *list);

#endif /* _OL_LRC_ENGINE_LIST_H_ */
