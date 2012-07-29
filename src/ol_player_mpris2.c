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
#include "ol_app_info.h"
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

struct Mpris2
{
  DBusGProxy *proxy;
  char *name;
  char *bus_name;
  char *icon_name;
  OlElapseEmulator *elapse;
};

static struct Mpris2 *_mpris2 = NULL;

struct KnownPlayers
{
  const char *name;
  const char *command;
  const char *bus_name;
  const char *icon_name;
  gboolean time_in_ms;
};

static struct KnownPlayers KNOWN_PLAYERS[] = {
  {"Rhythmbox", "rhythmbox", "org.mpris.MediaPlayer2.rhythmbox", "rhythmbox", FALSE},
};

static gboolean _get_activated (void);
static gboolean _get_played_time (int *played_time);
static gboolean _get_music_length (int *len);
static gboolean _get_music_info (OlMusicInfo *info);
static int _get_capacity (void);
static gboolean _play (void);
static gboolean _pause (void);
static gboolean _stop (void);
static gboolean _prev (void);
static gboolean _next (void);
static enum OlPlayerStatus _get_status (void);
static gboolean _seek (int pos_ms);
static gboolean _ensure_proxy (void);
static gboolean _try_dbus_name (const char *name);
static gboolean _proxy_destroy_cb (DBusGProxy *proxy, struct Mpris2 *mpris2);
static GHashTable *_get_metadata (void);
static const char *_get_icon_path (void);
static GList * _get_app_info_list (void);
static struct Mpris2 *_mpris2_new (const char *bus_name,
                                   DBusGProxy *proxy);
static void _mpris2_free (struct Mpris2 *mpris2);
static struct OlPlayer player = {0};

static gboolean
_proxy_destroy_cb (DBusGProxy *proxy, struct Mpris2 *userdata)
{
  ol_assert_ret (_mpris2 == userdata, FALSE);
  _mpris2_free (_mpris2);
  _mpris2 = NULL;
  return FALSE;
}

static struct Mpris2 *
_mpris2_new (const char *bus_name,
             DBusGProxy *proxy)
{
  ol_assert_ret (bus_name != NULL, NULL);
  ol_assert_ret (proxy != NULL, NULL);
  struct Mpris2 *ret = g_new0 (struct Mpris2, 1);
  ret->proxy = proxy;
  ret->bus_name = g_strdup (bus_name);
  gboolean time_in_ms = FALSE;
  int i;
  for (i = 0; i < G_N_ELEMENTS (KNOWN_PLAYERS); i++)
  {
    if (strcmp (bus_name, KNOWN_PLAYERS[i].bus_name) == 0)
    {
      ret->name = g_strdup (KNOWN_PLAYERS[i].name);
      ret->icon_name = g_strdup (KNOWN_PLAYERS[i].icon_name);
      time_in_ms = KNOWN_PLAYERS[i].time_in_ms;
    }
  }
  if (ret->icon_name == NULL)
    ret->icon_name = g_strdup (bus_name + strlen (NAME_PREFIX));
  if (ret->name == NULL)
    ret->name = g_strdup (ret->icon_name);
  g_signal_connect (ret->proxy,
                    "destroy",
                    G_CALLBACK (_proxy_destroy_cb),
                    (gpointer) ret);
  if (!time_in_ms)
    ret->elapse = ol_elapse_emulator_new (0, 1000);
  player.name = ret->name;
  return ret;
}

static void
_mpris2_free (struct Mpris2 *mpris2)
{
  ol_assert (mpris2 != NULL);
  g_object_unref (mpris2->proxy);
  mpris2->proxy = NULL;
  g_free (mpris2->bus_name);
  g_free (mpris2->icon_name);
  g_free (mpris2->name);
  if (mpris2->elapse)
    ol_elapse_emulator_free (mpris2->elapse);
  g_free (mpris2);
  player.name = "MPRIS2";
}

static gboolean
_try_dbus_name (const char *name)
{
  ol_debugf ("Trying MPRIS2: %s\n", name);
  DBusGConnection *connection = ol_dbus_get_connection ();
  if (connection == NULL)
    return FALSE;
  GError *error = NULL;
  DBusGProxy *proxy = dbus_g_proxy_new_for_name_owner (connection,
                                                       name,
                                                       PATH,
                                                       PLAYER_INTERFACE,
                                                       &error);
  if (proxy == NULL)
  {
    ol_debugf ("get proxy failed: %s\n", error->message);
    g_error_free (error);
    return FALSE;
  }
  _mpris2 = _mpris2_new (name, proxy);
  return TRUE;
}

