/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009-2011  Tiger Soldier <tigersoldi@gmail.com>
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
#include <sys/stat.h>
#include <glib.h>
#include "ol_lyric_manage.h"
#include "ol_path_pattern.h"
#include "ol_config.h"
#include "ol_utils.h"
#include "ol_debug.h"

static gboolean internal_for_each (OlMusicInfo *info,
                                   OlPathFunc func,
                                   gpointer userdata);

static gboolean
internal_for_each (OlMusicInfo *info,
                   OlPathFunc func,
                   gpointer userdata)
{
  ol_log_func ();
  OlConfig *config = ol_config_get_instance ();
  gsize pathlen, namelen;
  char **path_list = ol_config_get_str_list (config, "General", "lrc-path", &pathlen);
  char **name_list = ol_config_get_str_list (config, "General", "lrc-filename", &namelen);
  if (path_list == NULL || name_list == NULL)
    return FALSE;
  if (path_list == NULL || name_list == NULL ||
      info == NULL || func == NULL)
    return FALSE;
  ol_debugf ("  uri: %s\n", info->uri);
  gboolean ret = ol_path_pattern_for_each (path_list,
                                           name_list,
                                           info,
                                           func,
                                           userdata);
  g_strfreev (path_list);
  g_strfreev (name_list);
  return ret;
}

gboolean
internal_check_lyric_file (const char *filename, gpointer data)
{
  ol_log_func ();
  ol_debugf ("  filename:%s\n", filename);
  if (!ol_path_is_file (filename))
  {
    return FALSE;
  }
  char **ret_val = (char **) data;
  *ret_val = g_strdup (filename);
  return TRUE;
}

gboolean
internal_check_path_exist (const char *filename, gpointer data)
{
  ol_debugf ("%s:%s\n", __FUNCTION__, filename);
  ol_assert_ret (filename != NULL, FALSE);
  ol_assert_ret (data != NULL, FALSE);
  gboolean ret = TRUE;
  gchar * dirname = g_path_get_dirname (filename);
  if (dirname == NULL)
    return FALSE;
  if (g_mkdir_with_parents (dirname, S_IRUSR | S_IWUSR | S_IXUSR |
                             S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
  {
    ol_errorf ("  make directory '%s' failed\n", dirname);
    ret = FALSE;
  }
  else
  {
    char **ret_val = (char **) data;
    *ret_val = g_strdup (filename);
    ret = TRUE;
  }
  g_free (dirname);
  return ret;
}

char *
ol_lyric_find (OlMusicInfo *info)
{
  ol_log_func ();
  char *filename = NULL;
  internal_for_each (info, 
                     internal_check_lyric_file, 
                     (gpointer) &filename);
  return filename;
}

char *
ol_lyric_download_path (OlMusicInfo *info)
{
  ol_log_func ();
  char *filename = NULL;
  internal_for_each (info, 
                     internal_check_path_exist, 
                     (gpointer) &filename);
  return filename;
}
