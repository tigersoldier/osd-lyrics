/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2011  Tiger Soldier <tigersoldier@gmail.com>
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
#include <string.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include "ol_player_mpris2.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
#include "ol_elapse_emulator.h"
#include "ol_debug.h"

static const char *PATH = "/org/mpris/MediaPlayer2";
/* static const char *IFACE = "org.mpris.MediaPlayer2"; */
static const char *NAME_PREFIX = "org.mpris.MediaPlayer2";
static const char *PLAYER_INTERFACE = "org.mpris.MediaPlayer2.Player";
static const char *PLAY_METHOD = "Play";
static const char *PAUSE_METHOD = "Pause";
static const char *STOP_METHOD = "Stop";
static const char *NEXT_METHOD = "Next";
static const char *PREVIOUS_METHOD = "Previous";
static const char *SEEK_METHOD = "Seek";
static const char *METADATA_PROP = "Metadata";
static const char *STATUS_PROP = "PlaybackStatus";
static const char *POSITION_PROP = "Position";
static const char *CANPLAY_PROP = "CanPlay";
static const char *CANCONTROL_PROP = "CanControl";
static const char *CANSEEK_PROP = "CanSeek";
static const char *CANPAUSE_PROP = "CanPause";
static const char *CANPREV_PROP = "CanGoPrevious";
static const char *CANNEXT_PROP = "CanGoNext";
static const char *LENGTH_KEY = "mpris:length";
static const char *TITLE_KEY = "xesam:title";
static const char *ARTIST_KEY = "xesam:artist";
static const char *ALBUM_KEY = "xesam:album";
static const char *URI_KEY = "xesam:url";
static const char *TRACKNUM_KEY = "xesam:trackNumber";
static const char *PLAYING_STATUS = "Playing";
static const char *PAUSED_STATUS = "Paused";
static const char *STOPPED_STATUS = "Stopped";

static struct Mpris2
{
  DBusGProxy *proxy;
  OlElapseEmulator elapse;
} mpris2 = {0};

static gboolean _get_activated ();
static gboolean _get_played_time (int *played_time);
static gboolean _get_music_length (int *len);
static gboolean _get_music_info (OlMusicInfo *info);
static int _get_capacity ();
static gboolean _play ();
static gboolean _pause ();
static gboolean _stop ();
static gboolean _prev ();
static gboolean _next ();
static enum OlPlayerStatus _get_status ();
static gboolean _seek (int pos_ms);
static gboolean _ensure_proxy ();
static gboolean _try_dbus_name (const char *name);
static gboolean _proxy_destroy_cb (DBusGProxy *proxy, struct Mpris2 *mpris2);
static GHashTable *_get_metadata ();

static gboolean
_proxy_destroy_cb (DBusGProxy *proxy, struct Mpris2 *mpris2)
{
  g_object_unref (proxy);
  mpris2->proxy = NULL;
  return FALSE;
}

static gboolean
_try_dbus_name (const char *name)
{
  ol_debugf ("Trying MPRIS2 %s\n", name);
  DBusGConnection *connection = ol_dbus_get_connection ();
  if (connection == NULL)
    return FALSE;
  mpris2.proxy = dbus_g_proxy_new_for_name (connection,
                                            name,
                                            PATH,
                                            PLAYER_INTERFACE);
  if (mpris2.proxy == NULL)
    return FALSE;
  g_signal_connect (mpris2.proxy,
                    "destroy",
                    G_CALLBACK (_proxy_destroy_cb),
                    (gpointer) &mpris2);
  ol_elapse_emulator_init (&mpris2.elapse, 0, 1000);
  return TRUE;
}

static gboolean
_ensure_proxy ()
{
  if (mpris2.proxy != NULL)
    return TRUE;
  char **names = ol_dbus_list_names ();
  if (names == NULL)
    return FALSE;
  char **p;
  gboolean found = FALSE;
  for (p = names; *p != NULL; p++)
  {
    if (g_str_has_prefix (*p, NAME_PREFIX) &&
        _try_dbus_name (*p))
    {
      found = TRUE;
      break;
    }
  }
  g_strfreev (names);
  return found;
}

static GHashTable*
_get_metadata ()
{
  if (!_ensure_proxy ())
    return NULL;
  GHashTable *ret = NULL;
  if (!ol_dbus_get_dict_property (mpris2.proxy, METADATA_PROP, &ret))
    return NULL;
  return ret;
}

static gboolean
_get_activated ()
{
  return _ensure_proxy ();
}

static gboolean
_get_played_time (int *played_time)
{
  ol_log_func ();
  ol_assert_ret (played_time != NULL, FALSE);
  if (!_ensure_proxy ())
    return FALSE;
  gint64 position = 0;
  if (!ol_dbus_get_int64_property (mpris2.proxy,
                                   POSITION_PROP,
                                   &position))
    return FALSE;
  if (_get_status () == OL_PLAYER_PLAYING)
    *played_time = ol_elapse_emulator_get_real_ms (&mpris2.elapse,
                                                   position / 1000);
  else
    *played_time = ol_elapse_emulator_get_last_ms (&mpris2.elapse,
                                                   position / 1000);
  return TRUE;
}

static gboolean
_get_music_length (int *len)
{
  ol_log_func ();
  ol_assert_ret (len != NULL, FALSE);
  GHashTable *metadata = _get_metadata ();
  if (metadata == NULL)
    return FALSE;
  *len = (int)ol_get_int64_from_hash_table (metadata, LENGTH_KEY) / 1000;
  g_hash_table_unref (metadata);
  return TRUE;
}

