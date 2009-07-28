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
  fprintf (stderr, "%s\n", __FUNCTION__);
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_get_music_info (mpris, info);
}

static gboolean
ol_player_songbird_get_played_time (int *played_time)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  /* if (!ol_player_mpris_get_played_time (mpris, played_time)) */
  /*   return FALSE; */
  /* *played_time = ol_player_songbird_get_real_ms (*played_time); */
  /* return TRUE; */
  return ol_player_mpris_get_played_time (mpris, played_time);
}

static gboolean
ol_player_songbird_get_music_length (int *len)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_get_music_length (mpris, len);
}

static gboolean
ol_player_songbird_get_activated ()
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_get_activated (mpris);
}

OlPlayerController*
ol_player_songbird_get_controller ()
{
  printf ("%s\n",
          __FUNCTION__);
  OlPlayerController *controller = g_new (OlPlayerController, 1);
  controller->get_music_info = ol_player_songbird_get_music_info;
  controller->get_activated = ol_player_songbird_get_activated;
  controller->get_played_time = ol_player_songbird_get_played_time;
  controller->get_music_length = ol_player_songbird_get_music_length;
  return controller;
}
