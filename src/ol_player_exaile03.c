#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ol_player_exaile03.h"
#include "ol_player.h"
#include "ol_utils_dbus.h"
#include "ol_elapse_emulator.h"
#include "ol_debug.h"

static const char service[] = "org.exaile.Exaile";
static const char path[] = "/org/exaile/Exaile";
static const char interface[] = "org.exaile.Exaile";
static const char play[] = "Play";
static const char play_pause[] = "PlayPause";
static const char stop[] = "Stop";
static const char next[] = "Next";
static const char previous[] = "Prev";
static const char get_title[] = "title";
static const char get_artist[] = "artist";
static const char get_album[] = "album";
static const char get_uri[] = "__loc";
/* static const char get_cover_path[] = "get_cover_path"; */
static const char get_status[] = "status";
static const char duration[] = "__length";
static const char current_position[] = "CurrentPosition";
static const char get_track_attr[] = "GetTrackAttr";
static const char set_track_attr[] = "SetTrackAttr";
static const char change_volume[] = "ChangeVolume";
static const char query[] = "Query";

static DBusGConnection *connection = NULL;
static DBusGProxy *proxy = NULL;
static GError *error = NULL;
static OlElapseEmulator *elapse_emulator = NULL;

static gboolean ol_player_exaile03_get_music_info (OlMusicInfo *info);
static gboolean ol_player_exaile03_get_played_time (int *played_time);
static gboolean ol_player_exaile03_get_music_length (int *len);
static gboolean ol_player_exaile03_get_activated ();
static gboolean ol_player_exaile03_init_dbus ();
static enum OlPlayerStatus ol_player_exaile03_get_status ();
static int ol_player_exaile03_get_capacity ();
static gboolean ol_player_exaile03_play ();
static gboolean ol_player_exaile03_pause ();
static gboolean ol_player_exaile03_stop ();
static gboolean ol_player_exaile03_prev ();
static gboolean ol_player_exaile03_next ();

static enum OlPlayerStatus
ol_player_exaile03_get_status ()
{
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return OL_PLAYER_ERROR;
  char *buf = NULL;
  enum OlPlayerStatus ret = OL_PLAYER_UNKNOWN;
  ol_dbus_get_string (proxy, query, &buf);
  char status[30];
  if (buf != NULL)
  {
    if (strcmp (buf, "Not playing.") == 0)
      ret = OL_PLAYER_STOPPED;
    else if (strstr (buf, "status:") != NULL)
    {
      sscanf (buf, "status: %[^,],", status);
      /* fprintf (stderr, "status: %s\n", status); */
      if (strcmp (status, "playing") == 0)
        ret = OL_PLAYER_PLAYING;
      else if (strcmp (status, "paused") == 0)
        ret = OL_PLAYER_PAUSED;
    }
    g_free (buf);
  }
  return ret;
}

static gboolean
ol_player_exaile03_get_music_info (OlMusicInfo *info)
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (info == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  gchar *buf;
  enum OlPlayerStatus status = ol_player_exaile03_get_status ();
  if (status == OL_PLAYER_PLAYING || status == OL_PLAYER_PAUSED)
  {
    /* gets the title of current music */
    ol_music_info_clear (info);
    if (!ol_dbus_get_string_with_str_arg (proxy,
                                          get_track_attr,
                                          get_title,
                                          &info->title))
    {
      return FALSE;
    }
    /* gets the artist of current music */
    if (!ol_dbus_get_string_with_str_arg (proxy,
                                          get_track_attr,
                                          get_artist,
                                          &info->artist))
    {
      return FALSE;
    }
    /* gets the album of current music */
    if (!ol_dbus_get_string_with_str_arg (proxy,
                                          get_track_attr,
                                          get_album,
                                          &info->album))
    {
      return FALSE;
    }
    if (!ol_dbus_get_string_with_str_arg (proxy,
                                          get_track_attr,
                                          get_uri,
                                          &info->uri))
    {
      return FALSE;
    }
    ol_logf (OL_DEBUG,
             "%s\n"
             "  title:%s\n"
             "  artist:%s\n"
             "  album:%s\n",
             __FUNCTION__,
             info->title,
             info->artist,
             info->album);
    return TRUE;
  }
  else if (status == OL_PLAYER_STOPPED)
  {
    return TRUE;
  }
  else
  {
    ol_logf (OL_DEBUG, "  unknown status\n");
    return FALSE;
  }
}

