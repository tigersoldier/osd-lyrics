#include <string.h>
#include "ol_player_muine.h"
#include "ol_elapse_emulator.h"
#include "ol_music_info.h"
#include "ol_utils_dbus.h"
#include "ol_utils.h"
#include "ol_debug.h"

static const char SERVICE[] = "org.gnome.Muine";
static const char INTERFACE[] = "org.gnome.Muine.Player";
static const char PATH[] = "/org/gnome/Muine/Player";
static const char GET_CURRENT_SONG[] = "GetCurrentSong";
static const char GET_POSITION[] = "GetPosition";
static const char SET_POSITION[] = "SetPosition";
static const char GET_PLAYING[] = "GetPlaying";
static const char SET_PLAYING[] = "SetPlaying";
static const char PREVIOUS[] = "Previous";
static const char NEXT[] = "Next";

static const char PROP_TITLE[] = "title";
static const char PROP_ARTIST[] = "artist";
static const char PROP_ALBUM[] = "album";
static const char PROP_TRACK_NUMBER[] = "track_number";
static const char PROP_URI[] = "uri";
static const char PROP_DURATION[] = "duration";

static const char *_icon_paths[] = {
  "/usr/share/icons/hicolor/scalable/apps/muine.svg",
  "/usr/share/icons/hicolor/48x48/apps/muine.png",
  "/usr/local/share/icons/hicolor/scalable/apps/muine.svg",
  "/usr/local/share/icons/hicolor/48x48/apps/muine.png",
};

static DBusGProxy *proxy = NULL;
static OlElapseEmulator *elapse_emulator = NULL;

static const gboolean _ensure_connection (void);
static const char *_get_icon_path (void);
static gboolean _get_activated (void);
static gboolean _get_played_time (int *played_time);
static gboolean _get_music_length (int *len);
static gboolean _get_music_info (OlMusicInfo *info);
static gboolean _get_music_info_and_length (OlMusicInfo *info, int *len);
static enum OlPlayerStatus _get_status (void);
static int _get_capacity (void);
static gboolean _set_playing (gboolean playing);
static gboolean _play (void);
static gboolean _pause (void);
static gboolean _prev (void);
static gboolean _next (void);
static gboolean _seek (int pos_ms);

static gboolean
_seek (int pos_ms)
{
  if (!_ensure_connection ())
    return FALSE;
  return ol_dbus_set_int (proxy, SET_POSITION, pos_ms / 1000);
}

static gboolean
_set_playing (gboolean playing)
{
  if (!_ensure_connection ())
    return FALSE;
  return ol_dbus_set_bool (proxy, SET_PLAYING, playing);
}

static gboolean
_play (void)
{
  return _set_playing (TRUE);
}

static gboolean
_pause (void)
{
  return _set_playing (FALSE);
}

static gboolean
_prev (void)
{
  if (!_ensure_connection ())
    return FALSE;
  return ol_dbus_invoke (proxy, PREVIOUS);
}

static gboolean
_next (void)
{
  if (!_ensure_connection ())
    return FALSE;
  return ol_dbus_invoke (proxy, NEXT);
}

static int
_get_capacity ()
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    OL_PLAYER_PREV | OL_PLAYER_NEXT | OL_PLAYER_SEEK | OL_PLAYER_URI;
}

static enum OlPlayerStatus
_get_status ()
{
  if (!_ensure_connection ())
    return OL_PLAYER_ERROR;
  gboolean playing;
  if (!ol_dbus_get_bool (proxy, GET_PLAYING, &playing))
    return FALSE;
  if (playing)
    return OL_PLAYER_PLAYING;
  else
    return OL_PLAYER_PAUSED;
}

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

static gboolean
_get_played_time (int *played_time)
{
  ol_assert_ret (played_time != NULL, FALSE);
  if (!_ensure_connection ())
    return FALSE;
  int muine_time = 0;
  if (!ol_dbus_get_int (proxy, GET_POSITION, &muine_time))
    return FALSE;
  muine_time *= 1000;
  _ensure_elapse (muine_time);
  enum OlPlayerStatus status = _get_status ();
  if (status == OL_PLAYER_PLAYING)
    *played_time = ol_elapse_emulator_get_real_ms (elapse_emulator, muine_time);
  else if (status == OL_PLAYER_PAUSED)
    *played_time = ol_elapse_emulator_get_last_ms (elapse_emulator, muine_time);
  else
    *played_time = 0;
  return TRUE;
}

static gboolean
_get_music_length (int *len)
{
  ol_assert_ret (len != NULL, FALSE);
  return _get_music_info_and_length (NULL, len);
}

static gboolean
_get_music_info (OlMusicInfo *info)
{
  ol_assert_ret (info != NULL, FALSE);
  return _get_music_info_and_length (info, NULL);
}

static gboolean
_get_music_info_and_length (OlMusicInfo *info, int *len)
{
  ol_log_func ();
  if (!_ensure_connection ())
    return FALSE;
  char *infostr = NULL;
  if (!ol_dbus_get_string (proxy, GET_CURRENT_SONG, &infostr))
    return FALSE;
  if (info != NULL)
    ol_music_info_clear (info);
  char *current, *next;
  current = infostr;
  while (current != NULL)
  {
    char *key, *value;
    next = ol_split_a_line (current);
    key = current;
    value = strchr (current, ':');
    *value = '\0'; value++;
    key = ol_trim_string (key);
    value = ol_trim_string (value);
    if (strcmp (key, PROP_TITLE) == 0)
    {
      if (info != NULL) ol_music_info_set_title (info, value);
    }
    else if (strcmp (key, PROP_ARTIST) == 0)
    {
      if (info != NULL) ol_music_info_set_artist (info, value);
    }
    else if (strcmp (key, PROP_ALBUM) == 0)
    {
      if (info != NULL) ol_music_info_set_album (info, value);
    }
    else if (strcmp (key, PROP_URI) == 0)
    {
      if (info != NULL) ol_music_info_set_uri (info, value);
    }
    else if (strcmp (key, PROP_TRACK_NUMBER) == 0)
    {
      if (info != NULL) ol_music_info_set_track_number (info, atoi (value));
    }
    else if (strcmp (key, PROP_DURATION) == 0)
    {
      if (len != NULL)
        *len = atoi (value) * 1000;
    }
    current = next;
  }
  return TRUE;
}

static gboolean
_get_activated ()
{
  return _ensure_connection ();
}

static const gboolean
_ensure_connection ()
{
  if (proxy != NULL) return TRUE;
  return ol_dbus_connect (SERVICE, PATH, INTERFACE, NULL, NULL, &proxy);
}

static const char *
_get_icon_path ()
{
  int i;
  for (i = 0; i < ol_get_array_len (_icon_paths); i++)
  {
    if (ol_path_is_file (_icon_paths[i]))
      return _icon_paths[i];
  }
  return NULL;
}

struct OlPlayer*
ol_player_muine_get ()
{
  ol_log_func ();
  struct OlPlayer *controller = ol_player_new ("Muine");
  ol_player_set_cmd (controller, "muine");
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
  controller->seek = _seek;
  controller->get_icon_path = _get_icon_path;
  return controller;
}