static gboolean
_ensure_proxy (void)
{
  if (_mpris2 != NULL)
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
_get_metadata (void)
{
  if (_mpris2 == NULL)
    return NULL;
  GHashTable *ret = NULL;
  if (!ol_dbus_get_dict_property (_mpris2->proxy, METADATA_PROP, &ret))
    return NULL;
  return ret;
}

static gboolean
_get_activated (void)
{
  return _ensure_proxy ();
}

static gboolean
_get_played_time (int *played_time)
{
  ol_log_func ();
  ol_assert_ret (played_time != NULL, FALSE);
  if (_mpris2 == NULL)
    return FALSE;
  gint64 position = 0;
  if (!ol_dbus_get_int64_property (_mpris2->proxy,
                                   POSITION_PROP,
                                   &position))
  {
    if (g_str_equal (_mpris2->bus_name, "org.mpris.MediaPlayer2.rhythmbox") &&
        _get_status () != OL_PLAYER_ERROR)
    {
      /* Rhythmbox fails to provide position when stopped or changing tracks,
         we need to handle it more gracefully. */
      *played_time = 0;
      return TRUE;
    }
    return FALSE;
  }
  if (_mpris2->elapse != NULL)
  {
    if (_get_status () == OL_PLAYER_PLAYING)
      *played_time = ol_elapse_emulator_get_real_ms (_mpris2->elapse,
                                                     position / 1000);
    else
      *played_time = ol_elapse_emulator_get_last_ms (_mpris2->elapse,
                                                     position / 1000);
  }
  else
  {
    *played_time = position / 1000;
  }
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
_get_capacity (void)
{
  if (_mpris2 == NULL)
    return 0;
  int caps = OL_PLAYER_STATUS;
  gboolean prop = FALSE;
  if (ol_dbus_get_bool_property (_mpris2->proxy, CANPLAY_PROP, &prop) && prop)
    caps |= OL_PLAYER_PLAY;
  if (ol_dbus_get_bool_property (_mpris2->proxy, CANPAUSE_PROP, &prop) && prop)
    caps |= OL_PLAYER_PAUSE;
  if (ol_dbus_get_bool_property (_mpris2->proxy, CANPREV_PROP, &prop) && prop)
    caps |= OL_PLAYER_PREV;
  if (ol_dbus_get_bool_property (_mpris2->proxy, CANNEXT_PROP, &prop) && prop)
    caps |= OL_PLAYER_NEXT;
  if (ol_dbus_get_bool_property (_mpris2->proxy, CANSEEK_PROP, &prop) && prop)
    caps |= OL_PLAYER_SEEK;
  if (ol_dbus_get_bool_property (_mpris2->proxy, CANCONTROL_PROP, &prop) && prop)
    caps |= OL_PLAYER_STOP;
  return caps;
}

static gboolean
_play (void)
{
  if (_mpris2 == NULL)
    return 0;
  return ol_dbus_invoke (_mpris2->proxy, PLAY_METHOD);
}

static gboolean
_pause (void)
{
  if (_mpris2 == NULL)
    return 0;
  return ol_dbus_invoke (_mpris2->proxy, PAUSE_METHOD);
}

static gboolean
_stop (void)
{
  if (_mpris2 == NULL)
    return 0;
  return ol_dbus_invoke (_mpris2->proxy, STOP_METHOD);
}

static gboolean
_prev (void)
{
  if (_mpris2 == NULL)
    return 0;
  return ol_dbus_invoke (_mpris2->proxy, PREVIOUS_METHOD);
}

static gboolean
_next (void)
{
  if (_mpris2 == NULL)
    return 0;
  return ol_dbus_invoke (_mpris2->proxy, NEXT_METHOD);
}

static enum OlPlayerStatus
_get_status (void)
{
  if (_mpris2 == NULL)
    return OL_PLAYER_ERROR;
  enum OlPlayerStatus status = OL_PLAYER_UNKNOWN;
  char *prop = NULL;
  if (ol_dbus_get_string_property (_mpris2->proxy, STATUS_PROP, &prop))
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
  if (_mpris2 == NULL)
    return FALSE;
  int position;
  if (!_get_played_time (&position))
    return FALSE;
  GError *error = NULL;
  if (!dbus_g_proxy_call (_mpris2->proxy,
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

static const char *
_get_icon_path (void)
{
  return _mpris2->icon_name;
};

static GList *
_get_app_info_list (void)
{
  GList *ret = NULL;
  int i;
  for (i = 0; i < G_N_ELEMENTS (KNOWN_PLAYERS); i++)
  {
    GError *error = NULL;
    OlAppInfo *info = ol_app_info_new (KNOWN_PLAYERS[i].command,
                                       KNOWN_PLAYERS[i].name,
                                       KNOWN_PLAYERS[i].icon_name,
                                       OL_APP_INFO_PREFER_DESKTOP_FILE,
                                       &error);
    if (info)
    {
      ret = g_list_prepend (ret, info);
    }
    else
    {
      ol_errorf ("Cannot get player app info for %s: %s\n",
                 KNOWN_PLAYERS[i].name,
                 error->message);
      g_error_free (error);
    }
  }
  return ret;
}

struct OlPlayer*
ol_player_mpris2_get () {
  player.name = "MPRIS2";
  player.get_activated = _get_activated;
  player.get_played_time = _get_played_time;
  player.get_music_length = _get_music_length;
  player.get_music_info = _get_music_info;
  player.get_capacity = _get_capacity;
  player.play = _play;
  player.pause = _pause;
  player.stop = _stop;
  player.prev = _prev;
  player.next = _next;
  player.seek = _seek;
  player.get_icon_path = _get_icon_path;
  player.get_status = _get_status;
  player.get_app_info_list = _get_app_info_list;
  return &player;
}
