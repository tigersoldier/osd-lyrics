#include "ol_player_quodlibet.h"
#include "ol_utils_dbus.h"
#include "ol_utils.h"
#include "ol_debug.h"

const char *DBUS_NAME = "net.sacredchao.QuodLibet";
const char *DBUS_PATH = "/net/sacredchao/QuodLibet";
const char *DBUS_INTERFACE = "net.sacredchao.QuodLibet";
const char *PLAY = "Play";
const char *PAUSE = "Pause";
const char *GET_POSITION = "GetPosition";
const char *PREV = "Previous";
const char *NEXT = "Next";
const char *CURRENT_SONG = "CurrentSong";
const char *TITLE = "title";
const char *ARTIST = "artist";
const char *TRACK_NUMBER = "tracknumber";
const char *ALBUM = "album";
const char *LENGTH = "~#length";
const char *IS_PLAYING = "IsPlaying";

static DBusGProxy *proxy = NULL;
static GError *error = NULL;

static gboolean internal_get_music_info (OlMusicInfo *info);
static gboolean internal_get_played_time (int *played_time);
static gboolean internal_get_music_length (int *len);
static gboolean internal_ensure_dbus ();
static gboolean internal_get_activated ();
static gboolean internal_proxy_destroy_handler (DBusGProxy *proxy,
                                                gpointer userdata);
static enum OlPlayerStatus internal_get_status ();
static int internal_get_capacity ();
static gboolean internal_play ();
static gboolean internal_pause ();
static gboolean internal_prev ();
static gboolean internal_next ();
static gboolean internal_invoke (const char *method);

static int
internal_get_capacity ()
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    OL_PLAYER_PREV | OL_PLAYER_NEXT;
}

static gboolean
internal_invoke (const char *method)
{
  ol_log_func ();
  ol_debugf ("  Method: %s\n", method);
  ol_assert (method != NULL);
  if (!internal_ensure_dbus ())
    return FALSE;
  return ol_dbus_invoke (proxy, method);
}

static gboolean
internal_play ()
{
  return internal_invoke (PLAY);
}

static gboolean
internal_pause ()
{
  return internal_invoke (PAUSE);
}

static gboolean
internal_prev ()
{
  return internal_invoke (PREV);
}

static gboolean
internal_next ()
{
  return internal_invoke (NEXT);
}

static enum OlPlayerStatus
internal_get_status ()
{
  ol_log_func ();
  if (!internal_ensure_dbus ())
    return OL_PLAYER_ERROR;
  gboolean playing;
  int ret;
  if (ol_dbus_get_bool (proxy, IS_PLAYING, &playing))
  {
    if (playing)
      ret = OL_PLAYER_PLAYING;
    else
      ret = OL_PLAYER_PAUSED;
  }
  else
  {
    ret = OL_PLAYER_ERROR;
  }
  return ret;
}

static gboolean
internal_proxy_destroy_handler (DBusGProxy *proxy,
                                gpointer userdata)
{
  ol_log_func ();
  g_object_unref (proxy);
  proxy = NULL;
  return FALSE;
}

static gboolean
internal_get_music_length (int *len)
{
  ol_log_func ();
  ol_assert_ret (len != NULL, FALSE);
  if (!internal_ensure_dbus ())
    return FALSE;
  GHashTable *data_list = NULL;
  gboolean ret = TRUE;
  if (dbus_g_proxy_call (proxy,
                         CURRENT_SONG,
                         NULL, G_TYPE_INVALID,
                         dbus_g_type_get_map ("GHashTable",
                                              G_TYPE_STRING,
                                              G_TYPE_STRING),
                         &data_list,
                         G_TYPE_INVALID))
  {
    char *len_str = g_hash_table_lookup (data_list, LENGTH);
    if (len_str == NULL)
    {
      ret = FALSE;
    }
    else
    {
      sscanf (len_str, "%d", len);
      *len *= 60;
    }
    g_hash_table_destroy (data_list);
  }
  else
  {
    ret = FALSE;
  }
  return ret;
}

static gboolean
internal_get_played_time (int *played_time)
{
  ol_assert_ret (played_time != NULL, FALSE);
  if (!internal_ensure_dbus ())
    return FALSE;
  gint64 ret_time;
  gboolean ret = ol_dbus_get_int64 (proxy, GET_POSITION, &ret_time);
  *played_time = ret_time;
  return ret;
}

static gboolean
internal_get_activated ()
{
  ol_log_func ();
  return internal_ensure_dbus ();
}

static gboolean
internal_get_music_info (OlMusicInfo *info)
{
  ol_log_func ();
  ol_assert_ret (info != NULL, FALSE);
  if (!internal_ensure_dbus ())
    return FALSE;
  GHashTable *data_list = NULL;
  if (dbus_g_proxy_call (proxy,
                         CURRENT_SONG,
                         NULL, G_TYPE_INVALID,
                         dbus_g_type_get_map ("GHashTable",
                                              G_TYPE_STRING,
                                              G_TYPE_STRING),
                         &data_list,
                         G_TYPE_INVALID))
  {
    ol_music_info_clear (info);
    ol_debug ("got it");
    
    ol_music_info_set_title (info,
                             g_hash_table_lookup (data_list, TITLE));
    ol_music_info_set_album (info,
                             g_hash_table_lookup (data_list, ALBUM));
    ol_music_info_set_artist (info,
                              g_hash_table_lookup (data_list, ARTIST));
    char *number_str = g_hash_table_lookup (data_list, TRACK_NUMBER);
    if (number_str != NULL)
    {
      int track_num = 0;
      sscanf (number_str, "%d", &track_num);
      ol_music_info_set_track_number (info, track_num);
    }
    g_hash_table_destroy (data_list);
    ol_debugf("  artist:%s\n"
              "  title:%s\n"
              "  album:%s\n"
              "  track:%d\n"
              "  uri:%s\n",
              info->artist,
              info->title,
              info->album,
              info->track_number,
              info->uri);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static gboolean
internal_ensure_dbus ()
{
  if (proxy != NULL)
    return TRUE;
  DBusGConnection *connection = ol_dbus_get_connection ();
  if (connection == NULL)
  {
    return FALSE;
  }
  proxy = dbus_g_proxy_new_for_name_owner (connection, DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, &error);
  if (proxy == NULL)
  {
    ol_debugf ("get proxy failed: %s\n", error->message);
    g_error_free (error);
    error = NULL;
    return FALSE;
  }
  g_signal_connect (proxy,
                    "destroy",
                    G_CALLBACK (internal_proxy_destroy_handler),
                    NULL);
  return TRUE;
}

OlPlayerController*
ol_player_quodlibet_get_controller ()
{
  ol_log_func ();
  OlPlayerController *controller = ol_player_new ("Quod Libet");
  ol_player_set_cmd (controller, "quodlibet");
  controller->get_music_info = internal_get_music_info;
  controller->get_activated = internal_get_activated;
  controller->get_played_time = internal_get_played_time;
  controller->get_music_length = internal_get_music_length;
  controller->get_capacity = internal_get_capacity;
  controller->get_status = internal_get_status;
  controller->play = internal_play;
  controller->pause = internal_pause;
  /* controller->stop = internal_stop; */
  controller->prev = internal_prev;
  controller->next = internal_next;
  /* controller->seek = internal_seek; */
  return controller;
}

