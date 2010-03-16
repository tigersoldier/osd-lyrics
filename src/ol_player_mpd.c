#include "config.h"
#ifdef ENABLE_MPD
#include <stdio.h>
#include <stdlib.h>
#include <libmpd/libmpd.h>

#include "ol_player_mpd.h"
#include "ol_player.h"
#include "ol_elapse_emulator.h"
#include "ol_config.h"
#include "ol_utils.h"
#include "ol_debug.h"

static char *DEFAULT_HOSTNAME = "localhost";
static const int DEFAULT_PORT = 6600;
static char *hostname = NULL;
static int port;
static MpdObj *mpd = NULL;
static OlElapseEmulator *elapse_emulator = NULL;

static gboolean ol_player_mpd_get_music_info (OlMusicInfo *info);
static gboolean ol_player_mpd_get_played_time (int *played_time);
static gboolean ol_player_mpd_get_music_length (int *len);
static gboolean ol_player_mpd_ensure_connection ();
static gboolean ol_player_mpd_get_activated ();
static gboolean ol_player_mpd_proxy_destroy_handler (gpointer userdata);
static enum OlPlayerStatus ol_player_mpd_get_status ();
static int ol_player_mpd_get_capacity ();
static gboolean ol_player_mpd_play ();
static gboolean ol_player_mpd_pause ();
static gboolean ol_player_mpd_stop ();
static gboolean ol_player_mpd_prev ();
static gboolean ol_player_mpd_next ();
static gboolean ol_player_mpd_seek (int pos_ms);
static gboolean ol_player_mpd_init ();
static void config_change_handler (OlConfig *config, gchar *group, gchar *name, gpointer userdata);

static gboolean
ol_player_mpd_get_music_info (OlMusicInfo *info)
{
  ol_log_func ();
  ol_assert_ret (info != NULL, FALSE);
  if (!ol_player_mpd_ensure_connection ())
    return FALSE;
  mpd_status_update (mpd);
  mpd_Song *song = mpd_playlist_get_current_song (mpd);
  if (song)
  {
    ol_music_info_clear (info);
    if (song->title)
      info->title = g_strdup (song->title);
    if (song->artist)
      info->artist = g_strdup (song->artist);
    if (song->album)
      info->album = g_strdup (song->album);
    /* TODO: get uri from file infomation */
    /* if (song->file) */
    /*   info->uri = g_strdup (song->file); */
    if (song->track)
      sscanf (song->track, "%d", &info->track_number);
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
  }
  return TRUE;
}

static gboolean
ol_player_mpd_get_music_length (int *len)
{
  ol_log_func ();
  ol_assert_ret (len != NULL, FALSE);
  if (!ol_player_mpd_ensure_connection ())
    return FALSE;
  mpd_status_update (mpd);
  *len = mpd_status_get_total_song_time (mpd) * 1000;
  ol_debugf ("  length = %dms\n", *len);
  return TRUE;
}

static void
ol_player_mpd_ensure_elapse (int elapsed_time)
{
  if (elapse_emulator == NULL)
  {
    elapse_emulator = g_new (OlElapseEmulator, 1);
    if (elapse_emulator != NULL)
      ol_elapse_emulator_init (elapse_emulator, elapsed_time, 1000);
  }
}

static gboolean
ol_player_mpd_get_played_time (int *played_time)
{
  /* ol_log_func (); */
  ol_assert_ret (played_time != NULL, FALSE);
  if (!ol_player_mpd_ensure_connection ())
    return FALSE;
  mpd_status_update (mpd);
  int mpd_time = (mpd_status_get_elapsed_song_time (mpd) - 1) * 1000;
  ol_player_mpd_ensure_elapse (mpd_time);
  enum OlPlayerStatus status = ol_player_mpd_get_status ();
  if (status == OL_PLAYER_PLAYING)
    *played_time = ol_elapse_emulator_get_real_ms (elapse_emulator,
                                                   mpd_time);
  else if (status == OL_PLAYER_PAUSED)
    *played_time = ol_elapse_emulator_get_last_ms (elapse_emulator,
                                                   mpd_time);
  else
    *played_time = -1;
  return TRUE;
}

static gboolean
ol_player_mpd_get_activated ()
{
  return ol_player_mpd_ensure_connection ();
}

static gboolean
ol_player_mpd_ensure_connection ()
{
  /* ol_log_func (); */
  if (mpd == NULL)
  {
    if (!ol_player_mpd_init ())
      return FALSE;
  }
  if (mpd_check_connected (mpd))
    return TRUE;
  int result = mpd_connect (mpd);
  ol_debugf ("  connect result: %d\n", result);
  return result == MPD_OK;
}

