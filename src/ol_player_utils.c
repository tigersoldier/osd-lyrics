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
#include "ol_debug.h"

static char *_get_executable_name (const char *cmd);

GList *
ol_player_get_app_info_list (struct OlPlayer *player,
                             GList *list)
{
  ol_assert_ret (player != NULL, list);
  return ol_player_app_info_list_from_cmdline (list,
                                               ol_player_get_name (player),
                                               ol_player_get_cmd (player));
}

GList *
ol_player_app_info_list_from_cmdline (GList *list,
                                      const char *name,
                                      const char *cmd)
{
  ol_assert_ret (cmd != NULL, list);
  GAppInfo *app_info = NULL;
  gchar *desktop_name = g_strdup_printf ("%s.desktop", cmd);
  gchar *cmd_exe = _get_executable_name (cmd);
  if (name == NULL)
    name = cmd_exe;
  app_info = G_APP_INFO (g_desktop_app_info_new (desktop_name));
  g_free (desktop_name);
  if (app_info == NULL)
  {
    GError *error = NULL;
    app_info = g_app_info_create_from_commandline (cmd, name, 0, &error);
    if (error != NULL)
    {
      ol_debug ("Failed to create app info from command line %s: %s\n",
                error->message);
      g_error_free (error);
    }
  }
  if (app_info != NULL)
    list = g_list_prepend (list, app_info);
  g_free (cmd_exe);
  return list;
}

static char *
_get_executable_name (const char *cmd)
{
  ol_assert_ret (cmd != NULL, NULL);
  char *basename = NULL;
  char *ret = NULL;
  if (cmd[0] == '/')
  {
    basename = g_path_get_basename (cmd);
    cmd = basename;
  }
  const char *start, *end;
  start = cmd;
  while (*start && *start == ' ')
    start++;
  end = start;
  while (*end && *end != ' ')
    end++;
  ret = g_strndup (start, end - start);
  if (basename)
    g_free (basename);
  return ret;
}
