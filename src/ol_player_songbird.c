#include <stdio.h>
#include <sys/time.h>
#include <dbus/dbus-glib.h>
#include "ol_player_songbird.h"
#include "ol_utils.h"
#include "ol_utils_dbus.h"
#include "ol_player_mpris.h"
#include "ol_debug.h"

static const char *SERVICE = "org.mpris.songbird";
static const char *icon_paths[] = {
  "/usr/share/icons/hicolor/64x64/apps/songbird.png",
  "/usr/local/share/icons/hicolor/64x64/apps/songbird.png",
};

struct timeval begin_time;

static OlPlayerMpris *mpris = NULL;

static OlPlayerMpris* ol_player_songbird_get_mpris ();

static gboolean ol_player_songbird_get_music_info (OlMusicInfo *info);
static gboolean ol_player_songbird_get_played_time (int *played_time);
static gboolean ol_player_songbird_get_music_length (int *len);
static gboolean ol_player_songbird_get_activated (void);
static enum OlPlayerStatus ol_player_songbird_get_status (void);
static int ol_player_songbird_get_capacity (void);
static gboolean ol_player_songbird_play (void);
static gboolean ol_player_songbird_pause (void);
static gboolean ol_player_songbird_stop (void);
static gboolean ol_player_songbird_prev (void);
static gboolean ol_player_songbird_next (void);
static gboolean ol_player_songbird_seek (int pos_ms);
static const char *_get_icon_path (void);

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
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_get_music_info (mpris, info);
}

static gboolean
ol_player_songbird_get_played_time (int *played_time)
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  return ol_player_mpris_get_played_time (mpris, played_time);
}

static gboolean
ol_player_songbird_get_music_length (int *len)
{
  OlPlayerMpris *mpris = ol_player_songbird_get_mpris ();
  *len = 500000;
  return ol_player_mpris_get_music_length (mpris, len);
}

static gboolean
ol_player_songbird_get_activated ()
{
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

static const char *
_get_icon_path ()
{
  int i;
  for (i = 0; i < ol_get_array_len (icon_paths); i++)
  {
    if (ol_path_is_file (icon_paths[i]))
      return icon_paths[i];
  }
  return NULL;
}

struct OlPlayer*
ol_player_songbird_get ()
{
  ol_log_func ();
  struct OlPlayer *controller = ol_player_new ("Songbird");
  ol_player_set_cmd (controller, "songbird");
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
  controller->get_icon_path = _get_icon_path;
  return controller;
}
