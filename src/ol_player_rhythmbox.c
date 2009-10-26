#include <stdio.h>
#include <sys/time.h>
#include "ol_player_rhythmbox.h"
#include "ol_utils_dbus.h"
#include "ol_utils.h"

static const char service[] = "org.gnome.Rhythmbox";
static const char path_player[] = "/org/gnome/Rhythmbox/Player";
static const char interface_player[] = "org.gnome.Rhythmbox.Player";
static const char path_shell[] = "/org/gnome/Rhythmbox/Shell";
static const char interface_shell[] = "org.gnome.Rhythmbox.Shell";

static DBusGProxy *proxy_player = NULL;
static DBusGProxy *proxy_shell = NULL;
static GError *error = NULL;
static int ol_played_time;

static gboolean ol_player_rhythmbox_get_music_info (OlMusicInfo *info);
static gboolean ol_player_rhythmbox_get_played_time (int *played_time);
static gboolean ol_player_rhythmbox_get_music_length (int *len);
static GHashTable* ol_player_rhythmbox_get_song_properties ();
static gboolean ol_player_rhythmbox_init_dbus ();
static gboolean ol_player_rhythmbox_get_activated ();
static gboolean ol_player_rhythmbox_proxy_destroy_handler ();
static void ol_player_rhythmbox_elapsed_changed (DBusGProxy *player_proxy, int elapsed, gpointer data);
static int ol_player_rhythmbox_get_capacity ();

static int first_time = -1;
static int prev_time = 0;
struct timeval begin_time;

static int
ol_player_rhythmbox_init_timer (int time)
{
  first_time = time;
  prev_time = time;
  gettimeofday (&begin_time, NULL);
}

/** 
 * Return real position in milliseconds
 * Because Rhythmbox return position only in seconds, which means the position
 * is rounded to 1000, so we need to simulate a timer to get proper time
 * in milliseconds
 * @param time rounded time in milliseconds
 * 
 * @return 
 */
static int
ol_player_rhythmbox_get_real_ms (int time)
{
  if (first_time < 0 || prev_time - time > 1000 || time - prev_time > 1000)
  {
    /* reinitialize timer */
    puts ("init1");
    printf ("prev:%d, time:%d\n", prev_time, time);
    ol_player_rhythmbox_init_timer (time);
  }
  else
  {
    struct timeval current_time;
    gettimeofday (&current_time, NULL);
    int real_time = first_time +
      (current_time.tv_sec - begin_time.tv_sec) * 1000 +
      (current_time.tv_usec - begin_time.tv_usec) / 1000;
    if (real_time - time > 1000 || time - real_time > 1000 )
    {
      printf ("real_time: %d, time: %d\n", real_time, time);
      ol_player_rhythmbox_init_timer (time);
    }
    else
    {
      prev_time = time;
      time = real_time;
    }
  }
  return time;
}

static gboolean
ol_player_rhythmbox_proxy_destroy_handler (DBusGProxy *proxy, gboolean shell)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  if (proxy == proxy_shell)
  {
    g_object_unref (proxy_shell);
    proxy_shell = NULL;
  }
  else
  {
    g_object_unref (proxy_player);
    proxy_player = NULL;
  }
  return FALSE;
}

static void
ol_player_rhythmbox_elapsed_changed (DBusGProxy *player_proxy, int elapsed, gpointer data)
{
  ol_played_time = elapsed * 1000;
}

static GHashTable*
ol_player_rhythmbox_get_song_properties ()
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  if (proxy_player == NULL || proxy_shell == NULL)
    if (!ol_player_rhythmbox_init_dbus ())
      return NULL;
  GHashTable *data_list = NULL;;
  char *uri = NULL;
  if (ol_dbus_get_string (proxy_player, "getPlayingUri", &uri))
  {
    if (dbus_g_proxy_call (proxy_shell, "getSongProperties", NULL,
                           G_TYPE_STRING, uri,
                           G_TYPE_INVALID,
                           dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),
                           &data_list,
                           G_TYPE_INVALID))
    {
    }
    else
    {
      printf ("failed %s\n", uri);
      data_list = NULL;
    }
    g_free (uri);
  }
  return data_list;
}

