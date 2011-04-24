/* -*- c -*- */

// Time-stamp: <2010-12-08 13:56:23 Wednesday by sarlmolapple>

/**
 * @file ol_player_gmusicbrowser.c
 * @author sarlmolapple
 */

#include "ol_player_gmusicbrowser.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
#include "ol_elapse_emulator.h"
#include "ol_debug.h"

static const char service[] = "org.gmusicbrowser";
static const char path[] = "/org/gmusicbrowser";
static const char interface[] = "org.gmusicbrowser";
static const char play[] = "PlayPause";
static const char play_pause[] = "PlayPause";
static const char stop[] = "Quit";
static const char next[] = "NextSong";
static const char previous[] = "PrevSong";
static const char get_title[] = "title";
static const char get_artist[] = "artist";
static const char get_album[] = "album";
static const char get_uri[] = "path";
static const char get_state[] = "Playing";
static const char duration[] = "length";
static const char current_position[] = "GetPosition";
static const char change_volume[] = "volume";
static const char runcommand[] = "RunCommand";

static const char *icon_paths[] = {
    "/usr/share/app-install/icons/gmusicbrowser.png",
    "/usr/share/gmusicbrowser/pix/gmusicbrowser.png",
    "/usr/share/icons/gmusicbrowser.png",
    "/usr/share/icons/large/gmusicbrowser.png",
    "/usr/share/icons/mini/gmusicbrowser.png",
};

static DBusGConnection *connection = NULL;
static DBusGProxy *proxy = NULL;
static GError *error = NULL;
static OlElapseEmulator *elapse_emulator = NULL;

static gboolean ol_player_gmusicbrowser_get_music_info (OlMusicInfo *info);
static gboolean ol_player_gmusicbrowser_get_played_time (int *played_time);
static gboolean ol_player_gmusicbrowser_get_music_length (int *len);
static gboolean ol_player_gmusicbrowser_get_activated ();
static gboolean ol_player_gmusicbrowser_init_dbus ();
static enum OlPlayerStatus ol_player_gmusicbrowser_get_status ();
static int ol_player_gmusicbrowser_get_capacity ();
static gboolean ol_player_gmusicbrowser_play ();
static gboolean ol_player_gmusicbrowser_pause ();
static gboolean ol_player_gmusicbrowser_stop ();
static gboolean ol_player_gmusicbrowser_prev ();
static gboolean ol_player_gmusicbrowser_next ();
static enum OlPlayerStatus ol_player_gmusicbrowser_parse_status (const gboolean status);
static const char *_get_icon_path ();

static enum OlPlayerStatus
ol_player_gmusicbrowser_parse_status (const gboolean status)
{
  if (status)
    return OL_PLAYER_PLAYING;
  else
    return OL_PLAYER_STOPPED;
}

static enum OlPlayerStatus
ol_player_gmusicbrowser_get_status ()
{
  /* ol_logf (OL_DEBUG, "%s\n",__FUNCTION__); */
  if (connection == NULL || proxy == NULL)
    if (!ol_player_gmusicbrowser_init_dbus ())
    {
      return OL_PLAYER_ERROR;
    }
  gboolean buf = FALSE;
  enum OlPlayerStatus ret = OL_PLAYER_UNKNOWN;
  if (ol_dbus_get_bool (proxy, get_state, &buf)) 
  {
     ret = ol_player_gmusicbrowser_parse_status (buf);
  }
  return ret; 
}

