/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009 Tiger Soldier <tigersoldier@gmail.com>
 * Copyright (C) 2010 Sarlmol Apple <sarlmolapple@gmail.com>
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
#include <dbus/dbus-glib.h>
#include <glib.h>
#include "ol_player.h"
#include "ol_player_mpris.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
#include "ol_elapse_emulator.h"
#include "ol_music_info.h"
#include "ol_app_info.h"
#include "ol_debug.h"

enum MprisCaps {
  CAN_GO_NEXT           = 1 << 0,
  CAN_GO_PREV           = 1 << 1,
  CAN_PAUSE             = 1 << 2,
  CAN_PLAY              = 1 << 3,
  CAN_SEEK              = 1 << 4,
  CAN_PROVIDE_METADATA  = 1 << 5,
  CAN_HAS_TRACKLIST     = 1 << 6,
};

struct OlPlayerMpris
{
  gchar *app_name;
  gchar *bus_name;
  gchar *icon_name;
  DBusGProxy *proxy;
  DBusGProxyCall *position_call_id;
  DBusGProxyCall *metadata_call_id;
  DBusGProxyCall *status_call_id;
  OlElapseEmulator *elapse_emulator;
  enum OlPlayerStatus status;
  int played_time;
  gchar *title;
  gchar *artist;
  gchar *album;
  gchar *uri;
  int track_number;
  int music_len;
};

struct KnownPlayers
{
  const char *name;
  const char *command;
  const char *bus_name;
  const char *icon_name;
  gboolean time_in_ms;
  gboolean use_desktop_cmd;
};

static struct KnownPlayers KNOWN_PLAYERS[] = {
  {"Amarok 2", "amarok", "org.kde.amarok", "amarok", FALSE, TRUE}, /* Supported in 2.4 */
  {NULL, "audacious", "org.mpris.audacious", NULL, TRUE, TRUE},
  {"Clementine", "clementine", "org.mpris.clementine", "clementine", TRUE, TRUE},
  {"Decibel", "decibel-audio-player", "org.mpris.dap", "decibel-audio-player", FALSE, TRUE},
  {"Guayadeque", "guayadeque", "org.mpris.guayadeque", "guayadeque", FALSE, TRUE},
  {"Qmmp", "qmmp", "org.mpris.qmmp", "qmmp", FALSE, TRUE},
  {"VLC", "vlc --control dbus", "org.mpris.vlc", "vlc", TRUE, FALSE},
};

static const char *MPRIS_PREFIX = "org.mpris.";
static const char *MPRIS2_PREFIX = "org.mpris.MediaPlayer2";
static const char *PATH = "/Player";
static const char *ROOT_PATH = "/";
static const char *INTERFACE = "org.freedesktop.MediaPlayer";
static const char *IDENTITY = "Identity";
static const char *PLAY_METHOD = "Play";
static const char *PAUSE_METHOD = "Pause";
static const char *STOP_METHOD = "Stop";
static const char *NEXT_METHOD = "Next";
static const char *PREVIOUS_METHOD = "Prev";
static const char *GET_METADATA_METHOD = "GetMetadata";
static const char *GET_STATUS_METHOD = "GetStatus";
static const char *GET_POSITION_METHOD = "PositionGet";
static const char *SET_POSITION_METHOD = "PositionSet";
static struct OlPlayer controller = {0};

static struct OlPlayerMpris *mpris = NULL;

static struct OlPlayerMpris* _mpris_new (const char *app_name,
                                         const char *bus_name,
                                         DBusGProxy *proxy);
