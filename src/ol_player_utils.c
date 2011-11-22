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
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include "ol_player_utils.h"
#include "ol_player.h"
#include "ol_app_info.h"
#include "ol_debug.h"

GList *
ol_player_get_app_info_list (struct OlPlayer *player,
                             GList *list)
{
  ol_assert_ret (player != NULL, list);
  const gchar *cmd = ol_player_get_cmd (player);
  if (cmd != NULL)
  {
    GError *error = NULL;
    OlAppInfo *info = ol_app_info_new (cmd,
                                       ol_player_get_name (player),
                                       ol_player_get_icon_path (player),
                                       OL_APP_INFO_PREFER_DESKTOP_FILE,
                                       &error);
    if (!info)
    {
      ol_errorf ("Cannot get player app info: %s\n", error->message);
      g_error_free (error);
    }
    else
    {
      list = g_list_prepend (list, info);
    }
  }
  return list;
}
