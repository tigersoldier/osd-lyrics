#include <stdio.h>
#include <dbus/dbus-glib.h>
#include "ol_player_mpris.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"

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

static gboolean ol_player_mpris_init_dbus (OlPlayerMpris *mpris);
static gboolean ol_player_mpris_proxy_free (OlPlayerMpris *mpris);
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
ol_player_mpris_proxy_free (OlPlayerMpris *mpris)
{
  fprintf (stderr, "%s:%s\n", __FUNCTION__, mpris->name);
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
  if (dbus_g_proxy_call (mpris->proxy,
                         GET_METADATA_METHOD,
                         NULL,G_TYPE_INVALID,
                         dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
                         &data_list,
                         G_TYPE_INVALID))
  {
    return data_list;
  }
  else
  {
    return NULL;
  }
}

gboolean
ol_player_mpris_get_music_info (OlPlayerMpris *mpris, OlMusicInfo *info)
{
  GHashTable *data_list = NULL;
  gboolean ret = TRUE;
  if (info == NULL)
    return FALSE;
  if ((data_list = ol_player_mpris_get_metadata (mpris)) != NULL)
  {
    if (info->artist)
      g_free (info->artist);
    info->artist = g_strdup (ol_get_string_from_hash_table (data_list, "artist"));
    
    if (info->album)
      g_free (info->album);
    info->album = g_strdup (ol_get_string_from_hash_table (data_list, "album"));
    
    if (info->title)
      g_free (info->title);
    info->title = g_strdup (ol_get_string_from_hash_table (data_list, "title"));

    gchar *track_number_str = NULL;
    track_number_str = ol_get_string_from_hash_table (data_list, "tracknumber");
    if (track_number_str != NULL)
    {
      sscanf (track_number_str, "%d", &info->track_number);
      g_free (track_number_str);
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
  fprintf (stderr, "%s\n",
           __FUNCTION__);
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
    g_signal_connect (mpris->proxy, "destroy", G_CALLBACK (ol_player_mpris_proxy_free), mpris);
  }
  return TRUE;
}
