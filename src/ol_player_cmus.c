/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2012 alepulver <alepulver@gmail.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ol_player_cmus.h"
#include "ol_player.h"
#include "ol_utils_cmdline.h"
#include "ol_elapse_emulator.h"
#include "ol_utils.h"
#include "ol_debug.h"

static gboolean ol_player_cmus_get_music_info (OlMusicInfo *info);
static gboolean ol_player_cmus_get_played_time (int *played_time);
static gboolean ol_player_cmus_get_music_length (int *len);
static gboolean ol_player_cmus_get_activated ();
static enum OlPlayerStatus ol_player_cmus_get_status ();
static int ol_player_cmus_get_capacity ();
static gboolean ol_player_cmus_play ();
static gboolean ol_player_cmus_pause ();
static gboolean ol_player_cmus_stop ();
static gboolean ol_player_cmus_prev ();
static gboolean ol_player_cmus_next ();
static gboolean ol_player_cmus_seek (int pos_ms);
static gboolean ol_player_cmus_get_activated ();

enum CMUS_CfgLabel {
  CSI_STATUS, CSI_URI, CSI_POS, CSI_DURATION,
  CSI_TITLE, CSI_ARTIST, CSI_ALBUM
};

#define CMUS_CfgSize (CSI_ALBUM+1)
static const int CMUS_ELAPSE_ACCURACY = 1000;

// The order must match CMUS_CfgLabel
static const char *CMUS_CfgStrings[] = {
  "status ", "file ", "position ",  "duration ",
  "tag title ", "tag artist ", "tag album "
};

static char *CMUS_CfgValues[CMUS_CfgSize];

static OlElapseEmulator elapse_emulator = {0};

/*
 * There is a single command to obtain song information,
 * so it is ran once and parsed for all variables.
 */
static gboolean
cmus_update_info ()
{
  char *output = NULL;
  int result = ol_cmd_get_string ("cmus-remote -C status", &output);
  char *p_cur, *p_next;
  int i;
  for (i = 0; i < CMUS_CfgSize; i++)
  {
    if (CMUS_CfgValues[i] != NULL)
      g_free (CMUS_CfgValues[i]);
    CMUS_CfgValues[i] = NULL;
  }
  if (output == NULL || !result)
    return FALSE;
  p_cur = output;
  while ((p_next = ol_split_a_line (p_cur)) != NULL)
  {
    int i;
    for (i = 0; i < CMUS_CfgSize; i++)
    {
      const char *key = CMUS_CfgStrings[i];
      const int klen = strlen (key);
      if (klen < strlen (p_cur) && strncmp (p_cur, key, klen) == 0)
      {
        CMUS_CfgValues[i] = g_strdup (p_cur + klen);
        break;
      }
    }
    p_cur = p_next;
  }
  g_free (output);
  return TRUE;
}

static enum OlPlayerStatus cmus_get_status ()
{
  char *stat = CMUS_CfgValues[CSI_STATUS];
  if (stat != NULL)
  {
    if (strcmp (stat, "playing") == 0)
      return OL_PLAYER_PLAYING;
    else if (strcmp (stat, "paused") == 0)
      return OL_PLAYER_PAUSED;
    else if (strcmp (stat, "stopped") == 0)
      return OL_PLAYER_STOPPED;
    else
      return OL_PLAYER_UNKNOWN;
  }
  return OL_PLAYER_ERROR;
}

static gboolean
ol_player_cmus_get_activated ()
{
  return ol_cmd_exec ("cmus-remote");
}

static gboolean
ol_player_cmus_get_music_info (OlMusicInfo *info)
{
  ol_assert_ret (info != NULL, FALSE);
  if (!cmus_update_info ())
    return FALSE;

  int status = cmus_get_status();
  if (status != OL_PLAYER_PLAYING
      && status != OL_PLAYER_PAUSED
      && status != OL_PLAYER_STOPPED)
  {
    ol_music_info_clear (info);
    return TRUE;
  }

  ol_music_info_clear (info);
  info->title = g_strdup (CMUS_CfgValues[CSI_TITLE]);
  info->artist = g_strdup (CMUS_CfgValues[CSI_ARTIST]);
  info->album = g_strdup (CMUS_CfgValues[CSI_ALBUM]);
  info->uri = g_strdup (CMUS_CfgValues[CSI_URI]);

  return TRUE;
}

