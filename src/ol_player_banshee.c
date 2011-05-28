#include <dbus/dbus-glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ol_player_banshee.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
#include "ol_debug.h"

static const char *SERVICE = "org.bansheeproject.Banshee";
static const char *PATH = "/org/bansheeproject/Banshee/PlaybackController";
static const char *INTERFACE = "org.bansheeproject.Banshee.PlaybackController";
static const char *PATH2 = "/org/bansheeproject/Banshee/PlayerEngine";
static const char *INTERFACE2 = "org.bansheeproject.Banshee.PlayerEngine";	
static const char *PLAY = "Play";
static const char *PAUSE = "Pause";
static const char *STOP = "Close";
static const char *NEXT = "Next";
static const char *PREVIOUS = "Previous";
static const char *SEEK = "SetPosition";
static const char *GET_TITLE = "GetCurrentTrack";
static const char *GET_STATUS = "GetCurrentState";
static const char *DURATION = "GetLength";
static const char *CURRENT_POSITION = "GetPosition";
static const char *ICON_PATHS[] = {
  "/usr/share/icons/hicolor/48x48/apps/media-player-banshee.png",
  "/usr/local/share/icons/hicolor/48x48/apps/media-player-banshee.png",
};
static DBusGProxy *proxy = NULL;
static DBusGProxy *control_proxy = NULL;
static GError *error = NULL;

static gboolean ol_player_banshee_get_music_info (OlMusicInfo *info);
static gboolean ol_player_banshee_get_played_time (int *played_time);
static gboolean ol_player_banshee_get_music_length (int *len);
static gboolean ol_player_banshee_init_dbus ();
static gboolean ol_player_banshee_get_activated ();
static gboolean ol_player_banshee_proxy_destroy_handler (DBusGProxy *proxy, gpointer userdata);
static enum OlPlayerStatus ol_player_banshee_get_status ();
static int ol_player_banshee_get_capacity ();
static gboolean ol_player_banshee_play ();
static gboolean ol_player_banshee_pause ();
static gboolean ol_player_banshee_stop ();
static gboolean ol_player_banshee_prev ();
static gboolean ol_player_banshee_next ();
static gboolean ol_player_banshee_seek (int pos_ms);
static const char *ol_player_banshee_get_icon_path ();

static gboolean
ol_player_banshee_proxy_destroy_handler (DBusGProxy *_proxy, gpointer userdata)
{
  ol_log_func ();
  g_object_unref (_proxy);
  *(void**)userdata = NULL;
  return FALSE;
}

static gboolean
ol_player_banshee_get_music_info (OlMusicInfo *info)
{
  ol_log_func ();
  ol_assert_ret (info != NULL, FALSE);
  GHashTable *data_list = NULL;
  gboolean ret = TRUE;
  GError *error = NULL;
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  if (dbus_g_proxy_call (proxy,
                         GET_TITLE,
                         &error,
                         G_TYPE_INVALID,
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
    ol_debugf ("%s fail: %s\n", GET_TITLE, error->message);
    g_error_free (error);
    error = NULL;
    ret = FALSE;
  }
  return ret;
}

static gboolean
ol_player_banshee_get_music_length (int *len)
{
  ol_log_func ();
  ol_assert_ret (len != NULL, FALSE);
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  if (dbus_g_proxy_call (proxy,
                         DURATION,
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
  ol_assert_ret (played_time != NULL, FALSE);
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  if (ol_dbus_get_uint (proxy,
                        CURRENT_POSITION,
                        (unsigned int*)played_time))
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
  ol_log_func ();
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  return TRUE;
}


static gboolean
ol_player_banshee_init_dbus ()
{
  /* ol_log_func (); */
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
    proxy = dbus_g_proxy_new_for_name_owner (connection, SERVICE, PATH2, INTERFACE2, &error);
    if (proxy == NULL)
    {
      ol_debugf ("get proxy failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
      return FALSE;
    }
    g_signal_connect (proxy, "destroy", G_CALLBACK (ol_player_banshee_proxy_destroy_handler), &proxy);
  }
  if (control_proxy == NULL)
  {
    control_proxy = dbus_g_proxy_new_for_name_owner (connection, SERVICE, PATH, INTERFACE, &error);
    if (control_proxy == NULL)
    {
      ol_debugf ("get proxy failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
      return FALSE;
    }
    g_signal_connect (control_proxy, "destroy", G_CALLBACK (ol_player_banshee_proxy_destroy_handler), &control_proxy);
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
                          GET_STATUS,
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
                         PLAY);
}

static gboolean
ol_player_banshee_pause ()
{
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy,
                         PAUSE);
}

static gboolean
ol_player_banshee_stop ()
{
  if (proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  return ol_dbus_invoke (proxy,
                         STOP);
}

static gboolean
ol_player_banshee_prev ()
{
  if (control_proxy == NULL)
    if (!ol_player_banshee_init_dbus ())
      return FALSE;
  return dbus_g_proxy_call (control_proxy,
                            PREVIOUS,
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
                            NEXT,
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
                            SEEK,
                            NULL,
                            G_TYPE_UINT,
                            (guint) pos_ms,
                            G_TYPE_INVALID,
                            G_TYPE_INVALID);
}

static const char *
ol_player_banshee_get_icon_path ()
{
  int i;
  for (i = 0; i < ol_get_array_len (ICON_PATHS); i++)
  {
    if (ol_path_is_file (ICON_PATHS[i]))
      return ICON_PATHS[i];
  }
  return NULL;
}

struct OlPlayer*
ol_player_banshee_get ()
{
  ol_log_func ();
  struct OlPlayer *controller = ol_player_new ("Banshee");
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
  controller->get_icon_path = ol_player_banshee_get_icon_path;
  return controller;
}