static gboolean
ol_player_exaile03_get_played_time (int *played_time)
{
  ol_logf (OL_DEBUG, "%s\n",
           __FUNCTION__);
  char *posstr = NULL;
  int minute, second;
  int exaile03_time;
  if (played_time == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_exaile03_get_status ();
  if (status == OL_PLAYER_PLAYING || status == OL_PLAYER_PAUSED)
  {
    if (!ol_dbus_get_string (proxy, current_position, &posstr))
      return FALSE;
    if (posstr == NULL)
      return FALSE;
    sscanf (posstr, "%d:%d", &minute, &second);
    g_free (posstr);
    exaile03_time = (minute * 60 + second) * 1000;
    if (elapse_emulator == NULL)
    {
      elapse_emulator = g_new (OlElapseEmulator, 1);
      if (elapse_emulator != NULL)
        ol_elapse_emulator_init (elapse_emulator, exaile03_time, 1000);
    }
    if (elapse_emulator != NULL)
    {
      if (status == OL_PLAYER_PLAYING)
        *played_time = ol_elapse_emulator_get_real_ms (elapse_emulator, exaile03_time);
      else /* if (status == OL_PLAYER_PAUSED)  */
        *played_time = ol_elapse_emulator_get_last_ms (elapse_emulator, exaile03_time);
    }
    else
      *played_time = exaile03_time;
  }
  else
  {
    *played_time = -1;
  }
  /* fprintf (stderr, "  %d\n", *played_time); */
  return TRUE;
}

static gboolean
ol_player_exaile03_get_music_length (int *len)
{
  ol_logf ("%s\n",
          __FUNCTION__);
  if (len == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  gchar *buf = NULL;
  enum OlPlayerStatus status = ol_player_exaile03_get_status ();
  if (status == OL_PLAYER_PLAYING || status == OL_PLAYER_PAUSED)
  {
    if (!ol_dbus_get_string_with_str_arg (proxy, get_track_attr, duration, &buf))
    {
      return FALSE;
    }
    int sec, usec;
    if (sscanf (buf, "%d.%d", &sec, &usec) == 2)
    {
      *len = sec * 1000 + usec / 1000000;
    }
    /* fprintf (stderr, "  %d\n", *len); */
    g_free (buf);
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
ol_player_exaile03_get_activated ()
{
  ol_logf (OL_DEBUG, "%s\n",
           __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  ol_logf (OL_DEBUG, "  init dbus success\n");
  gchar *buf = NULL;
  if (ol_dbus_get_string (proxy, query, &buf))
  {
    return TRUE;
  }
  else
  {
    ol_logf (OL_INFO, "exaile 0.3  get activated failed\n");
    return FALSE;
  }
}

static gboolean
ol_player_exaile03_init_dbus ()
{
  ol_logf (OL_DEBUG, "%s\n",
           __FUNCTION__);
  if (connection == NULL)
  {
    connection = dbus_g_bus_get (DBUS_BUS_SESSION,
                               &error);
    if (connection == NULL)
    {
      fprintf (stderr, "get connection failed: %s\n", error->message);
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
      fprintf (stderr, "get proxy failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
      return FALSE;
    }
  }
  return TRUE;
}

static int
ol_player_exaile03_get_capacity ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_STOP |
    OL_PLAYER_PAUSE | OL_PLAYER_PREV | OL_PLAYER_NEXT;
}

static gboolean
ol_player_exaile03_stop ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy, stop);
}

static gboolean
ol_player_exaile03_play ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_exaile03_get_status ();
  if (status == OL_PLAYER_ERROR)
    return FALSE;
  switch (status)
  {
  case OL_PLAYER_PAUSED:
    return ol_dbus_invoke (proxy, play_pause);
  case OL_PLAYER_PLAYING:
    return TRUE;
  case OL_PLAYER_STOPPED:
  default:
    return ol_dbus_invoke (proxy, play);
  }
}

static gboolean
ol_player_exaile03_pause ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_exaile03_get_status ();
  if (status == OL_PLAYER_ERROR)
    return FALSE;
  if (status == OL_PLAYER_PLAYING)
    return ol_dbus_invoke (proxy, play_pause);
}

static gboolean
ol_player_exaile03_prev ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy, previous);
}

static gboolean
ol_player_exaile03_next ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy, next);
}

OlPlayerController*
ol_player_exaile03_get_controller ()
{
  ol_logf (OL_DEBUG, "%s\n",__FUNCTION__);
  OlPlayerController *controller = g_new (OlPlayerController, 1);
  controller->get_music_info = ol_player_exaile03_get_music_info;
  controller->get_activated = ol_player_exaile03_get_activated;
  controller->get_played_time = ol_player_exaile03_get_played_time;
  controller->get_music_length = ol_player_exaile03_get_music_length;
  controller->get_capacity = ol_player_exaile03_get_capacity;
  controller->get_status = ol_player_exaile03_get_status;
  controller->play = ol_player_exaile03_play;
  controller->pause = ol_player_exaile03_pause;
  controller->prev = ol_player_exaile03_prev;
  controller->next = ol_player_exaile03_next;
  controller->seek = NULL;
  controller->stop = ol_player_exaile03_stop;
  return controller;
}

