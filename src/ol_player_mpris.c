#include <stdio.h>
#include <dbus/dbus-glib.h>
#include "ol_player.h"
#include "ol_player_mpris.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
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

static const char *PATH = "/Player";
static const char *INTERFACE = "org.freedesktop.MediaPlayer";
static const char *PLAY_METHOD = "Play";
static const char *PAUSE_METHOD = "Pause";
static const char *STOP_METHOD = "Stop";
static const char *NEXT_METHOD = "Next";
static const char *PREVIOUS_METHOD = "Prev";
static const char *GET_METADATA_METHOD = "GetMetadata";
static const char *GET_STATUS_METHOD = "GetStatus";
static const char *GET_POSITION_METHOD = "PositionGet";
static const char *SET_POSITION_METHOD = "PositionSet";

static gboolean ol_player_mpris_init_dbus (OlPlayerMpris *mpris);
static gboolean ol_player_mpris_proxy_free (DBusGProxy *proxy, OlPlayerMpris *mpris);
static gboolean ol_player_mpris_update_metadata (OlPlayerMpris *mpris);
static void _clear_str (gchar **str);
static void _get_played_time_cb(DBusGProxy *proxy,
                                DBusGProxyCall *call_id,
                                OlPlayerMpris *mpris);
static void _get_metadata_cb(DBusGProxy *proxy,
                             DBusGProxyCall *call_id,
                             OlPlayerMpris *mpris);
static void _get_status_cb(DBusGProxy *proxy,
                           DBusGProxyCall *call_id,
                           OlPlayerMpris *mpris);

OlPlayerMpris*
ol_player_mpris_new (const char *service)
{
  OlPlayerMpris *mpris = g_new0 (OlPlayerMpris, 1);
  mpris->name = g_strdup (service);
  mpris->proxy = NULL;
  return mpris;
}

static void
_clear_str (gchar **str)
{
  if (str != NULL && *str != NULL)
  {
    g_free (*str);
    *str = NULL;
  }
}

static gboolean
ol_player_mpris_proxy_free (DBusGProxy *proxy, OlPlayerMpris *mpris)
{
  ol_assert_ret (mpris != NULL, FALSE);
  g_object_unref (mpris->proxy);
  if (mpris->call_id)
    dbus_g_proxy_cancel_call (proxy, mpris->call_id);
  if (mpris->metadata_call_id)
    dbus_g_proxy_cancel_call (proxy, mpris->metadata_call_id);
  _clear_str (&mpris->title);
  _clear_str (&mpris->artist);
  _clear_str (&mpris->album);
  _clear_str (&mpris->uri);
  mpris->music_len = -1;
  mpris->played_time = -1;
  mpris->proxy = NULL;
  return FALSE;
}

static void _get_metadata_cb(DBusGProxy *proxy,
                             DBusGProxyCall *call_id,
                             OlPlayerMpris *mpris)
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

      const gchar *track_number_str = NULL;
      track_number_str = ol_get_string_from_hash_table (data_list, "tracknumber");
      if (track_number_str != NULL)
      {
        sscanf (track_number_str, "%d", &mpris->track_number);
      }
      g_hash_table_destroy (data_list);
    }
  }
}

static gboolean
ol_player_mpris_update_metadata (OlPlayerMpris *mpris)
{
  ol_assert_ret (mpris != NULL, FALSE);
  if (mpris->proxy == NULL && !ol_player_mpris_init_dbus (mpris))
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

gboolean
ol_player_mpris_get_music_info (OlPlayerMpris *mpris, OlMusicInfo *info)
{
  ol_assert_ret (mpris != NULL, FALSE);
  ol_assert_ret (info != NULL, FALSE);
  if (ol_player_mpris_update_metadata (mpris))
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
_get_played_time_cb(DBusGProxy *proxy, DBusGProxyCall *call_id, OlPlayerMpris *mpris)
{
    mpris->call_id =NULL;
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
      
gboolean
ol_player_mpris_get_played_time (OlPlayerMpris *mpris, int *played_time)
{
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  if (mpris->call_id == NULL)
    mpris->call_id = dbus_g_proxy_begin_call (mpris->proxy,
                                              GET_POSITION_METHOD,
                                              (DBusGProxyCallNotify)_get_played_time_cb,
                                              mpris,
                                              NULL,
                                              G_TYPE_INVALID);
  *played_time = mpris->played_time;
  return TRUE;
}


gboolean
ol_player_mpris_get_music_length (OlPlayerMpris *mpris, int *len)
{
  ol_assert_ret (mpris != NULL, FALSE);
  ol_assert_ret (len != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  if (ol_player_mpris_update_metadata (mpris))
  {
    *len = mpris->music_len;
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}

gboolean
ol_player_mpris_get_activated (OlPlayerMpris *mpris)
{
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return TRUE;
}

static gboolean
ol_player_mpris_init_dbus (OlPlayerMpris *mpris)
{
  DBusGConnection *connection = ol_dbus_get_connection ();
  GError *error = NULL;
  if (connection == NULL)
  {
    return FALSE;
  }
  if (mpris->proxy == NULL)
  {
    mpris->proxy = dbus_g_proxy_new_for_name_owner (connection, mpris->name, PATH, INTERFACE, &error);
    if (mpris->proxy == NULL)
    {
      ol_debugf ("get proxy failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
      return FALSE;
    }
    g_signal_connect (mpris->proxy, "destroy", G_CALLBACK (ol_player_mpris_proxy_free), (gpointer) mpris);
  }
  return TRUE;
}

int
ol_player_mpris_get_capacity (OlPlayerMpris *mpris)
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    OL_PLAYER_STOP | OL_PLAYER_PREV | OL_PLAYER_NEXT | OL_PLAYER_SEEK;
}

static void
_get_status_cb(DBusGProxy *proxy,
               DBusGProxyCall *call_id,
               OlPlayerMpris *mpris)
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

enum OlPlayerStatus
ol_player_mpris_get_status (OlPlayerMpris *mpris)
{
  ol_assert_ret (mpris != NULL, OL_PLAYER_ERROR);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
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

gboolean
ol_player_mpris_play (OlPlayerMpris *mpris)
{
  ol_assert_ret (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return ol_dbus_invoke (mpris->proxy, PLAY_METHOD);
}

gboolean
ol_player_mpris_pause (OlPlayerMpris *mpris)
{
  ol_assert_ret (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return ol_dbus_invoke (mpris->proxy, PAUSE_METHOD);
}

gboolean
ol_player_mpris_stop (OlPlayerMpris *mpris)
{
  ol_assert_ret (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return ol_dbus_invoke (mpris->proxy, STOP_METHOD);
}

gboolean
ol_player_mpris_prev (OlPlayerMpris *mpris)
{
  ol_assert_ret (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return ol_dbus_invoke (mpris->proxy, PREVIOUS_METHOD);
}

gboolean
ol_player_mpris_next (OlPlayerMpris *mpris)
{
  ol_assert_ret (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return ol_dbus_invoke (mpris->proxy, NEXT_METHOD);
}

gboolean
ol_player_mpris_seek (OlPlayerMpris *mpris, int pos_ms)
{
  ol_assert_ret (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return dbus_g_proxy_call (mpris->proxy,
                            SET_POSITION_METHOD,
                            NULL,
                            G_TYPE_INT,
                            pos_ms,
                            G_TYPE_INVALID,
                            G_TYPE_INVALID);
}