static gboolean
ol_player_mpd_init ()
{
  OlConfig *config = ol_config_get_instance ();
  hostname = ol_config_get_string (config, "Player", "mpd-hostname");
  if (hostname == NULL)
    hostname = g_strdup (DEFAULT_HOSTNAME);
  port = ol_config_get_int (config, "Player", "mpd-port");
  if (port == 0)
    port = DEFAULT_PORT;
  mpd = mpd_new (hostname, port, NULL);
  if (mpd == NULL)
  {
    ol_error ("Create MPD Object failed");
    return FALSE;
  }
  mpd_set_connection_timeout (mpd, 0.1);
  /* TODO: connect signals here */
  return TRUE;
}

static enum
OlPlayerStatus ol_player_mpd_get_status ()
{
  /* ol_log_func (); */
  if (!ol_player_mpd_ensure_connection ())
    return OL_PLAYER_ERROR;
  int status = mpd_player_get_state (mpd);
  switch (status)
  {
  case MPD_PLAYER_PAUSE:
    return OL_PLAYER_PAUSED;
  case MPD_PLAYER_PLAY:
    return OL_PLAYER_PLAYING;
  case MPD_PLAYER_STOP:
    return OL_PLAYER_STOPPED;
  case MPD_PLAYER_UNKNOWN:
  default:
    return OL_PLAYER_UNKNOWN;
  }
}

static int
ol_player_mpd_get_capacity ()
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    OL_PLAYER_STOP | OL_PLAYER_PREV | OL_PLAYER_NEXT | OL_PLAYER_SEEK;
}

static gboolean
ol_player_mpd_play ()
{
  ol_log_func ();
  if (!ol_player_mpd_ensure_connection ())
    return FALSE;
  mpd_player_play (mpd);
  return TRUE;
}

static gboolean
ol_player_mpd_pause ()
{
  ol_log_func ();
  if (!ol_player_mpd_ensure_connection ())
    return FALSE;
  mpd_player_pause (mpd);
  return TRUE;
}

static gboolean
ol_player_mpd_stop ()
{
  ol_log_func ();
  if (!ol_player_mpd_ensure_connection ())
    return FALSE;
  mpd_player_stop (mpd);
  return TRUE;
}

static gboolean
ol_player_mpd_prev ()
{
  ol_log_func ();
  if (!ol_player_mpd_ensure_connection ())
    return FALSE;
  mpd_player_prev (mpd);
  return TRUE;
}

static gboolean
ol_player_mpd_next ()
{
  ol_log_func ();
  if (!ol_player_mpd_ensure_connection ())
    return FALSE;
  mpd_player_next (mpd);
  return TRUE;
}

static gboolean
ol_player_mpd_seek (int pos_ms)
{
  ol_log_func ();
  if (!ol_player_mpd_ensure_connection ())
    return FALSE;
  mpd_player_seek (mpd, pos_ms / 1000);
  return TRUE;
}

static void
config_change_handler (OlConfig *config, gchar *group, gchar *name, gpointer userdata)
{
  ol_log_func ();
  ol_debugf ("  [%s]%s\n", group, name);
  if (strcmp (name, "mpd-hostname") == 0)
  {
    ol_debugf ("  mpd-hostname: %s\n", ol_config_get_string (config, group, name));
    if (hostname != NULL)
      g_free (hostname);
    hostname = ol_config_get_string (config, group, name);
    if (hostname == NULL)
      hostname = g_strdup (DEFAULT_HOSTNAME);
    if (mpd != NULL)
      mpd_set_hostname (mpd, hostname);
  }
  if (strcmp (name, "mpd-port") == 0)
  {
    ol_debugf ("  mpd-port: %d\n", ol_config_get_int (config, group, name));
    port = ol_config_get_int (config, group, name);
    if (port == 0)
      port = DEFAULT_PORT;
    if (mpd != NULL)
      mpd_set_port (mpd, port);
  }
}

struct OlPlayer*
ol_player_mpd_get ()
{
  ol_log_func ();
  struct OlPlayer *controller = ol_player_new ("MPD");
  ol_player_set_cmd (controller, "gksu /etc/init.d/mpd start");
  controller->get_music_info = ol_player_mpd_get_music_info;
  controller->get_activated = ol_player_mpd_get_activated;
  controller->get_played_time = ol_player_mpd_get_played_time;
  controller->get_music_length = ol_player_mpd_get_music_length;
  controller->get_capacity = ol_player_mpd_get_capacity;
  controller->get_status = ol_player_mpd_get_status;
  controller->play = ol_player_mpd_play;
  controller->pause = ol_player_mpd_pause;
  controller->stop = ol_player_mpd_stop;
  controller->prev = ol_player_mpd_prev;
  controller->next = ol_player_mpd_next;
  controller->seek = ol_player_mpd_seek;
  return controller;
}
#endif  /* ENABLE_MPD */
