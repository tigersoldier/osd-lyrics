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
static GHashTable* ol_player_mpris_get_metadata (OlPlayerMpris *mpris);

OlPlayerMpris*
ol_player_mpris_new (const char *service)
{
  OlPlayerMpris *mpris = g_new (OlPlayerMpris, 1);
  mpris->name = g_strdup (service);
  mpris->proxy = NULL;
  return mpris;
}

static gboolean
ol_player_mpris_proxy_free (DBusGProxy *proxy, OlPlayerMpris *mpris)
{
  fprintf (stderr, "%s:%c\n", __FUNCTION__, *mpris->name);
  g_object_unref (mpris->proxy);
  mpris->proxy = NULL;
  return FALSE;
}

static GHashTable*
ol_player_mpris_get_metadata (OlPlayerMpris *mpris)
{
  GHashTable *data_list = NULL;
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  fprintf (stderr, "%s\n", __FUNCTION__);
  if (dbus_g_proxy_call (mpris->proxy,
                         GET_METADATA_METHOD,
                         NULL,G_TYPE_INVALID,
                         dbus_g_type_get_map ("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
                         &data_list,
                         G_TYPE_INVALID))
  {
    fprintf (stderr, "%s succeed\n", __FUNCTION__);
    return data_list;
  }
  else
  {
    return NULL;
  }
  return NULL;
}

gboolean
ol_player_mpris_get_music_info (OlPlayerMpris *mpris, OlMusicInfo *info)
{
  /* fprintf (stderr, "%s\n", __FUNCTION__); */
  GHashTable *data_list = NULL;
  gboolean ret = TRUE;
  if (info == NULL)
    return FALSE;
  if ((data_list = ol_player_mpris_get_metadata (mpris)) != NULL)
  {
    ol_music_info_clear (info);
    info->artist = g_strdup (ol_get_string_from_hash_table (data_list, "artist"));
    info->album = g_strdup (ol_get_string_from_hash_table (data_list, "album"));
    info->title = g_strdup (ol_get_string_from_hash_table (data_list, "title"));
    info->uri = g_strdup (ol_get_string_from_hash_table (data_list, "location"));

    gchar *track_number_str = NULL;
    track_number_str = ol_get_string_from_hash_table (data_list, "tracknumber");
    if (track_number_str != NULL)
    {
      sscanf (track_number_str, "%d", &info->track_number);
    }
    g_hash_table_destroy (data_list);
  }
  else
  {
    ret = FALSE;
  }
  return ret;
}

gboolean
ol_player_mpris_get_played_time (OlPlayerMpris *mpris, int *played_time)
{
  if (!played_time)
    return FALSE;
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  if (ol_dbus_get_int (mpris->proxy,
                       GET_POSITION_METHOD,
                       played_time))
  {
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}

gboolean
ol_player_mpris_get_music_length (OlPlayerMpris *mpris, int *len)
{
  if (!len)
    return FALSE;
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  GHashTable *metadata = NULL;
  if ((metadata = ol_player_mpris_get_metadata (mpris)) != NULL)
  {
    *len = ol_get_int_from_hash_table (metadata, "mtime");
    g_hash_table_destroy (metadata);
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
  fprintf (stderr, "%s\n", __FUNCTION__);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return TRUE;
}

static gboolean
ol_player_mpris_init_dbus (OlPlayerMpris *mpris)
{
  fprintf (stderr, "%s:%0x\n",
           __FUNCTION__, (int) mpris);
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
      fprintf (stderr, "get proxy failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
      return FALSE;
    }
    g_signal_connect (mpris->proxy, "destroy", G_CALLBACK (ol_player_mpris_proxy_free), (gpointer) mpris);
    printf ("MPRIS address: %0x\n", (int)mpris);
  }
  return TRUE;
}

int
ol_player_mpris_get_capacity (OlPlayerMpris *mpris)
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    OL_PLAYER_STOP | OL_PLAYER_PREV | OL_PLAYER_NEXT | OL_PLAYER_SEEK;
}

enum OlPlayerStatus
ol_player_mpris_get_status (OlPlayerMpris *mpris)
{
  GValueArray *status = NULL;
  g_return_val_if_fail (mpris != NULL, OL_PLAYER_ERROR);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return OL_PLAYER_ERROR;
  if (dbus_g_proxy_call (mpris->proxy,
                         GET_STATUS_METHOD,
                         NULL,G_TYPE_INVALID,
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
    switch (stat)
    {
    case 0:
      return OL_PLAYER_PLAYING;
    case 1:
      return OL_PLAYER_PAUSED;
    case 2:
      return OL_PLAYER_STOPPED;
    default:
      return OL_PLAYER_UNKNOWN;
    }
    g_value_array_free (status);
  }
  else
  {
    return OL_PLAYER_ERROR;
  }
}

gboolean
ol_player_mpris_play (OlPlayerMpris *mpris)
{
  g_return_val_if_fail (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return ol_dbus_invoke (mpris->proxy, PLAY_METHOD);
}

gboolean
ol_player_mpris_pause (OlPlayerMpris *mpris)
{
  g_return_val_if_fail (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return ol_dbus_invoke (mpris->proxy, PAUSE_METHOD);
}

gboolean
ol_player_mpris_stop (OlPlayerMpris *mpris)
{
  g_return_val_if_fail (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return ol_dbus_invoke (mpris->proxy, STOP_METHOD);
}

gboolean
ol_player_mpris_prev (OlPlayerMpris *mpris)
{
  g_return_val_if_fail (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return ol_dbus_invoke (mpris->proxy, PREVIOUS_METHOD);
}

gboolean
ol_player_mpris_next (OlPlayerMpris *mpris)
{
  g_return_val_if_fail (mpris != NULL, FALSE);
  if (mpris->proxy == NULL)
    if (!ol_player_mpris_init_dbus (mpris))
      return FALSE;
  return ol_dbus_invoke (mpris->proxy, NEXT_METHOD);
}

gboolean
ol_player_mpris_seek (OlPlayerMpris *mpris, int pos_ms)
{
  g_return_val_if_fail (mpris != NULL, FALSE);
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