static void _mpris_free (struct OlPlayerMpris *mpris);
static gboolean _get_music_info (OlMusicInfo *info);
static gboolean _get_played_time (int *played_time);
static gboolean _get_music_length (int *len);
static gboolean _get_activated (void);
static int _get_capacity (void);
static enum OlPlayerStatus _get_status (void);
static gboolean _play (void);
static gboolean _pause (void);
static gboolean _stop (void);
static gboolean _prev (void);
static gboolean _next (void);
static gboolean _seek (int pos_ms);
static struct OlPlayerMpris *_init_dbus (const char *bus_name);
static gboolean _proxy_free (DBusGProxy *proxy, gpointer userdata);
static gboolean _update_metadata (struct OlPlayerMpris *mpris);
static void _get_played_time_cb(DBusGProxy *proxy,
                                DBusGProxyCall *call_id,
                                struct OlPlayerMpris *mpris);
static void _get_metadata_cb(DBusGProxy *proxy,
                             DBusGProxyCall *call_id,
                             struct OlPlayerMpris *mpris);
static void _get_status_cb(DBusGProxy *proxy,
                           DBusGProxyCall *call_id,
                           struct OlPlayerMpris *mpris);
static const char *_get_icon_path (void);
static GList *_get_app_info_list (void);
static void _free (void);

static void
_get_metadata_cb (DBusGProxy *proxy,
                  DBusGProxyCall *call_id,
                  struct OlPlayerMpris *mpris)
{
  ol_assert (proxy != NULL);
  ol_assert (mpris != NULL);
  if (call_id == mpris->metadata_call_id)
  {
    mpris->metadata_call_id = NULL;
    GHashTable *data_list = NULL;
    if (dbus_g_proxy_end_call (mpris->proxy,
                               call_id,
                               NULL,
                               dbus_g_type_get_map ("GHashTable",
                                                    G_TYPE_STRING,
                                                    G_TYPE_VALUE),
                               &data_list,
                               G_TYPE_INVALID))
    {
      g_free (mpris->artist);
      mpris->artist = g_strdup (ol_get_string_from_hash_table (data_list, "artist"));
      g_free (mpris->album);
      mpris->album = g_strdup (ol_get_string_from_hash_table (data_list, "album"));
      g_free (mpris->title);
      mpris->title = g_strdup (ol_get_string_from_hash_table (data_list, "title"));
      g_free (mpris->uri);
      mpris->uri = g_strdup (ol_get_string_from_hash_table (data_list, "location"));

      const char* track_number_str;
      track_number_str = ol_get_string_from_hash_table (data_list, "tracknumber");
      if (track_number_str != NULL)
        sscanf (track_number_str, "%d", &mpris->track_number);
      else
        mpris->track_number = ol_get_int_from_hash_table (data_list, "tracknumber");
      mpris->music_len = ol_get_uint_from_hash_table (data_list, "mtime");
      g_hash_table_destroy (data_list);
    }
  }
}

static gboolean
_update_metadata (struct OlPlayerMpris *mpris)
{
  ol_assert_ret (mpris != NULL, FALSE);
  if (mpris == NULL)
    return FALSE;
  if (mpris->metadata_call_id != NULL)
    return TRUE;
  mpris->metadata_call_id = dbus_g_proxy_begin_call (mpris->proxy,
                                                     GET_METADATA_METHOD,
                                                     (DBusGProxyCallNotify)_get_metadata_cb,
                                                     mpris,
                                                     NULL,
                                                     G_TYPE_INVALID);
  return mpris->metadata_call_id != NULL;
}

static gboolean
_get_music_info (OlMusicInfo *info)
{
  if (mpris == NULL)
    return FALSE;
  ol_assert_ret (info != NULL, FALSE);
  if (_update_metadata (mpris))
  {
    ol_music_info_clear (info);
    ol_music_info_set_artist (info, mpris->artist);
    ol_music_info_set_album (info, mpris->album);
    ol_music_info_set_title (info, mpris->title);
    ol_music_info_set_uri (info, mpris->uri);
    ol_music_info_set_track_number (info, mpris->track_number);
    return TRUE;
  } else {
    return FALSE;
  }
}

