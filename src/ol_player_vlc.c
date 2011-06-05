/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2010  Sarlmol Apple <sarlmolapple@gmail.com>
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
#include <dbus/dbus-glib.h>
#include "ol_player_vlc.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
#include "ol_player_mpris.h"
#include "ol_elapse_emulator.h"
#include "ol_debug.h"

static const char *SERVICE = "org.mpris.vlc";
static const char *icon_paths[] = {\
    "/usr/share/icons/hicolor/128x128/apps/vlc.png",
    "/usr/share/icons/hicolor/16x16/apps/vlc.png",
    "/usr/share/icons/hicolor/256x256/apps/vlc.png",
    "/usr/share/icons/hicolor/32x32/apps/vlc.png",
    "/usr/share/icons/hicolor/48x48/apps/vlc.png"
};

static OlPlayerMpris *mpris = NULL;
static OlElapseEmulator *elapse_emulator = NULL;

static OlPlayerMpris* ol_player_vlc_get_mpris ();

static gboolean ol_player_vlc_get_music_info (OlMusicInfo *info);
static gboolean ol_player_vlc_get_played_time (int *played_time);
static gboolean ol_player_vlc_get_music_length (int *len);
static gboolean ol_player_vlc_get_activated ();
static enum OlPlayerStatus ol_player_vlc_get_status ();
static int ol_player_vlc_get_capacity ();
static gboolean ol_player_vlc_play ();
static gboolean ol_player_vlc_pause ();
static gboolean ol_player_vlc_stop ();
static gboolean ol_player_vlc_prev ();
static gboolean ol_player_vlc_next ();
static gboolean ol_player_vlc_seek (int pos_ms);
static void ol_player_vlc_ensure_elapse (int elapsed_time);
static const char *ol_player_vlc_get_icon_path ();

static void
ol_player_vlc_ensure_elapse (int elapsed_time)
{
  if (elapse_emulator == NULL)
  {
    elapse_emulator = g_new (OlElapseEmulator, 1);
    if (elapse_emulator != NULL)
      ol_elapse_emulator_init (elapse_emulator, elapsed_time, 1000);
  }
}

static OlPlayerMpris*
ol_player_vlc_get_mpris ()
{
  if (mpris == NULL)
  {
    mpris = ol_player_mpris_new (SERVICE);
  }
  return mpris;
}

static gboolean
ol_player_vlc_get_music_info (OlMusicInfo *info)
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_get_music_info (mpris, info);
}

static gboolean
ol_player_vlc_get_played_time (int *played_time)
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  int vlc_time = 0;
  if (!ol_player_mpris_get_played_time (mpris, &vlc_time))
    return FALSE;
  ol_player_vlc_ensure_elapse (vlc_time);

  enum OlPlayerStatus status = ol_player_vlc_get_status ();
  if (status == OL_PLAYER_PLAYING)
      *played_time = ol_elapse_emulator_get_real_ms (elapse_emulator, vlc_time);
  else
      *played_time = -1;
  return TRUE;
}

static gboolean
ol_player_vlc_get_music_length (int *len)
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_get_music_length (mpris, len);
}

static gboolean
ol_player_vlc_get_activated ()
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_get_activated (mpris);
}

static enum OlPlayerStatus
ol_player_vlc_get_status ()
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_get_status (mpris);
}

static int
ol_player_vlc_get_capacity ()
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_get_capacity (mpris);
}

static gboolean
ol_player_vlc_play ()
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_play (mpris);
}

static gboolean
ol_player_vlc_pause ()
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_pause (mpris);
}

static gboolean
ol_player_vlc_stop ()
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_stop (mpris);
}

static gboolean
ol_player_vlc_prev ()
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_prev (mpris);
}

static gboolean
ol_player_vlc_next ()
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_next (mpris);
}

static gboolean
ol_player_vlc_seek (int pos_ms)
{
  OlPlayerMpris *mpris = ol_player_vlc_get_mpris ();
  return ol_player_mpris_seek (mpris, pos_ms);
}

static const char *
ol_player_vlc_get_icon_path ()
{
  int i;
  for (i = 0; i < ol_get_array_len (icon_paths); i++)
  {
    if (ol_path_is_file (icon_paths[i]))
      return icon_paths[i];
  }
  return NULL;
}

struct OlPlayer*
ol_player_vlc_get ()
{
  ol_log_func ();
  struct OlPlayer *controller = ol_player_new ("vlc");
  ol_player_set_cmd (controller, "vlc --control dbus");
  controller->get_music_info = ol_player_vlc_get_music_info;
  controller->get_activated = ol_player_vlc_get_activated;
  controller->get_played_time = ol_player_vlc_get_played_time;
  controller->get_music_length = ol_player_vlc_get_music_length;
  controller->get_capacity = ol_player_vlc_get_capacity;
  controller->get_status = ol_player_vlc_get_status;
  controller->play = ol_player_vlc_play;
  controller->pause = ol_player_vlc_pause;
  controller->stop = ol_player_vlc_stop;
  controller->prev = ol_player_vlc_prev;
  controller->next = ol_player_vlc_next;
  controller->seek = ol_player_vlc_seek;
  controller->get_icon_path = ol_player_vlc_get_icon_path;
  return controller;
}