static gboolean
_get_music_info (OlMusicInfo *info)
{
  ol_log_func ();
  ol_assert_ret (info != NULL, FALSE);
  GHashTable *metadata = _get_metadata ();
  if (metadata == NULL)
    return FALSE;
  ol_music_info_clear (info);
  ol_music_info_set_title (info,
                           ol_get_string_from_hash_table (metadata,
                                                          TITLE_KEY));
  char **artist_list = ol_get_str_list_from_hash_table (metadata,
                                                        ARTIST_KEY);
  if (artist_list != NULL)
  {
    char *artist = NULL;
    if (g_strv_length (artist_list) > 0)
      artist = g_strjoinv (",", artist_list);
    ol_music_info_set_artist (info, artist);
    g_free (artist);
  }
                   
  ol_music_info_set_album (info,
                           ol_get_string_from_hash_table (metadata,
                                                          ALBUM_KEY));
  ol_music_info_set_uri (info,
                         ol_get_string_from_hash_table (metadata,
                                                        URI_KEY));
  ol_music_info_set_track_number (info,
                                  ol_get_int_from_hash_table (metadata,
                                                              TRACKNUM_KEY));
  g_hash_table_unref (metadata);
  return TRUE;
}

static int
_get_capacity ()
{
  if (!_ensure_proxy ())
    return 0;
  int caps = OL_PLAYER_STATUS;
  gboolean prop = FALSE;
  if (ol_dbus_get_bool_property (mpris2.proxy, CANPLAY_PROP, &prop) && prop)
    caps |= OL_PLAYER_PLAY;
  if (ol_dbus_get_bool_property (mpris2.proxy, CANPAUSE_PROP, &prop) && prop)
    caps |= OL_PLAYER_PAUSE;
  if (ol_dbus_get_bool_property (mpris2.proxy, CANPREV_PROP, &prop) && prop)
    caps |= OL_PLAYER_PREV;
  if (ol_dbus_get_bool_property (mpris2.proxy, CANNEXT_PROP, &prop) && prop)
    caps |= OL_PLAYER_NEXT;
  if (ol_dbus_get_bool_property (mpris2.proxy, CANSEEK_PROP, &prop) && prop)
    caps |= OL_PLAYER_SEEK;
  if (ol_dbus_get_bool_property (mpris2.proxy, CANCONTROL_PROP, &prop) && prop)
    caps |= OL_PLAYER_STOP;
  return caps;
}

static gboolean
_play ()
{
  if (!_ensure_proxy ())
    return 0;
  return ol_dbus_invoke (mpris2.proxy, PLAY_METHOD);
}

static gboolean
_pause ()
{
  if (!_ensure_proxy ())
    return 0;
  return ol_dbus_invoke (mpris2.proxy, PAUSE_METHOD);
}

static gboolean
_stop ()
{
  if (!_ensure_proxy ())
    return 0;
  return ol_dbus_invoke (mpris2.proxy, STOP_METHOD);
}

static gboolean
_prev ()
{
  if (!_ensure_proxy ())
    return 0;
  return ol_dbus_invoke (mpris2.proxy, PREVIOUS_METHOD);
}

static gboolean
_next ()
{
  if (!_ensure_proxy ())
    return 0;
  return ol_dbus_invoke (mpris2.proxy, NEXT_METHOD);
}

static enum OlPlayerStatus
_get_status ()
{
  if (!_ensure_proxy ())
    return OL_PLAYER_ERROR;
  enum OlPlayerStatus status = OL_PLAYER_UNKNOWN;
  char *prop = NULL;
  if (ol_dbus_get_string_property (mpris2.proxy, STATUS_PROP, &prop))
  {
    ol_debugf ("MPRIS2 status: %s\n", prop);
    if (strcmp (prop, PLAYING_STATUS) == 0)
      status = OL_PLAYER_PLAYING;
    else if (strcmp (prop, PAUSED_STATUS) == 0)
      status = OL_PLAYER_PAUSED;
    else if (strcmp (prop, STOPPED_STATUS) == 0)
      status = OL_PLAYER_STOPPED;
    g_free (prop);
  }
  return status;
}

static gboolean
_seek (int pos_ms)
{
  if (!_ensure_proxy ())
    return FALSE;
  int position;
  if (!_get_played_time (&position))
    return FALSE;
  GError *error = NULL;
  if (!dbus_g_proxy_call (mpris2.proxy,
                          SEEK_METHOD,
                          &error,
                          G_TYPE_INT64, (pos_ms - position) * 1000,
                          G_TYPE_INVALID,
                          G_TYPE_INVALID))
  {
    ol_debugf ("Seek failed: %s\n", error->message);
    g_error_free (error);
    return FALSE;
  }
  return TRUE;
}

struct OlPlayer*
ol_player_mpris2_get () {
  static struct OlPlayer player = {
    .name = "MPRIS2",
    .get_activated = _get_activated,
    .get_played_time = _get_played_time,
    .get_music_length = _get_music_length,
    .get_music_info = _get_music_info,
    .get_capacity = _get_capacity,
    .play = _play,
    .pause = _pause,
    .stop = _stop,
    .prev = _prev,
    .next = _next,
    .seek = _seek,
    .get_status = _get_status,
  };
  return &player;
}