static void
_get_played_time_cb (DBusGProxy *proxy,
                     DBusGProxyCall *call_id,
                     struct OlPlayerMpris *mpris)
{
    mpris->position_call_id =NULL;
    GError *error = NULL;
    dbus_g_proxy_end_call (proxy,
                           call_id,
                           &error,
                           G_TYPE_INT,
                           &mpris->played_time,
                           G_TYPE_INVALID);
    if (error != NULL) {
      ol_errorf ("Error in method call : %s\n", error->message); 
      g_error_free (error);
    }
}
      
static gboolean
_get_played_time (int *played_time)
{
  ol_assert_ret (played_time != NULL, FALSE);
  if (mpris == NULL)
    return FALSE;
  if (mpris->position_call_id == NULL)
    mpris->position_call_id = dbus_g_proxy_begin_call (mpris->proxy,
                                              GET_POSITION_METHOD,
                                              (DBusGProxyCallNotify)_get_played_time_cb,
                                              mpris,
                                              NULL,
                                              G_TYPE_INVALID);
  if (mpris->elapse_emulator)
  {
    enum OlPlayerStatus status = _get_status ();
    if (status == OL_PLAYER_PLAYING)
      *played_time = ol_elapse_emulator_get_real_ms (mpris->elapse_emulator,
                                                     mpris->played_time);
    else
      *played_time = ol_elapse_emulator_get_last_ms (mpris->elapse_emulator,
                                                     mpris->played_time);
  }
  else
  {
    *played_time = mpris->played_time;
  }
  return TRUE;
}


