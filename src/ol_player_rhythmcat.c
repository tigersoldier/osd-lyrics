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

#include <dbus/dbus-glib.h>

#include <dbus/dbus-glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ol_player_rhythmcat.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
#include "ol_debug.h"

static const char *SERVICE = "org.supercat.RhythmCat";
static const char *PATH = "/org/supercat/RhythmCat/Shell";
static const char *INTERFACE = "org.supercat.RhythmCat.Shell";
static const char *PLAY = "Play";
static const char *PAUSE = "Pause";
static const char *STOP = "Stop";
static const char *NEXT = "Next";
static const char *PREVIOUS = "Prev";
static const char *SEEK = "SetPosition";
static const char *GET_METADATA = "GetCurrentTrack";
static const char *GET_STATUS = "GetState";
/* static const char *DURATION = "GetDuration"; */
static const char *CURRENT_POSITION = "GetPosition";
static const char *ICON_PATHS[] = {
  "/usr/share/RhythmCat/images/RhythmCat_128x128.PNG",
  "/usr/local/share/RhythmCat/images/RhythmCat_128x128.PNG",
};
static DBusGProxy *proxy = NULL;
static GError *error = NULL;

static gboolean ol_player_rhythmcat_get_music_info (OlMusicInfo *info);
static gboolean ol_player_rhythmcat_get_played_time (int *played_time);
static gboolean ol_player_rhythmcat_get_music_length (int *len);
static gboolean ol_player_rhythmcat_ensure_dbus ();
static gboolean ol_player_rhythmcat_get_activated ();
static enum OlPlayerStatus ol_player_rhythmcat_get_status ();
static int ol_player_rhythmcat_get_capacity ();
static gboolean ol_player_rhythmcat_play ();
static gboolean ol_player_rhythmcat_pause ();
static gboolean ol_player_rhythmcat_stop ();
static gboolean ol_player_rhythmcat_prev ();
static gboolean ol_player_rhythmcat_next ();
static gboolean ol_player_rhythmcat_seek (int pos);
static const char *ol_player_rhythmcat_get_icon_path ();

static gboolean
ol_player_rhythmcat_get_music_info (OlMusicInfo *info)
{
  ol_log_func ();
  ol_assert_ret (info != NULL, FALSE);

  gboolean ret = TRUE;
  GError *error = NULL;

  gchar *artist, *album, *title, *uri;
  guint track_number;

  if (!ol_player_rhythmcat_ensure_dbus ())
    return FALSE;
  if (dbus_g_proxy_call (proxy,
                         GET_METADATA,
                         &error,
                         G_TYPE_INVALID,
                         G_TYPE_STRING,
                         &uri,
                         G_TYPE_STRING,
                         &title,
                         G_TYPE_STRING,
                         &artist,
                         G_TYPE_STRING,
                         &album,
                         G_TYPE_STRING,
                         NULL,
                         G_TYPE_UINT64,
                         NULL,
                         G_TYPE_UINT,
                         &track_number,
                         G_TYPE_UINT,
                         NULL,
                         G_TYPE_UINT,
                         NULL,
                         G_TYPE_UINT,
                         NULL,
                         G_TYPE_INVALID
                         ))
  {
    ol_music_info_clear (info);
    ol_music_info_set_title (info, title);
    ol_music_info_set_artist (info, artist);
    ol_music_info_set_album (info, album);
    ol_music_info_set_uri (info, uri);
    info->track_number = track_number;
  }
  else
  {
    ol_debugf ("%s fail: %s\n", GET_METADATA, error->message);
    g_error_free (error);
    error = NULL;
    ret = FALSE;
  }
  return ret;
}

