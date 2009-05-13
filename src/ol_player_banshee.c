#include <dbus/dbus-glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "ol_player_banshee.h"
#include "ol_utils.h"

static const char *service = "org.bansheeproject.Banshee";
static const char *path = "/org/bansheeproject/Banshee/PlaybackController";
static const char *interface = "org.bansheeproject.Banshee.PlaybackController";
static const char *path2 = "/org/bansheeproject/Banshee/PlayerEngine";
static const char *interface2 = "org.bansheeproject.Banshee.PlayerEngine";	
static const char *play = "Play";
static const char *pause = "Pause";
static const char *play_pause = "TogglePlaying";
static const char *next = "Next";
static const char *previous = "Previous";
static const char *get_title = "GetCurrentTrack";
static const char *get_artist = "";
static const char *get_album = "";
static const char *get_cover_path = "";
static const char *get_status = "GetCurrentState";
static const char *duration = "GetLength";
static const char *current_position = "GetPosition";

static DBusGConnection *connection = NULL;
static DBusGProxy *proxy = NULL;
static GError *error = NULL;

static gboolean ol_player_banshee_get_music_info (OlMusicInfo *info);
static gboolean ol_player_banshee_get_played_time (int *played_time);
static gboolean ol_player_banshee_get_music_length (int *len);
static gboolean ol_player_banshee_init_dbus ();
static gboolean ol_player_banshee_get_activated ();


static gboolean
ol_player_banshee_get_music_info (OlMusicInfo *info)
{
/*   printf ("%s\n", */
/*           __FUNCTION__); */
  if (info == NULL)
    return FALSE;
  GHashTable *data_list = NULL;
  gchar *str;
  gint int_val;
  gboolean ret = TRUE;
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  if (dbus_g_proxy_call (proxy,
                        get_title,
                        NULL,G_TYPE_INVALID,
                        dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
                        &data_list,
                        G_TYPE_INVALID))
  {
    if (info->artist)
      g_free (info->artist);
    info->artist = g_strdup (ol_get_string_from_hash_table (data_list, "artist"));
    if (info->album)
      g_free (info->album);
    info->album = g_strdup (ol_get_string_from_hash_table (data_list, "album"));
    if (info->title)
      g_free (info->title);
    info->title = ol_get_string_from_hash_table (data_list, "name");
    info->track_number = ol_get_int_from_hash_table (data_list, "track-number");
  }
  else
  {
    printf ("%s fail\n", get_title);
    ret = FALSE;
  }
  return ret;
}

static gboolean
ol_player_banshee_get_music_length (int *len)
{
  if (!len)
    return FALSE;
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  if (dbus_g_proxy_call (proxy,
                         duration,
                         NULL,
                         G_TYPE_INVALID,
                         G_TYPE_UINT,
                         len,
                         G_TYPE_INVALID))
  {
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}

static gboolean
ol_player_banshee_get_played_time (int *played_time)
{
  if (!played_time)
    return FALSE;
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  if (ol_dbus_get_uint (proxy,
                        current_position,
                        played_time))
  {
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}

static gboolean
ol_player_banshee_get_activated ()
{
/*   printf ("%s\n", */
/*           __FUNCTION__); */
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  gint int_val;
  if (ol_dbus_get_uint (proxy,
                        current_position,
                        &int_val))
  {
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}


static gboolean
ol_player_banshee_init_dbus ()
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
    {
      g_object_unref (proxy);
      proxy = NULL;
    }
  }
  if (proxy == NULL)
  {
    proxy = dbus_g_proxy_new_for_name_owner (connection, service, path2, interface2, &error);
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
ol_player_banshee_get_controller ()
{
  printf ("%s\n",
          __FUNCTION__);
  OlPlayerController *controller = g_new (OlPlayerController, 1);
  controller->get_music_info = ol_player_banshee_get_music_info;
  controller->get_activated = ol_player_banshee_get_activated;
  controller->get_played_time = ol_player_banshee_get_played_time;
  controller->get_music_length = ol_player_banshee_get_music_length;
  return controller;
}