static gboolean
_get_music_length (int *len)
{
  ol_assert_ret (len != NULL, FALSE);
  if (mpris == NULL)
    return FALSE;
  if (_update_metadata (mpris))
  {
    *len = mpris->music_len;
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}

static struct OlPlayerMpris*
_mpris_new (const char *app_name, const char *bus_name, DBusGProxy *proxy)
{
  ol_assert_ret (app_name != NULL, NULL);
  ol_assert_ret (proxy != NULL, NULL);
  ol_debug ("New MPRIS player: %s (%s)",
            app_name, bus_name);
  struct OlPlayerMpris *mpris = g_new0 (struct OlPlayerMpris, 1);
  mpris->app_name = g_strdup (app_name);
  mpris->bus_name = g_strdup (bus_name);
  mpris->proxy = proxy;
  int i;
  gboolean time_in_ms = FALSE;
  for (i = 0; i < G_N_ELEMENTS (KNOWN_PLAYERS); i++)
  {
    if (strcmp (bus_name,
                KNOWN_PLAYERS[i].bus_name) == 0)
    {
      time_in_ms = KNOWN_PLAYERS[i].time_in_ms;
      mpris->icon_name = g_strdup (KNOWN_PLAYERS[i].icon_name);
      break;
    }
  }
  if (!time_in_ms)
    mpris->elapse_emulator = ol_elapse_emulator_new (0, 1000);
  if (mpris->icon_name == NULL)
    mpris->icon_name = g_strdup (bus_name + strlen (MPRIS_PREFIX));
  controller.name = mpris->app_name;
  return mpris;
}

static void
_mpris_free (struct OlPlayerMpris *mpris)
{
  g_object_unref (mpris->proxy);
  /* if (mpris->call_id) */
  /*   dbus_g_proxy_cancel_call (proxy, mpris->call_id); */
  /* if (mpris->metadata_call_id) */
  /*   dbus_g_proxy_cancel_call (proxy, mpris->metadata_call_id); */
  g_free (mpris->title);
  g_free (mpris->artist);
  g_free (mpris->album);
  g_free (mpris->uri);
  g_free (mpris->app_name);
  g_free (mpris->icon_name);
  mpris->music_len = -1;
  mpris->played_time = -1;
  mpris->proxy = NULL;
  controller.name = "MPRIS";
  if (mpris->elapse_emulator != NULL)
    ol_elapse_emulator_free (mpris->elapse_emulator);
}

static struct OlPlayerMpris *
_init_dbus (const char *bus_name)
{
  ol_assert_ret (bus_name != NULL, NULL);
  ol_assert_ret (mpris == NULL, mpris);
  DBusGConnection *connection = ol_dbus_get_connection ();
  char *app_name = NULL;
  GError *error = NULL;
  struct OlPlayerMpris *ret = NULL;
  if (connection == NULL)
  {
    return NULL;
  }
  DBusGProxy *proxy = dbus_g_proxy_new_for_name_owner (connection,
                                                       bus_name,
                                                       PATH,
                                                       INTERFACE,
                                                       &error);
  if (proxy == NULL)
  {
    ol_debugf ("get proxy failed: %s\n", error->message);
    g_error_free (error);
    return NULL;
  }
  DBusGProxy *root_proxy = dbus_g_proxy_new_from_proxy (proxy,
                                                        INTERFACE,
                                                        ROOT_PATH);
  if (root_proxy != NULL)
  {
    if (!ol_dbus_get_string (root_proxy, IDENTITY, &app_name))
    {
      ol_errorf ("Cannot get identity from %s\n", bus_name);
    }
  }
  g_object_unref (root_proxy);
  if (app_name == NULL)
    app_name = g_strdup (bus_name);
  g_signal_connect (proxy, "destroy", G_CALLBACK (_proxy_free), NULL);
  ret = _mpris_new (app_name, bus_name, proxy);
  g_free (app_name);
  return ret;
}

static gboolean
_proxy_free (DBusGProxy *proxy, gpointer userdata)
{
  ol_assert_ret (mpris != NULL, FALSE);
  ol_assert_ret (mpris->proxy == proxy, FALSE);
  _mpris_free (mpris);
  mpris = NULL;
  return FALSE;
}

static gboolean
_find_player (void)
{
  ol_assert_ret (mpris == NULL, TRUE);
  char **names = ol_dbus_list_names ();
  if (names == NULL)
    return FALSE;
  char **bus_name;
  for (bus_name = names; *bus_name != NULL; bus_name++)
  {
    if (g_str_has_prefix (*bus_name, MPRIS_PREFIX) &&
        !g_str_has_prefix (*bus_name, MPRIS2_PREFIX))
    {
      mpris = _init_dbus (*bus_name);
      if (mpris != NULL)
        break;
    }
  }
  g_strfreev (names);
  return mpris != NULL;
}

static gboolean
_get_activated (void)
{
  if (mpris != NULL)
    return TRUE;
  return _find_player ();
}

static int
_get_capacity (void)
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    OL_PLAYER_STOP | OL_PLAYER_PREV | OL_PLAYER_NEXT | OL_PLAYER_SEEK;
}

static void
_get_status_cb(DBusGProxy *proxy,
               DBusGProxyCall *call_id,
               struct OlPlayerMpris *mpris)
{
  static const enum OlPlayerStatus status_map[] =
    {OL_PLAYER_PLAYING, OL_PLAYER_PAUSED, OL_PLAYER_STOPPED};
  ol_assert (mpris != NULL);

  GValueArray *status = NULL;
  if (call_id == mpris->status_call_id)
  {
    mpris->status_call_id = NULL;
    if (dbus_g_proxy_end_call (mpris->proxy,
                               call_id,
                               NULL,
                               dbus_g_type_get_struct("GValueArray",
                                                      G_TYPE_INT,
                                                      G_TYPE_INT,
                                                      G_TYPE_INT,
                                                      G_TYPE_INT,
                                                      G_TYPE_INVALID),
                               &status,
                               G_TYPE_INVALID))
    {
      GValue *value = g_value_array_get_nth (status, 0);
      gint stat = g_value_get_int (value);
      if (stat >= 0 && stat < ol_get_array_len (status_map))
        mpris->status = status_map[stat];
      else
        mpris->status = OL_PLAYER_UNKNOWN;
      g_value_array_free (status);
    }
    else
    {
      mpris->status = OL_PLAYER_ERROR;
    }
  }
}

