#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ol_player.h"
#include "ol_player_exaile.h"
#include "ol_utils_dbus.h"
#include "ol_elapse_emulator.h"

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

static gboolean ol_player_exaile_get_music_info (OlMusicInfo *info);
static gboolean ol_player_exaile_get_played_time (int *played_time);
static gboolean ol_player_exaile_get_music_length (int *len);
static gboolean ol_player_exaile_get_activated ();
static gboolean ol_player_exaile_init_dbus ();
static enum OlPlayerStatus ol_player_exaile_get_status ();

static enum OlPlayerStatus
ol_player_exaile_get_status ()
{
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile_init_dbus ())
      return FALSE;
  char *status = NULL;
  enum OlPlayerStatus ret = OL_PLAYER_UNKNOWN;
  ol_dbus_get_string (proxy, get_status, &status);
  if (status != NULL)
  {
    /* fprintf (stderr, "status: %s\n", status); */
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
ol_player_exaile_get_music_info (OlMusicInfo *info)
{
  if (info == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile_init_dbus ())
      return FALSE;
  gchar *buf;
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
  printf ("%s\n"
          "  title:%s\n"
          "  artist:%s\n",
          __FUNCTION__,
          info->title,
          info->artist);
  return TRUE;
}

static gboolean
ol_player_exaile_get_played_time (int *played_time)
{
  /* printf ("%s\n", */
  /*         __FUNCTION__); */
  guint8 percent;
  int duration;
  int exaile_time;
  if (played_time == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_exaile_get_status ();
  if (status == OL_PLAYER_PLAYING || status == OL_PLAYER_PAUSED)
  {
    if (!ol_dbus_get_uint8 (proxy, current_position, &percent))
      return FALSE;
    if (!ol_player_exaile_get_music_length (&duration))
      return FALSE;
    exaile_time = duration * percent / 100;
    if (elapse_emulator == NULL)
    {
      elapse_emulator = g_new (OlElapseEmulator, 1);
      if (elapse_emulator != NULL)
        ol_elapse_emulator_init (elapse_emulator, exaile_time, duration / 100);
    }
    if (elapse_emulator != NULL)
    {
      if (status == OL_PLAYER_PLAYING)
        *played_time = ol_elapse_emulator_get_real_ms (elapse_emulator, exaile_time);
      else /* if (status == OL_PLAYER_PAUSED)  */
        *played_time = ol_elapse_emulator_get_last_ms (elapse_emulator, exaile_time);
    }
    else
      *played_time = exaile_time;
  }
  else
  {
    *played_time = -1;
  }
  return TRUE;
}

static gboolean
ol_player_exaile_get_music_length (int *len)
{
  /* printf ("%s\n", */
  /*         __FUNCTION__); */
  if (len == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile_init_dbus ())
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
ol_player_exaile_get_activated ()
{
  printf ("%s\n",
          __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_exaile_init_dbus ())
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
ol_player_exaile_init_dbus ()
{
  printf ("%s\n",
          __FUNCTION__);
  if (connection == NULL)
  {
    connection = dbus_g_bus_get (DBUS_BUS_SESSION,
                               &error);
    if (connection == NULL)
    {
      printf ("get connection failed: %s\n", error->message);
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
      printf ("get proxy failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
      return FALSE;
    }
  }
  return TRUE;
}

OlPlayerController*
ol_player_exaile_get_controller ()
{
  printf ("%s\n",
          __FUNCTION__);
  OlPlayerController *controller = g_new (OlPlayerController, 1);
  controller->get_music_info = ol_player_exaile_get_music_info;
  controller->get_activated = ol_player_exaile_get_activated;
  controller->get_played_time = ol_player_exaile_get_played_time;
  controller->get_music_length = ol_player_exaile_get_music_length;
  return controller;
}
