#include <stdio.h>
#include "ol_player_juk.h"
#include "ol_utils_dbus.h"
#include "ol_utils.h"
#include "ol_elapse_emulator.h"
#include "ol_debug.h"

static const char SERVICE[] = "org.kde.juk";
static const char INTERFACE_PLAYER[] = "org.kde.juk.player";
static const char PATH_PLAYER[] = "/Player";
static const char PLAYING[] = "playing";
static const char PAUSED[] = "paused";
static const char TOTAL_TIME[] = "totalTime";
static const char CURRENT_TIME[] = "currentTime";
static const char TRACK_PROP[] = "trackProperty";
static const char TITLE[] = "Title";
static const char ARTIST[] = "Artist";
static const char ALBUM[] = "Album";
static const char PATH[] = "Path";
static const char TRACK[] = "Track";
static const char PLAY[] = "play";
static const char PAUSE[] = "pause";
static const char STOP[] = "stop";
static const char NEXT[] = "forward";
static const char PREV[] = "back";

static DBusGProxy *proxy = NULL;
static OlElapseEmulator *elapse_emulator = NULL;

static gboolean _ensure_dbus ();
static gboolean _destroy_dbus (DBusGProxy *_proxy, gboolean shell);
static int _get_capacity ();
static gboolean _get_played_time (int *played_time);
static enum OlPlayerStatus _get_status ();
static gboolean _get_activated ();
static gboolean _get_music_length (int *len);
static gboolean _get_played_time (int *played_time);
static gboolean _get_music_info (OlMusicInfo *info);
static void _ensure_elapse (int elapsed_time);
static gboolean _play (void);
static gboolean _pause (void);
static gboolean _stop (void);
static gboolean _prev (void);
static gboolean _next (void);

static void
_ensure_elapse (int elapsed_time)
{
  if (elapse_emulator == NULL)
  {
    elapse_emulator = g_new (OlElapseEmulator, 1);
    if (elapse_emulator != NULL)
      ol_elapse_emulator_init (elapse_emulator, elapsed_time, 1000);
  }
}

static int
_get_capacity ()
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    OL_PLAYER_STOP | OL_PLAYER_PREV | OL_PLAYER_NEXT | OL_PLAYER_URI;
}

static enum OlPlayerStatus
_get_status ()
{
  if (!_ensure_dbus())
    return OL_PLAYER_ERROR;
  gboolean playing, paused;
  if (!ol_dbus_get_bool (proxy, PLAYING, &playing))
    return OL_PLAYER_ERROR;
  if (playing)
    return OL_PLAYER_PLAYING;
  if (!ol_dbus_get_bool (proxy, PAUSED, &paused))
    return OL_PLAYER_ERROR;
  if (paused)
    return OL_PLAYER_PAUSED;
  else
    return OL_PLAYER_STOPPED;
}

static gboolean
_get_activated ()
{
  return _ensure_dbus ();
}

static gboolean
_get_music_length (int *len)
{
  ol_assert_ret (len != NULL, FALSE);
  if (!_ensure_dbus ())
    return FALSE;
  int juk_duration;
  if (!ol_dbus_get_int (proxy, TOTAL_TIME, &juk_duration))
    return FALSE;
  *len = juk_duration * 1000;
  return TRUE;
}

static gboolean
_get_played_time (int *played_time)
{
  ol_assert_ret (played_time != NULL, FALSE);
  if (!_ensure_dbus ())
    return FALSE;
  int juk_time;
  if (!ol_dbus_get_int (proxy, CURRENT_TIME, &juk_time))
    return FALSE;
  juk_time *= 1000;
  _ensure_elapse (juk_time);
  enum OlPlayerStatus status = _get_status ();
  if (status == OL_PLAYER_PLAYING)
    *played_time = ol_elapse_emulator_get_real_ms (elapse_emulator, juk_time);
  else if (status == OL_PLAYER_PAUSED)
    *played_time = ol_elapse_emulator_get_last_ms (elapse_emulator, juk_time);
  else
    *played_time = 0;
  return TRUE;
}