static enum OlPlayerStatus
_get_status (void)
{
  if (mpris == NULL)
    return OL_PLAYER_ERROR;
  if (mpris->status_call_id == NULL)
    mpris->status_call_id = dbus_g_proxy_begin_call (mpris->proxy,
                                                     GET_STATUS_METHOD,
                                                     (DBusGProxyCallNotify)_get_status_cb,
                                                     mpris,
                                                     NULL,
                                                     G_TYPE_INVALID);
  return mpris->status;
}

static gboolean
_invoke_cmd (const char *cmd)
{
  if (mpris == NULL)
    return FALSE;
  return ol_dbus_invoke (mpris->proxy, cmd);
}

static gboolean
_play (void)
{
  return _invoke_cmd (PLAY_METHOD);
}

static gboolean
_pause (void)
{
  return _invoke_cmd (PAUSE_METHOD);
}

static gboolean
_stop (void)
{
  return _invoke_cmd (STOP_METHOD);
}

static gboolean
_prev (void)
{
  return _invoke_cmd (PREVIOUS_METHOD);
}

static gboolean
_next (void)
{
  return _invoke_cmd (NEXT_METHOD);
}

static gboolean
_seek (int pos_ms)
{
  if (mpris == NULL)
    return FALSE;
  mpris->played_time = pos_ms;
  if (mpris->position_call_id)
  {
    dbus_g_proxy_cancel_call (mpris->proxy, mpris->position_call_id);
    mpris->position_call_id = 0;
  }
  return dbus_g_proxy_call (mpris->proxy,
                            SET_POSITION_METHOD,
                            NULL,
                            G_TYPE_INT,
                            pos_ms,
                            G_TYPE_INVALID,
                            G_TYPE_INVALID);
}

static const char *
_get_icon_path (void)
{
  ol_assert_ret (mpris != NULL, NULL);
  return mpris->icon_name;
};

static void
_free (void)
{
  if (mpris)
  {
    _mpris_free (mpris);
  }
  mpris = NULL;
}

static GList *
_get_app_info_list (void)
{
  GList *ret = NULL;
  int i;
  for (i = 0; i < G_N_ELEMENTS (KNOWN_PLAYERS); i++)
  {
    GError *error = NULL;
    enum OlAppInfoFlags flags = OL_APP_INFO_USE_DESKTOP_ICON |
      OL_APP_INFO_USE_DESKTOP_NAME;
    if (KNOWN_PLAYERS[i].use_desktop_cmd)
      flags |= OL_APP_INFO_USE_DESKTOP_CMDLINE;
    OlAppInfo *info = ol_app_info_new (KNOWN_PLAYERS[i].command,
                                       KNOWN_PLAYERS[i].name,
                                       KNOWN_PLAYERS[i].icon_name,
                                       flags |
                                       OL_APP_INFO_WITH_PREFIX,
                                       &error);
    if (!info)
    {
      ol_errorf ("Cannot get player app info: %s\n",
                 error->message);
      g_error_free (error);
    }
    else
    {
      ret = g_list_prepend (ret, info);
    }
  }
  return ret;
}

struct OlPlayer*
ol_player_mpris_get (void)
{
  ol_log_func ();
  controller.name = "MPRIS";
  controller.get_music_info = _get_music_info;
  controller.get_activated = _get_activated;
  controller.get_played_time = _get_played_time;
  controller.get_music_length = _get_music_length;
  controller.get_capacity = _get_capacity;
  controller.get_status = _get_status;
  controller.play = _play;
  controller.pause = _pause;
  controller.stop = _stop;
  controller.prev = _prev;
  controller.next = _next;
  controller.seek = _seek;
  controller.get_icon_path = _get_icon_path;
  controller.get_app_info_list = _get_app_info_list;
  controller.free = _free;
  return &controller;
}
