#include <stdio.h>
#include <sys/time.h>
#include <dbus/dbus-glib.h>
#include "ol_player_songbird.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
#include "ol_player_mpris.h"

static const char *SERVICE = "org.mpris.songbird";

static int first_time = -1;
static int prev_time = 0;
struct timeval begin_time;

static OlPlayerMpris *mpris = NULL;

static OlPlayerMpris* ol_player_songbird_get_mpris ();

static gboolean ol_player_songbird_get_music_info (OlMusicInfo *info);
static gboolean ol_player_songbird_get_played_time (int *played_time);
static gboolean ol_player_songbird_get_music_length (int *len);
static gboolean ol_player_songbird_get_activated ();
static enum OlPlayerStatus ol_player_songbird_get_status ();
static int ol_player_songbird_get_capacity ();
static gboolean ol_player_songbird_play ();
static gboolean ol_player_songbird_pause ();
static gboolean ol_player_songbird_stop ();
static gboolean ol_player_songbird_prev ();
static gboolean ol_player_songbird_next ();
static gboolean ol_player_songbird_seek (int pos_ms);

static OlPlayerMpris*
ol_player_songbird_get_mpris ()
{
  if (mpris == NULL)
  {
    mpris = ol_player_mpris_new (SERVICE);
  }
  return mpris;
}

static gboolean
ol_player_songbird_get_music_info (OlMusicInfo *info)
{
  /* fprintf (stderr, "%s\n", __FUNCTION__); */
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_get_music_info (mpris, info);
}

static gboolean
ol_player_songbird_get_played_time (int *played_time)
{
  /* fprintf (stderr, "%s\n", __FUNCTION__); */
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_get_played_time (mpris, played_time);
}

static gboolean
ol_player_songbird_get_music_length (int *len)
{
  /* fprintf (stderr, "%s\n", __FUNCTION__); */
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  *len = 500000;
  return ol_player_mpris_get_music_length (mpris, len);
}

static gboolean
ol_player_songbird_get_activated ()
{
  /* fprintf (stderr, "%s\n", __FUNCTION__); */
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_get_activated (mpris);
}

static enum OlPlayerStatus
ol_player_songbird_get_status ()
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_get_status (mpris);
}

static int
ol_player_songbird_get_capacity ()
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_get_capacity (mpris);
}

static gboolean
ol_player_songbird_play ()
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_play (mpris);
}

static gboolean
ol_player_songbird_pause ()
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_pause (mpris);
}

static gboolean
ol_player_songbird_stop ()
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_stop (mpris);
}

static gboolean
ol_player_songbird_prev ()
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_prev (mpris);
}

static gboolean
ol_player_songbird_next ()
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_next (mpris);
}

static gboolean
ol_player_songbird_seek (int pos_ms)
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_seek (mpris, pos_ms);
}

OlPlayerController*
ol_player_songbird_get_controller ()
{
  fprintf (stderr, "%s\n",
           __FUNCTION__);
  OlPlayerController *controller = g_new0 (OlPlayerController, 1);
  controller->get_music_info = ol_player_songbird_get_music_info;
  controller->get_activated = ol_player_songbird_get_activated;
  controller->get_played_time = ol_player_songbird_get_played_time;
  controller->get_music_length = ol_player_songbird_get_music_length;
  controller->get_status = ol_player_songbird_get_status;
  controller->get_capacity = ol_player_songbird_get_capacity;
  controller->play = ol_player_songbird_play;
  controller->pause = ol_player_songbird_pause;
  controller->stop = ol_player_songbird_stop;
  controller->prev = ol_player_songbird_prev;
  controller->next = ol_player_songbird_next;
  controller->seek = ol_player_songbird_seek;
  return controller;
}