static gboolean
ol_player_cmus_get_music_length (int *len)
{
  ol_assert_ret (len != NULL, FALSE);
  if (!cmus_update_info ())
    return FALSE;
  int status = cmus_get_status ();
  if (status != OL_PLAYER_PLAYING && status != OL_PLAYER_PAUSED)
  {
    *len = -1;
    return TRUE;
  }
  *len = CMUS_CfgValues[CSI_DURATION] != NULL ?
    500 + strtol (CMUS_CfgValues[CSI_DURATION], NULL, 10) * 1000 : 0;
  return TRUE;
}

static gboolean
ol_player_cmus_get_played_time (int *played_time)
{
  ol_assert_ret (played_time != NULL, FALSE);
  if (!cmus_update_info ())
    return FALSE;
  int status = cmus_get_status ();
  gint64 cmus_time = CMUS_CfgValues[CSI_POS] != NULL ?
    strtol (CMUS_CfgValues[CSI_POS], NULL, 10) * 1000 : 0;
  switch (status)
  {
  case OL_PLAYER_PLAYING:
    *played_time = ol_elapse_emulator_get_real_ms (&elapse_emulator,
                                                   cmus_time);
    break;
  case OL_PLAYER_PAUSED:
    *played_time = ol_elapse_emulator_get_last_ms (&elapse_emulator,
                                                   cmus_time);
    break;
  default:
    ol_elapse_emulator_init (&elapse_emulator, 0, CMUS_ELAPSE_ACCURACY);
    *played_time = 0;
  }
  return TRUE;
}

static enum OlPlayerStatus
ol_player_cmus_get_status ()
{
  if (!cmus_update_info ())
    return OL_PLAYER_ERROR;

  return cmus_get_status ();
}

static int
ol_player_cmus_get_capacity ()
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    OL_PLAYER_STOP | OL_PLAYER_PREV | OL_PLAYER_NEXT | OL_PLAYER_SEEK;
}

static gboolean
ol_player_cmus_play ()
{
  if (!ol_player_cmus_get_activated ())
    return FALSE;
  return ol_cmd_exec ("cmus-remote -C player-play");
}

static gboolean
ol_player_cmus_pause ()
{
  if (!ol_player_cmus_get_activated ())
    return FALSE;
  return ol_cmd_exec ("cmus-remote -C player-pause");
}

static gboolean
ol_player_cmus_stop ()
{
  if (!ol_player_cmus_get_activated ())
    return FALSE;
  return ol_cmd_exec ("cmus-remote -C player-stop");
}

static gboolean
ol_player_cmus_prev ()
{
  if (!ol_player_cmus_get_activated ())
    return FALSE;
  return ol_cmd_exec ("cmus-remote -C player-prev");
}

static gboolean
ol_player_cmus_next ()
{
  if (!ol_player_cmus_get_activated ())
    return FALSE;
  return ol_cmd_exec ("cmus-remote -C player-next");
}

static gboolean
ol_player_cmus_seek (int pos_ms)
{
  if (!ol_player_cmus_get_activated ())
    return FALSE;
  char *cmd = g_strdup_printf ("cmus-remote -C \"seek %d\"", pos_ms / 1000);
  gboolean ret = ol_cmd_exec (cmd);
  g_free (cmd);
  return ret;
}

struct OlPlayer*
ol_player_cmus_get ()
{
  ol_log_func ();
  ol_elapse_emulator_init (&elapse_emulator, 0, CMUS_ELAPSE_ACCURACY);
  struct OlPlayer *controller = ol_player_new ("cmus");
  controller->get_music_info = ol_player_cmus_get_music_info;
  controller->get_activated = ol_player_cmus_get_activated;
  controller->get_played_time = ol_player_cmus_get_played_time;
  controller->get_music_length = ol_player_cmus_get_music_length;
  controller->get_capacity = ol_player_cmus_get_capacity;
  controller->get_status = ol_player_cmus_get_status;
  controller->play = ol_player_cmus_play;
  controller->pause = ol_player_cmus_pause;
  controller->stop = ol_player_cmus_stop;
  controller->prev = ol_player_cmus_prev;
  controller->next = ol_player_cmus_next;
  controller->seek = ol_player_cmus_seek;
  return controller;
}
