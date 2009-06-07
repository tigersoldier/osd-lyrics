#include <stdio.h>
#include <dbus/dbus-glib.h>
#include "ol_player_mpris.h"
#include "ol_utils.h"

static const char *SERVICE = "org.kde.amarok";
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

static DBusGConnection *connection = NULL;
static DBusGProxy *proxy = NULL;
static GError *error = NULL;

static gboolean ol_player_mpris_init_dbus ();
static gboolean ol_player_mpris_get_music_info (OlMusicInfo *info);
static gboolean ol_player_mpris_get_played_time (int *played_time);
static gboolean ol_player_mpris_get_music_length (int *len);
static gboolean ol_player_mpris_get_activated ();
static GHashTable* ol_player_mpris_get_metadata ();

static GHashTable*
ol_player_mpris_get_metadata ()
{
  GHashTable *data_list = NULL;
  if (proxy == NULL)
    if (!ol_player_mpris_init_dbus ())
      return FALSE;
  if (dbus_g_proxy_call (proxy,
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

static gboolean
ol_player_mpris_get_music_info (OlMusicInfo *info)
{
  GHashTable *data_list = NULL;
  gboolean ret = TRUE;
  if (info == NULL)
    return FALSE;
  if ((data_list = ol_player_mpris_get_metadata ()) != NULL)
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

static gboolean
ol_player_mpris_get_played_time (int *played_time)
{
  if (!played_time)
    return FALSE;
  if (proxy == NULL)
    if (!ol_player_mpris_init_dbus ())
      return FALSE;
  if (ol_dbus_get_int (proxy,
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

static gboolean
ol_player_mpris_get_music_length (int *len)
{
  if (!len)
    return FALSE;
  if (proxy == NULL)
    if (!ol_player_mpris_init_dbus ())
      return FALSE;
  GHashTable *metadata = NULL;
  if ((metadata = ol_player_mpris_get_metadata ()) != NULL)
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

static gboolean
ol_player_mpris_get_activated ()
{
  int dummy;
  fprintf (stderr, "%s\n", __FUNCTION__);
  return ol_player_mpris_get_music_length (&dummy);
}

static gboolean
ol_player_mpris_init_dbus ()
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
    proxy = dbus_g_proxy_new_for_name_owner (connection, SERVICE, PATH, INTERFACE, &error);
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