static gboolean
ol_player_rhythmcat_get_music_length (int *len)
{
  ol_log_func ();
  ol_assert_ret (len != NULL, FALSE);
  if (!ol_player_rhythmcat_ensure_dbus ())
    return FALSE;
  guint64 duration;
  /* The GetDuration call returns wrong value in build 20110821, use the value
     in metadata */
  if (dbus_g_proxy_call (proxy,
                         GET_METADATA,
                         &error,
                         G_TYPE_INVALID,
                         G_TYPE_STRING,
                         NULL,  /* uri */
                         G_TYPE_STRING,
                         NULL,  /* title */
                         G_TYPE_STRING,
                         NULL,  /* artist */
                         G_TYPE_STRING,
                         NULL,  /* album */
                         G_TYPE_STRING,
                         NULL,  /* comment */
                         G_TYPE_UINT64,
                         &duration,
                         G_TYPE_UINT,
                         NULL,  /* track number */
                         G_TYPE_UINT,
                         NULL,
                         G_TYPE_UINT,
                         NULL,
                         G_TYPE_UINT,
                         NULL,
                         G_TYPE_INVALID
                         ))
  {
    *len = duration / 1000000;
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}

static gboolean
ol_player_rhythmcat_get_played_time (int *played_time)
{
  ol_assert_ret (played_time != NULL, FALSE);
  if (!ol_player_rhythmcat_ensure_dbus ())
    return FALSE;
  gint64 ns;
  if (ol_dbus_get_int64 (proxy,
                         CURRENT_POSITION,
                         &ns))
  {
    *played_time = ns / 1000000;
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}

static gboolean
ol_player_rhythmcat_get_activated ()
{
  ol_log_func ();
  return (ol_player_rhythmcat_ensure_dbus ());
}


static gboolean
ol_player_rhythmcat_ensure_dbus ()
{
  if (proxy != NULL)
    return TRUE;
  return ol_dbus_connect (SERVICE,
                          PATH,
                          INTERFACE,
                          G_CALLBACK (ol_dbus_unref_proxy),
                          NULL,
                          &proxy);
}

static enum
OlPlayerStatus ol_player_rhythmcat_get_status ()
{
  if (!ol_player_rhythmcat_ensure_dbus ())
      return OL_PLAYER_ERROR;
  int state_no;
  if (ol_dbus_get_int (proxy,
                       GET_STATUS,
                       &state_no))
  {
    enum OlPlayerStatus status;
    if (state_no == 1 || state_no == 2) /* GST_STATE_NULL, GST_STATE_READY */
      status = OL_PLAYER_STOPPED;
    else if (state_no == 4) /* GST_STATE_PLAYING */
      status = OL_PLAYER_PLAYING;
    else if (state_no == 3) /* GST_STATE_PAUSED */
      status = OL_PLAYER_PAUSED;
    else
      status = OL_PLAYER_UNKNOWN;
    return status;
  }
  else
  {
    return OL_PLAYER_ERROR;
  }
}

static int
ol_player_rhythmcat_get_capacity ()
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    OL_PLAYER_STOP | OL_PLAYER_PREV | OL_PLAYER_NEXT | OL_PLAYER_SEEK;
}

static gboolean
ol_player_rhythmcat_play ()
{
  if (!ol_player_rhythmcat_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy,
                         PLAY);
}

static gboolean
ol_player_rhythmcat_pause ()
{
  if (!ol_player_rhythmcat_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy,
                         PAUSE);
}

static gboolean
ol_player_rhythmcat_stop ()
{
  if (!ol_player_rhythmcat_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy,
                         STOP);
}

static gboolean
ol_player_rhythmcat_prev ()
{
  if (!ol_player_rhythmcat_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy,
                         PREVIOUS);
}

static gboolean
ol_player_rhythmcat_next ()
{
  if (!ol_player_rhythmcat_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy,
                         NEXT);
}

static gboolean
ol_player_rhythmcat_seek (int pos)
{
  if (!ol_player_rhythmcat_ensure_dbus ())
    return FALSE;
  gint64 position = pos;
  position *= 1000000;
  return dbus_g_proxy_call (proxy,
                            SEEK,
                            NULL,
                            G_TYPE_INT64,
                            position,
                            G_TYPE_INVALID,
                            G_TYPE_INVALID);
}

static const char *
ol_player_rhythmcat_get_icon_path ()
{
  int i;
  for (i = 0; i < ol_get_array_len (ICON_PATHS); i++)
  {
    if (ol_path_is_file (ICON_PATHS[i]))
      return ICON_PATHS[i];
  }
  return NULL;
}

struct OlPlayer*
ol_player_rhythmcat_get ()
{
  ol_log_func ();
  struct OlPlayer *controller = ol_player_new ("RhythmCat");
  ol_player_set_cmd (controller, "RhythmCat");
  controller->get_music_info = ol_player_rhythmcat_get_music_info;
  controller->get_activated = ol_player_rhythmcat_get_activated;
  controller->get_played_time = ol_player_rhythmcat_get_played_time;
  controller->get_music_length = ol_player_rhythmcat_get_music_length;
  controller->get_capacity = ol_player_rhythmcat_get_capacity;
  controller->get_status = ol_player_rhythmcat_get_status;
  controller->play = ol_player_rhythmcat_play;
  controller->pause = ol_player_rhythmcat_pause;
  controller->stop = ol_player_rhythmcat_stop;
  controller->prev = ol_player_rhythmcat_prev;
  controller->next = ol_player_rhythmcat_next;
  controller->seek = ol_player_rhythmcat_seek;
  controller->get_icon_path = ol_player_rhythmcat_get_icon_path;
  return controller;
}