static gboolean
ol_player_rhythmbox_get_music_info (OlMusicInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);
  if (proxy_player == NULL || proxy_shell == NULL)
    if (!ol_player_rhythmbox_init_dbus ())
      return FALSE;
  GHashTable *data_list;
  char *uri = NULL;
  gboolean ret = TRUE;
  if (data_list = ol_player_rhythmbox_get_song_properties ())
  {
    if (info->artist)
      g_free (info->artist);
    info->artist = g_strdup (ol_get_string_from_hash_table (data_list, "artist"));
    if (info->title)
      g_free (info->title);
    info->title = g_strdup (ol_get_string_from_hash_table (data_list, "title"));
    if (info->album)
      g_free (info->album);
    info->album = g_strdup (ol_get_string_from_hash_table (data_list, "album"));
    info->track_number = ol_get_int_from_hash_table (data_list, "track-number");
    g_hash_table_destroy (data_list);
    printf ("%s %s %s %d\n", info->artist, info->title, info->album, info->track_number);
  }
  else
  {
    ret = FALSE;
  }
  return ret;
}

static gboolean
ol_player_rhythmbox_get_played_time (int *played_time)
{
  if (!played_time)
    return FALSE;
  if ((!proxy_player || !proxy_shell) && !ol_player_rhythmbox_init_dbus ())
    return FALSE;
  *played_time = ol_player_rhythmbox_get_real_ms (ol_played_time);
  return TRUE;
}

static gboolean
ol_player_rhythmbox_get_music_length (int *len)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  g_return_val_if_fail (len != NULL, FALSE);
  GHashTable *data_list = ol_player_rhythmbox_get_song_properties ();
  if (data_list == NULL)
    return FALSE;
  *len = ol_get_int_from_hash_table (data_list, "duration");
  g_hash_table_destroy (data_list);
  return TRUE;
}

static gboolean
ol_player_rhythmbox_get_activated ()
{
  if ((!proxy_player || !proxy_shell) && !ol_player_rhythmbox_init_dbus ())
    return FALSE;
  return TRUE;
}

static gboolean
ol_player_rhythmbox_init_dbus ()
{
  printf ("%s\n",
          __FUNCTION__);
  DBusGConnection *connection = ol_dbus_get_connection ();
  if (connection == NULL)
  {
    return FALSE;
  }
  /* if (proxy_player != NULL) */
  /* { */
  /*   g_object_unref (proxy_player); */
  /*   proxy_player = NULL; */
  /* } */
  if (proxy_player == NULL)
  {
    proxy_player = dbus_g_proxy_new_for_name_owner (connection, service, path_player, interface_player, &error);
    if (proxy_player == NULL)
    {
      printf ("get proxy failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
      return FALSE;
    }
    g_signal_connect (proxy_player, "destroy", G_CALLBACK (ol_player_rhythmbox_proxy_destroy_handler), NULL);
    dbus_g_proxy_add_signal(proxy_player, "elapsedChanged",
                            G_TYPE_UINT,
                            G_TYPE_INVALID);
    dbus_g_proxy_connect_signal(proxy_player, "elapsedChanged",
                                G_CALLBACK(ol_player_rhythmbox_elapsed_changed), NULL, NULL);
  }
  if (proxy_shell == NULL)
  {
    proxy_shell = dbus_g_proxy_new_for_name_owner (connection, service, path_shell, interface_shell, &error);
    if (proxy_shell == NULL)
    {
      printf ("get proxy failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
      return FALSE;
    }
    g_signal_connect (proxy_shell, "destroy", G_CALLBACK (ol_player_rhythmbox_proxy_destroy_handler), NULL);
  }
  return TRUE;
}

static int
ol_player_rhythmbox_get_capacity ()
{
  return 0;
}

OlPlayerController*
ol_player_rhythmbox_get_controller ()
{
  printf ("%s\n",
          __FUNCTION__);
  OlPlayerController *controller = g_new0 (OlPlayerController, 1);
  controller->get_music_info = ol_player_rhythmbox_get_music_info;
  controller->get_activated = ol_player_rhythmbox_get_activated;
  controller->get_played_time = ol_player_rhythmbox_get_played_time;
  controller->get_music_length = ol_player_rhythmbox_get_music_length;
  controller->get_capacity = ol_player_get_capacity;
  return controller;
}
