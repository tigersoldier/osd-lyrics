/* -*- c -*- */

// Time-stamp: <2010-12-06 06:50:11 Monday by sarlmolapple>

/**
 * @file 
 * @author Sarlmol Apple
 */

#include "ol_player_listen.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
#include "ol_elapse_emulator.h"
#include "ol_debug.h"

static const char service[] = "org.gnome.Listen";
static const char path[] = "/org/gnome/listen";
static const char interface[] = "org.gnome.Listen";
static const char play[] = "play_pause";
static const char play_pause[] = "play_pause";
static const char stop[] = "quit";
static const char next[] = "next";
static const char previous[] = "previous";
static const char get_title[] = "get_title";
static const char get_artist[] = "get_artist";
static const char get_album[] = "get_album";
static const char get_uri[] = "get_uri";
static const char get_state[] = "playing";
static const char duration[] = "current_song_length";
static const char current_position[] = "current_position";
static const char change_volume[] = "volume";
static const char query[] = "hello";
static const char *icon_paths[] = {
  "/usr/share/app-install/icons/listen.png",
  "/usr/share/listen/img/bg_listen.png",
  "/usr/share/listen/img/listen.png",
  "/usr/share/pixmaps/listen.png",
};

static DBusGConnection *connection = NULL;
static DBusGProxy *proxy = NULL;
static GError *error = NULL;
static OlElapseEmulator *elapse_emulator = NULL;

static gboolean ol_player_listen_get_music_info (OlMusicInfo *info);
static gboolean ol_player_listen_get_played_time (int *played_time);
static gboolean ol_player_listen_get_music_length (int *len);
static gboolean ol_player_listen_get_activated ();
static gboolean ol_player_listen_init_dbus ();
static enum OlPlayerStatus ol_player_listen_get_status ();
static int ol_player_listen_get_capacity ();
static gboolean ol_player_listen_play ();
static gboolean ol_player_listen_pause ();
static gboolean ol_player_listen_stop ();
static gboolean ol_player_listen_prev ();
static gboolean ol_player_listen_next ();
static enum OlPlayerStatus ol_player_listen_parse_status (const gboolean status);
static const char *_get_icon_path ();

static enum OlPlayerStatus
ol_player_listen_parse_status (const gboolean status)
{
  if (status)
    return OL_PLAYER_PLAYING;
  else
    return OL_PLAYER_STOPPED;
}

static enum OlPlayerStatus
ol_player_listen_get_status ()
{
  if (connection == NULL || proxy == NULL)
    if (!ol_player_listen_init_dbus ())
      return OL_PLAYER_ERROR;
  gboolean buf = FALSE;
  enum OlPlayerStatus ret = OL_PLAYER_UNKNOWN;
  if (ol_dbus_get_bool (proxy, get_state, &buf)) 
  {
    ret = ol_player_listen_parse_status (buf);
    //g_free (buf);
  }
  return ret; 
}

static gboolean
ol_player_listen_get_music_info (OlMusicInfo *info)
{
  /* ol_log_func (); */
  ol_assert_ret (info != NULL, FALSE);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_listen_init_dbus ())
    {
      ol_debug ("Initialize dbus proxy failed\n");
      return FALSE;
    }
  enum OlPlayerStatus status = ol_player_listen_get_status ();
  ol_debugf ("  status: %d\n", (int)status);
  if (status == OL_PLAYER_PLAYING)
  {
    ol_music_info_clear (info);
    /* gets the title of current music */
    if (!ol_dbus_get_string (proxy,
                             get_title,
                             &info->title))
    {
      ol_error ("  Get title failed");
    }
    /* gets the artist of current music */
    if (!ol_dbus_get_string (proxy,
                             get_artist,
                             &info->artist))
    {
      ol_error ("  Get artist failed");
    }
    /* gets the album of current music */
    if (!ol_dbus_get_string (proxy,
                             get_album,
                             &info->album))
    {
      ol_error ("  Get album failed");
    }
    /* gets the location of the file */
    if (!ol_dbus_get_string (proxy,
                             get_uri,
                             &info->uri))
    {
      ol_error ("  Get track number failed");
    }
    return TRUE;
  }
  else if (status == OL_PLAYER_STOPPED)
  {
    return TRUE;
  }
  else
  {
    ol_errorf ("  unknown status\n");
    return FALSE;
  }
}

