#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ol_player.h"
#include "ol_player_exaile03.h"
#include "ol_utils_dbus.h"
#include "ol_elapse_emulator.h"

static const char service[] = "org.exaile.Exaile";
static const char path[] = "/org/exaile/Exaile";
static const char interface[] = "org.exaile.Exaile";
/* static const char play[] = "Play"; */
/* static const char play_pause[] = "PlayPause"; */
/* static const char stop[] = "Stop"; */
/* static const char next[] = "next_track"; */
/* static const char previous[] = "Prev"; */
static const char get_title[] = "title";
static const char get_artist[] = "artist";
static const char get_album[] = "album";
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

static enum OlPlayerStatus
ol_player_exaile03_get_status ()
{
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
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
  if (info == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  gchar *buf;
  /* gets the title of current music */
  if (info->title)
  {
    g_free (info->title);
    info->title = NULL;
  }
  if (!ol_dbus_get_string_with_str_arg (proxy,
                                        get_track_attr,
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
  if (!ol_dbus_get_string_with_str_arg (proxy,
                                        get_track_attr,
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
  if (!ol_dbus_get_string_with_str_arg (proxy,
                                        get_track_attr,
                                        get_album,
                                        &info->album))
  {
    return FALSE;
  }
  /* fprintf (stderr, */
  /*          "%s\n" */
  /*          "  title:%s\n" */
  /*          "  artist:%s\n" */
  /*          "  album:%s\n", */
  /*          __FUNCTION__, */
  /*          info->title, */
  /*          info->artist, */
  /*          info->album); */
  return TRUE;
}

static gboolean
ol_player_exaile03_get_played_time (int *played_time)
{
  /* fprintf (stderr, "%s\n", */
  /*          __FUNCTION__); */
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
  /* printf ("%s\n", */
  /*         __FUNCTION__); */
  if (len == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  gchar *buf = NULL;
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

static gboolean
ol_player_exaile03_get_activated ()
{
  fprintf (stderr, "%s\n",
           __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile03_init_dbus ())
      return FALSE;
  gchar *buf = NULL;
  if (ol_dbus_get_string (proxy, query, &buf))
  {
    return TRUE;
  }
  else
  {
    /* fprintf (stderr, "  failed\n"); */
    return FALSE;
  }
}

static gboolean
ol_player_exaile03_init_dbus ()
{
  fprintf (stderr, "%s\n",
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

OlPlayerController*
ol_player_exaile03_get_controller ()
{
  fprintf (stderr, "%s\n",
           __FUNCTION__);
  OlPlayerController *controller = g_new (OlPlayerController, 1);
  controller->get_music_info = ol_player_exaile03_get_music_info;
  controller->get_activated = ol_player_exaile03_get_activated;
  controller->get_played_time = ol_player_exaile03_get_played_time;
  controller->get_music_length = ol_player_exaile03_get_music_length;
  return controller;
}
