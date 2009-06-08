#include <stdio.h>
#include <sys/time.h>
#include <dbus/dbus-glib.h>
#include "ol_player_amarok2.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
#include "ol_player_mpris.h"

static const char *SERVICE = "org.kde.amarok";

static int first_time = -1;
static int prev_time = 0;
struct timeval begin_time;

static OlPlayerMpris *mpris = NULL;

static OlPlayerMpris* ol_player_amarok2_get_mpris ();
static int ol_player_amarok2_get_real_ms (int time);
static int ol_player_amarok2_init_timer (int time);

static gboolean ol_player_amarok2_get_music_info (OlMusicInfo *info);
static gboolean ol_player_amarok2_get_played_time (int *played_time);
static gboolean ol_player_amarok2_get_music_length (int *len);
static gboolean ol_player_amarok2_get_activated ();

static OlPlayerMpris*
ol_player_amarok2_get_mpris ()
{
  if (mpris == NULL)
  {
    mpris = ol_player_mpris_new (SERVICE);
  }
  return mpris;
}

static int
ol_player_amarok2_init_timer (int time)
{
  first_time = time;
  prev_time = time;
  gettimeofday (&begin_time, NULL);
}

/** 
 * Return real position in milliseconds
 * Because Amarok2 return position only in seconds, which means the position
 * is rounded to 1000, so we need to simulate a timer to get proper time
 * in milliseconds
 * @param time 
 * 
 * @return 
 */
static int
ol_player_amarok2_get_real_ms (int time)
{
  if (first_time < 0 || prev_time - time > 1000 || time - prev_time > 1000)
  {
    /* reinitialize timer */
    puts ("init1");
    printf ("prev:%d, time:%d\n", prev_time, time);
    ol_player_amarok2_init_timer (time);
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
      puts ("init2");
      printf ("real_time: %d, time: %d\n", real_time, time);
      ol_player_amarok2_init_timer (time);
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
ol_player_amarok2_get_music_info (OlMusicInfo *info)
{
  OlPlayerMpris *mpris = ol_player_amarok2_get_mpris ();
  return ol_player_mpris_get_music_info (mpris, info);
}

static gboolean
ol_player_amarok2_get_played_time (int *played_time)
{
  OlPlayerMpris *mpris = ol_player_amarok2_get_mpris ();
  if (!ol_player_mpris_get_played_time (mpris, played_time))
    return FALSE;
  *played_time = ol_player_amarok2_get_real_ms (*played_time);
  return TRUE;
}

static gboolean
ol_player_amarok2_get_music_length (int *len)
{
  OlPlayerMpris *mpris = ol_player_amarok2_get_mpris ();
  return ol_player_mpris_get_music_length (mpris, len);
}

static gboolean
ol_player_amarok2_get_activated ()
{
  OlPlayerMpris *mpris = ol_player_amarok2_get_mpris ();
  return ol_player_mpris_get_activated (mpris);
}

OlPlayerController*
ol_player_amarok2_get_controller ()
{
  printf ("%s\n",
          __FUNCTION__);
  OlPlayerController *controller = g_new (OlPlayerController, 1);
  controller->get_music_info = ol_player_amarok2_get_music_info;
  controller->get_activated = ol_player_amarok2_get_activated;
  controller->get_played_time = ol_player_amarok2_get_played_time;
  controller->get_music_length = ol_player_amarok2_get_music_length;
  return controller;
}
