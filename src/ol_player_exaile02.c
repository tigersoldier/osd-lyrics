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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ol_player.h"
#include "ol_player_exaile02.h"
#include "ol_utils_dbus.h"
#include "ol_elapse_emulator.h"
#include "ol_debug.h"

static const char service[] = "org.exaile.DBusInterface";
static const char path[] = "/DBusInterfaceObject";
static const char interface[] = "org.exaile.DBusInterface";
static const char play[] = "play_pause";
static const char pause[] = "play_pause";
static const char stop[] = "stop";
static const char next[] = "next_track";
static const char previous[] = "prev_track";
static const char get_title[] = "get_title";
static const char get_artist[] = "get_artist";
static const char get_album[] = "get_album";
static const char get_cover_path[] = "get_cover_path";
static const char get_status[] = "status";
static const char duration[] = "get_length";
static const char current_position[] = "current_position";

static DBusGConnection *connection = NULL;
static DBusGProxy *proxy = NULL;
static GError *error = NULL;
static OlElapseEmulator *elapse_emulator = NULL;

static gboolean ol_player_exaile02_get_music_info (OlMusicInfo *info);
static gboolean ol_player_exaile02_get_played_time (int *played_time);
static gboolean ol_player_exaile02_get_music_length (int *len);
static gboolean ol_player_exaile02_get_activated (void);
static gboolean ol_player_exaile02_init_dbus (void);
static enum OlPlayerStatus ol_player_exaile02_get_status (void);
static int ol_player_exaile02_get_capacity (void);

static enum OlPlayerStatus
ol_player_exaile02_get_status (void)
{
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile02_init_dbus ())
      return FALSE;
  char *status = NULL;
  enum OlPlayerStatus ret = OL_PLAYER_UNKNOWN;
  ol_dbus_get_string (proxy, get_status, &status);
  if (status != NULL)
  {
    if (strcmp (status, "playing") == 0)
      ret = OL_PLAYER_PLAYING;
    else if (strcmp (status, "paused") == 0)
      ret = OL_PLAYER_PAUSED;
    else if (strcmp (status, "stoped") == 0)
      ret = OL_PLAYER_STOPPED;
    g_free (status);
  }
  return ret;
}

static gboolean
ol_player_exaile02_get_music_info (OlMusicInfo *info)
{
  if (info == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile02_init_dbus ())
      return FALSE;
  /* gets the title of current music */
  if (info->title)
  {
    g_free (info->title);
    info->title = NULL;
  }
  if (!ol_dbus_get_string (proxy,
                           get_title,
                           &info->title))
  {
    return FALSE;
  }
  /* gets the artist of current music */
  if (info->artist)
  {
    g_free (info->artist);
    info->artist = NULL;
  }
  if (!ol_dbus_get_string (proxy,
                           get_artist,
                           &info->artist))
  {
    return FALSE;
  }
  /* gets the album of current music */
  if (info->album)
  {
    g_free (info->album);
    info->album = NULL;
  }
  if (!ol_dbus_get_string (proxy,
                           get_album,
                           &info->album))
  {
    return FALSE;
  }
  ol_debugf ("%s\n"
             "  title:%s\n"
             "  artist:%s\n",
             __FUNCTION__,
             info->title,
             info->artist);
  return TRUE;
}

static gboolean
ol_player_exaile02_get_played_time (int *played_time)
{
  guint8 percent;
  int duration;
  int exaile02_time;
  if (played_time == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile02_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_exaile02_get_status ();
  if (status == OL_PLAYER_PLAYING || status == OL_PLAYER_PAUSED)
  {
    if (!ol_dbus_get_uint8 (proxy, current_position, &percent))
      return FALSE;
    if (!ol_player_exaile02_get_music_length (&duration))
      return FALSE;
    exaile02_time = duration * percent / 100;
    if (elapse_emulator == NULL)
    {
      elapse_emulator = g_new (OlElapseEmulator, 1);
      if (elapse_emulator != NULL)
        ol_elapse_emulator_init (elapse_emulator, exaile02_time, duration / 100);
    }
    if (elapse_emulator != NULL)
    {
      if (status == OL_PLAYER_PLAYING)
        *played_time = ol_elapse_emulator_get_real_ms (elapse_emulator, exaile02_time);
      else /* if (status == OL_PLAYER_PAUSED)  */
        *played_time = ol_elapse_emulator_get_last_ms (elapse_emulator, exaile02_time);
    }
    else
      *played_time = exaile02_time;
  }
  else
  {
    *played_time = -1;
  }
  return TRUE;
}

static gboolean
ol_player_exaile02_get_music_length (int *len)
{
  if (len == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile02_init_dbus ())
      return FALSE;
  gchar *buf = NULL;
  if (!ol_dbus_get_string (proxy, duration, &buf))
  {
    return FALSE;
  }
  int min, sec;
  if (sscanf (buf, "%d:%d", &min, &sec) == 2)
  {
    *len = (min * 60 + sec) * 1000;
  }
  g_free (buf);
  return TRUE;
}

static gboolean
ol_player_exaile02_get_activated (void)
{
  ol_log_func ();
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile02_init_dbus ())
      return FALSE;
  gchar buf[1024];
  if (dbus_g_proxy_call (proxy,
                         get_title,
                         NULL,
                         G_TYPE_INVALID,
                         G_TYPE_STRING,
                         buf,
                         G_TYPE_INVALID))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static gboolean
ol_player_exaile02_init_dbus (void)
{
  ol_log_func ();
  if (connection == NULL)
  {
    connection = dbus_g_bus_get (DBUS_BUS_SESSION,
                               &error);
    if (connection == NULL)
    {
      ol_debugf ("get connection failed: %s\n", error->message);
      g_error_free(error);
      error = NULL;
      return FALSE;
    }
    if (proxy != NULL)
      g_object_unref (proxy);
    proxy = NULL;
  }
  if (proxy == NULL)
  {
    proxy = dbus_g_proxy_new_for_name_owner (connection, service, path, interface, &error);
    if (proxy == NULL)
    {
      ol_debugf ("get proxy failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
      return FALSE;
    }
  }
  return TRUE;
}

static int
ol_player_exaile02_get_capacity (void)
{
  return OL_PLAYER_STATUS;
}

struct OlPlayer*
ol_player_exaile02_get (void)
{
  ol_log_func ();
  struct OlPlayer *controller = ol_player_new ("Exaile 0.2");
  ol_player_set_cmd (controller, "exaile");
  controller->get_music_info = ol_player_exaile02_get_music_info;
  controller->get_activated = ol_player_exaile02_get_activated;
  controller->get_played_time = ol_player_exaile02_get_played_time;
  controller->get_music_length = ol_player_exaile02_get_music_length;
  controller->get_capacity = ol_player_exaile02_get_capacity;
  controller->get_status = ol_player_exaile02_get_status;
  return controller;
}
