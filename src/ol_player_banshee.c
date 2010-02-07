#include <dbus/dbus-glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "ol_player_banshee.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"

static const char *service = "org.bansheeproject.Banshee";
static const char *path = "/org/bansheeproject/Banshee/PlaybackController";
static const char *interface = "org.bansheeproject.Banshee.PlaybackController";
static const char *path2 = "/org/bansheeproject/Banshee/PlayerEngine";
static const char *interface2 = "org.bansheeproject.Banshee.PlayerEngine";	
static const char *play = "Play";
static const char *pause = "Pause";
static const char *stop = "Close";
static const char *play_pause = "TogglePlaying";
static const char *next = "Next";
static const char *previous = "Previous";
static const char *seek = "SetPosition";
static const char *get_title = "GetCurrentTrack";
static const char *get_artist = "";
static const char *get_album = "";
static const char *get_cover_path = "";
static const char *get_status = "GetCurrentState";
static const char *duration = "GetLength";
static const char *current_position = "GetPosition";

static DBusGProxy *proxy = NULL;
static DBusGProxy *control_proxy = NULL;
static GError *error = NULL;

static gboolean ol_player_banshee_get_music_info (OlMusicInfo *info);
static gboolean ol_player_banshee_get_played_time (int *played_time);
static gboolean ol_player_banshee_get_music_length (int *len);
static gboolean ol_player_banshee_init_dbus ();
static gboolean ol_player_banshee_get_activated ();
static gboolean ol_player_banshee_proxy_destroy_handler (gpointer userdata);
static enum OlPlayerStatus ol_player_banshee_get_status ();
static int ol_player_banshee_get_capacity ();
static gboolean ol_player_banshee_play ();
static gboolean ol_player_banshee_pause ();
static gboolean ol_player_banshee_stop ();
static gboolean ol_player_banshee_prev ();
static gboolean ol_player_banshee_next ();
static gboolean ol_player_banshee_seek (int pos_ms);

static gboolean
ol_player_banshee_proxy_destroy_handler (gpointer userdata)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  g_object_unref (userdata);
  proxy = NULL;
  return FALSE;
}

static gboolean
ol_player_banshee_get_music_info (OlMusicInfo *info)
{
  /* fprintf (stderr, "%s\n", __FUNCTION__); */
  if (info == NULL)
    return FALSE;
  GHashTable *data_list = NULL;
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
    ol_music_info_clear (info);
    info->artist = g_strdup (ol_get_string_from_hash_table (data_list, "artist"));
    
    info->album = g_strdup (ol_get_string_from_hash_table (data_list, "album"));
    
    info->title = g_strdup (ol_get_string_from_hash_table (data_list, "name"));

    info->uri = g_strdup (ol_get_string_from_hash_table (data_list, "URI"));
    
    info->track_number = ol_get_int_from_hash_table (data_list, "track-number");
    g_hash_table_destroy (data_list);
  }
  else
  {
    fprintf (stderr, "%s fail\n", get_title);
    proxy = NULL;
    ret = FALSE;
  }
  return ret;
}

static gboolean
ol_player_banshee_get_music_length (int *len)
{
  /* fprintf (stderr, "%s\n", __FUNCTION__); */
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
  return TRUE;
}


static gboolean
ol_player_banshee_init_dbus ()
{
  printf ("%s\n",
          __FUNCTION__);
  DBusGConnection *connection = ol_dbus_get_connection ();
  if (connection == NULL)
  {
    return FALSE;
  }
  if (proxy != NULL)
  {
    g_object_unref (proxy);
    proxy = NULL;
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
    g_signal_connect (proxy, "destroy", G_CALLBACK (ol_player_banshee_proxy_destroy_handler), proxy);
  }
  if (control_proxy == NULL)
  {
    control_proxy = dbus_g_proxy_new_for_name_owner (connection, service, path, interface, &error);
    if (control_proxy == NULL)
    {
      printf ("get proxy failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
      return FALSE;
    }
    g_signal_connect (control_proxy, "destroy", G_CALLBACK (ol_player_banshee_proxy_destroy_handler), control_proxy);
  }
  return TRUE;
}

static enum
OlPlayerStatus ol_player_banshee_get_status ()
{
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return OL_PLAYER_ERROR;
  char *status_str = NULL;
  if (ol_dbus_get_string (proxy,
                          get_status,
                          &status_str))
  {
    enum OlPlayerStatus status;
    if (strcmp (status_str, "idle") == 0)
      status = OL_PLAYER_STOPPED;
    else if (strcmp (status_str, "playing") == 0)
      status = OL_PLAYER_PLAYING;
    else if (strcmp (status_str, "paused") == 0)
      status = OL_PLAYER_PAUSED;
    else
      status = OL_PLAYER_UNKNOWN;
    g_free (status_str);
    return status;
  }
  else
  {
    return OL_PLAYER_ERROR;
  }
}

static int
ol_player_banshee_get_capacity ()
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    OL_PLAYER_STOP | OL_PLAYER_PREV | OL_PLAYER_NEXT | OL_PLAYER_SEEK;
}

static gboolean
ol_player_banshee_play ()
{
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy,
                         play);
}

static gboolean
ol_player_banshee_pause ()
{
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy,
                         pause);
}

static gboolean
ol_player_banshee_stop ()
{
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy,
                         stop);
}

static gboolean
ol_player_banshee_prev ()
{
  if (control_proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  return dbus_g_proxy_call (control_proxy,
                            previous,
                            NULL,
                            G_TYPE_BOOLEAN,
                            TRUE,
                            G_TYPE_INVALID,
                            G_TYPE_INVALID);
}

static gboolean
ol_player_banshee_next ()
{
  if (control_proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  return dbus_g_proxy_call (control_proxy,
                            next,
                            NULL,
                            G_TYPE_BOOLEAN,
                            TRUE,
                            G_TYPE_INVALID,
                            G_TYPE_INVALID);
}

static gboolean
ol_player_banshee_seek (int pos_ms)
{
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  return dbus_g_proxy_call (proxy,
                            seek,
                            NULL,
                            G_TYPE_UINT,
                            (guint) pos_ms,
                            G_TYPE_INVALID,
                            G_TYPE_INVALID);
}

OlPlayerController*
ol_player_banshee_get_controller ()
{
  printf ("%s\n",
          __FUNCTION__);
  OlPlayerController *controller = ol_player_new ("Banshee");
  ol_player_set_cmd (controller, "banshee");
  controller->get_music_info = ol_player_banshee_get_music_info;
  controller->get_activated = ol_player_banshee_get_activated;
  controller->get_played_time = ol_player_banshee_get_played_time;
  controller->get_music_length = ol_player_banshee_get_music_length;
  controller->get_capacity = ol_player_banshee_get_capacity;
  controller->get_status = ol_player_banshee_get_status;
  controller->play = ol_player_banshee_play;
  controller->pause = ol_player_banshee_pause;
  controller->stop = ol_player_banshee_stop;
  controller->prev = ol_player_banshee_prev;
  controller->next = ol_player_banshee_next;
  controller->seek = ol_player_banshee_seek;
  return controller;
}