static gboolean
ol_player_gmusicbrowser_get_music_info (OlMusicInfo *info)
{
  ol_assert_ret (info != NULL, FALSE);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_gmusicbrowser_init_dbus ())
    {
      ol_debug ("Initialize dbus proxy failed\n");
      return FALSE;
    }
  enum OlPlayerStatus status = ol_player_gmusicbrowser_get_status ();
  if (status == OL_PLAYER_PLAYING)
  {
    ol_music_info_clear (info);
    GHashTable *metadata = NULL;
    dbus_g_proxy_call (proxy,
                       "CurrentSong",
                       NULL,
                       G_TYPE_INVALID,
                       DBUS_TYPE_G_STRING_STRING_HASHTABLE,
                       &metadata,
                       G_TYPE_INVALID);
    /* gets the title of current music */
    info->title = (gchar *) g_hash_table_lookup (metadata, get_title);
    if (!info->title)
    {
      ol_error ("  Get title failed");
    }
    /* gets the artist of current music */
    info->artist = (gchar *) g_hash_table_lookup (metadata, get_artist);
    if (!info->artist)
    {
      ol_error ("  Get artist failed");
    }
    /* gets the album of current music */
    info->album = (gchar *) g_hash_table_lookup (metadata, get_album);
    if (!info->album)
    {
      ol_error ("  Get album failed");
    }
    /* gets the location of the file */
    info->uri = (gchar *) g_hash_table_lookup (metadata, get_uri);
    if (!info->uri)
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
ol_player_gmusicbrowser_get_played_time (int *played_time)
{
  /* ol_logf (OL_DEBUG, "%s\n",__FUNCTION__);*/
  int gmusicbrowser_time;
  gdouble _time;
  if (played_time == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_gmusicbrowser_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_gmusicbrowser_get_status ();
  if (status == OL_PLAYER_PLAYING)
  {
    if (!ol_dbus_get_double (proxy, current_position, &_time))
        return FALSE;
    gmusicbrowser_time = _time*1000;
    if (elapse_emulator == NULL)
    {
      elapse_emulator = g_new (OlElapseEmulator, 1);
      if (elapse_emulator != NULL)
        ol_elapse_emulator_init (elapse_emulator, gmusicbrowser_time, 1000);
    }
    if (elapse_emulator != NULL)
    {
      if (status == OL_PLAYER_PLAYING)
        *played_time = ol_elapse_emulator_get_real_ms (elapse_emulator, gmusicbrowser_time);
      else 
        *played_time = ol_elapse_emulator_get_last_ms (elapse_emulator, gmusicbrowser_time);
    }
    else
    *played_time = gmusicbrowser_time;
  }
  else
  {
    *played_time = -1;
  }
  return TRUE;
}

static gboolean
ol_player_gmusicbrowser_get_music_length (int *len)
{
  if (len == NULL)
    return FALSE;
  if (connection == NULL || proxy == NULL)
    if (!ol_player_gmusicbrowser_init_dbus ())
      return FALSE;
  int minute;
  enum OlPlayerStatus status = ol_player_gmusicbrowser_get_status ();
  if (status == OL_PLAYER_PLAYING)
  {
    GHashTable *metadata = NULL;
    dbus_g_proxy_call (proxy,
                       "CurrentSong",
                       NULL,
                       G_TYPE_INVALID,
                       DBUS_TYPE_G_STRING_STRING_HASHTABLE,
                       &metadata,
                       G_TYPE_INVALID);
    gchar *lenstr = NULL;
    lenstr = (gchar *) g_hash_table_lookup (metadata, duration);
    if (lenstr == NULL)
        return FALSE;
    sscanf (lenstr, "%d", &minute);
    *len = minute*1000;
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
ol_player_gmusicbrowser_get_activated ()
{
  /*ol_logf (OL_DEBUG, "%s\n",__FUNCTION__);*/
  if (connection == NULL || proxy == NULL)
    if (!ol_player_gmusicbrowser_init_dbus ())
      return FALSE;
  gboolean buf;
  if (ol_dbus_get_bool (proxy, get_state, &buf))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}

static gboolean
ol_player_gmusicbrowser_init_dbus ()
{
  /*ol_logf (OL_DEBUG, "%s\n",__FUNCTION__);*/  
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
ol_player_gmusicbrowser_get_capacity ()
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_STOP |
    OL_PLAYER_PAUSE | OL_PLAYER_PREV | OL_PLAYER_NEXT;
}

static gboolean
ol_player_gmusicbrowser_stop ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_gmusicbrowser_init_dbus ())
      return FALSE;
  return ol_dbus_invoke_with_str_arg (proxy, runcommand, stop);
}

static gboolean
ol_player_gmusicbrowser_play ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_gmusicbrowser_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_gmusicbrowser_get_status ();
  if (status == OL_PLAYER_ERROR)
    return FALSE;
  switch (status)
  {
  case OL_PLAYER_PLAYING:
    return TRUE;
  case OL_PLAYER_STOPPED:
  default:
      return ol_dbus_invoke_with_str_arg (proxy, runcommand, play);
  }
}

static gboolean
ol_player_gmusicbrowser_pause ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_gmusicbrowser_init_dbus ())
      return FALSE;
  enum OlPlayerStatus status = ol_player_gmusicbrowser_get_status ();
  if (status == OL_PLAYER_ERROR)
    return FALSE;
  if (status == OL_PLAYER_PLAYING)
      return ol_dbus_invoke_with_str_arg (proxy, runcommand, play_pause);
  return TRUE;
}

static gboolean
ol_player_gmusicbrowser_prev ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_gmusicbrowser_init_dbus ())
      return FALSE;
  return ol_dbus_invoke_with_str_arg (proxy, runcommand, previous);
}

static gboolean
ol_player_gmusicbrowser_next ()
{
  ol_logf (OL_DEBUG, "%s\n", __FUNCTION__);
  if (connection == NULL || proxy == NULL)
    if (!ol_player_gmusicbrowser_init_dbus ())
      return FALSE;
  return ol_dbus_invoke_with_str_arg (proxy, runcommand, next);
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
ol_player_gmusicbrowser_get ()
{
  ol_logf (OL_DEBUG, "%s\n",__FUNCTION__);
  struct OlPlayer *controller = ol_player_new ("Gmusicbrowser");
  ol_player_set_cmd (controller, "gmusicbrowser");
  controller->get_music_info = ol_player_gmusicbrowser_get_music_info;
  controller->get_activated = ol_player_gmusicbrowser_get_activated;
  controller->get_played_time = ol_player_gmusicbrowser_get_played_time;
  controller->get_music_length = ol_player_gmusicbrowser_get_music_length;
  controller->get_capacity = ol_player_gmusicbrowser_get_capacity;
  controller->get_status = ol_player_gmusicbrowser_get_status;
  controller->play = ol_player_gmusicbrowser_play;
  controller->pause = ol_player_gmusicbrowser_pause;
  controller->prev = ol_player_gmusicbrowser_prev;
  controller->next = ol_player_gmusicbrowser_next;
  controller->seek = NULL;
  controller->stop = ol_player_gmusicbrowser_stop;
  controller->get_icon_path = _get_icon_path;
  return controller;
}