static gboolean
_get_music_info (OlMusicInfo *info)
{
  ol_assert_ret (info != NULL, FALSE);
  if (!_ensure_dbus ())
    return FALSE;
  ol_music_info_clear (info);
  char *value = NULL;
  if (!ol_dbus_get_string_with_str_arg (proxy, TRACK_PROP, TITLE, &value))
    return FALSE;
  ol_music_info_set_title (info, value);
  if (value != NULL) g_free (value);
  
  if (!ol_dbus_get_string_with_str_arg (proxy, TRACK_PROP, ARTIST, &value))
    return FALSE;
  ol_music_info_set_artist (info, value);
  if (value != NULL) g_free (value);
  
  if (!ol_dbus_get_string_with_str_arg (proxy, TRACK_PROP, ALBUM, &value))
    return FALSE;
  ol_music_info_set_album (info, value);
  if (value != NULL) g_free (value);

  /* FIXME: Escape the URI */
  if (!ol_dbus_get_string_with_str_arg (proxy, TRACK_PROP, PATH, &value))
    return FALSE;
  ol_music_info_set_uri (info, value);
  if (value != NULL) g_free (value);

  int track = -1;
  if (!ol_dbus_get_string_with_str_arg (proxy, TRACK_PROP, TRACK, &value))
    return FALSE;
  if (sscanf (value, "%d", &track) == 1)
    ol_music_info_set_track_number (info, track);
  if (value != NULL) g_free (value);

  /* ol_debugf("  artist:%s\n" */
  /*           "  title:%s\n" */
  /*           "  album:%s\n" */
  /*           "  track:%d\n" */
  /*           "  uri:%s\n", */
  /*           ol_music_info_get_artist (info), */
  /*           ol_music_info_get_title (info), */
  /*           ol_music_info_get_album (info), */
  /*           ol_music_info_get_track_number (info), */
  /*           ol_music_info_get_uri (info)); */
  return TRUE;
}

static gboolean
_play (void)
{
  if (!_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy, PLAY);
}

static gboolean
_pause (void)
{
  if (!_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy, PAUSE);
}

static gboolean
_stop (void)
{
  if (!_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy, STOP);
}

static gboolean
_prev (void)
{
  if (!_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy, PREV);
}

static gboolean
_next (void)
{
  if (!_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy, NEXT);
}

static gboolean
_destroy_dbus (DBusGProxy *_proxy, gboolean shell)
{
  ol_log_func ();
  g_object_unref (_proxy);
  proxy = NULL;
  return FALSE;
}

static gboolean
_ensure_dbus ()
{
  static DBusGConnection *connection = NULL;
  if (proxy != NULL)
    return TRUE;
  if (connection == NULL)
    connection = ol_dbus_get_connection ();
  if (connection == NULL)
    return FALSE;
  GError *error = NULL;
  proxy = dbus_g_proxy_new_for_name_owner (connection,
                                           SERVICE,
                                           PATH_PLAYER,
                                           INTERFACE_PLAYER,
                                           &error);
  if (proxy == NULL)
  {
    ol_infof ("get proxy failed: %s\n", error->message);
    g_error_free (error);
    error = NULL;
    return FALSE;
  }
  g_signal_connect (proxy,
                    "destroy",
                    G_CALLBACK (_destroy_dbus),
                    NULL);
  return TRUE;
}

struct OlPlayer*
ol_player_juk_get ()
{
  ol_log_func ();
  struct OlPlayer *controller = ol_player_new ("Juk");
  ol_player_set_cmd (controller, "juk");
  controller->get_music_info = _get_music_info;
  controller->get_activated = _get_activated;
  controller->get_played_time = _get_played_time;
  controller->get_music_length = _get_music_length;
  controller->get_capacity = _get_capacity;
  controller->get_status = _get_status;
  controller->play = _play;
  controller->pause = _pause;
  controller->prev = _prev;
  controller->next = _next;
  /* controller->seek = _seek; */
  controller->stop = _stop;
  /* controller->get_icon_path = _get_icon_path; */
  return controller;
}