static gboolean
ol_player_listen_get_played_time (int *played_time)
{
  /* ol_log_func (); */
  int listen_time;
  if (played_time == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_listen_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_listen_get_status ();
  if (status == OL_PLAYER_PLAYING)
  {
    if (!ol_dbus_get_int (proxy, current_position, &listen_time))
      return FALSE;
    listen_time = listen_time*1000;
    if (elapse_emulator == NULL)
    {
      elapse_emulator = g_new (OlElapseEmulator, 1);
      if (elapse_emulator != NULL)
        ol_elapse_emulator_init (elapse_emulator, listen_time, 1000);
    }
    if (elapse_emulator != NULL)
    {
      if (status == OL_PLAYER_PLAYING)
        *played_time = ol_elapse_emulator_get_real_ms (elapse_emulator, listen_time);
      else /* if (status == OL_PLAYER_PAUSED)  */
        *played_time = ol_elapse_emulator_get_last_ms (elapse_emulator, listen_time);
    }
    else
      *played_time = listen_time;
  }
  else
  {
    *played_time = -1;
  }
  return TRUE;
}

static gboolean
ol_player_listen_get_music_length (int *len)
{
  if (len == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_listen_init_dbus ())
      return FALSE;
  gint64 buf;
  enum OlPlayerStatus status = ol_player_listen_get_status ();
  if (status == OL_PLAYER_PLAYING)
  {
    if (!ol_dbus_get_int64 (proxy, duration, &buf))
    {
      return FALSE;
    }
    *len = buf*1000;
    //g_free (buf);
    return TRUE;
  }
  else if (status == OL_PLAYER_STOPPED)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static gboolean
ol_player_listen_get_activated ()
{
  /* ol_log_func (); */
  if (connection == NULL || proxy == NULL)
    if (!ol_player_listen_init_dbus ())
      return FALSE;
  gchar *buf = NULL;
  if (ol_dbus_get_string (proxy, query, &buf))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static gboolean
ol_player_listen_init_dbus ()
{
  /* ol_log_func (); */
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
ol_player_listen_get_capacity ()
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_STOP |
    OL_PLAYER_PAUSE | OL_PLAYER_PREV | OL_PLAYER_NEXT;
}

static gboolean
ol_player_listen_stop ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_listen_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy, stop);
}

static gboolean
ol_player_listen_play ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_listen_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_listen_get_status ();
  if (status == OL_PLAYER_ERROR)
    return FALSE;
  switch (status)
  {
  case OL_PLAYER_PLAYING:
    return TRUE;
  case OL_PLAYER_STOPPED:
  default:
    return ol_dbus_invoke (proxy, play);
  }
}

static gboolean
ol_player_listen_pause ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_listen_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_listen_get_status ();
  if (status == OL_PLAYER_ERROR)
    return FALSE;
  if (status == OL_PLAYER_PLAYING)
    return ol_dbus_invoke (proxy, play_pause);
  return TRUE;
}

static gboolean
ol_player_listen_prev ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_listen_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy, previous);
}

static gboolean
ol_player_listen_next ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_listen_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy, next);
}

static const char *
_get_icon_path ()
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
ol_player_listen_get ()
{
  ol_logf (OL_DEBUG, "%s\n",__FUNCTION__);
  struct OlPlayer *controller = ol_player_new ("Listen Music Player");
  ol_player_set_cmd (controller, "listen");
  controller->get_music_info = ol_player_listen_get_music_info;
  controller->get_activated = ol_player_listen_get_activated;
  controller->get_played_time = ol_player_listen_get_played_time;
  controller->get_music_length = ol_player_listen_get_music_length;
  controller->get_capacity = ol_player_listen_get_capacity;
  controller->get_status = ol_player_listen_get_status;
  controller->play = ol_player_listen_play;
  controller->pause = ol_player_listen_pause;
  controller->prev = ol_player_listen_prev;
  controller->next = ol_player_listen_next;
  controller->seek = NULL;
  controller->stop = ol_player_listen_stop;
  controller->get_icon_path = _get_icon_path;
  return controller;
}

