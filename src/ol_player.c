#include <stdio.h>
#include "config.h"
#include "ol_player.h"
#include "ol_debug.h"
#include "ol_player_banshee.h"
#include "ol_player_exaile02.h"
#include "ol_player_exaile03.h"
#ifdef ENABLE_AMAROK1
#include "ol_player_amarok1.h"
#endif  /* ENABLE_AMAROK1 */
#include "ol_player_amarok2.h"
#include "ol_player_audacious.h"
#include "ol_player_songbird.h"
#ifdef ENABLE_XMMS2
#include "ol_player_xmms2.h"
#endif  /* ENABLE_XMMS2 */
#include "ol_player_rhythmbox.h"
#ifdef ENABLE_MPD
#include "ol_player_mpd.h"
#endif  /* ENABLE_MPD */
#include "ol_player_moc.h"
#include "ol_player_quodlibet.h"

static GArray *controllers = NULL;

void
ol_player_init ()
{
  if (controllers == NULL)
  {
    controllers = g_array_new (FALSE, TRUE, sizeof (OlPlayerController*));
#ifdef ENABLE_AMAROK1
    ol_player_register_controller (ol_player_amarok1_get_controller ());
#endif
    ol_player_register_controller (ol_player_amarok2_get_controller ());
    ol_player_register_controller (ol_player_banshee_get_controller ());
    ol_player_register_controller (ol_player_exaile02_get_controller ());
    ol_player_register_controller (ol_player_exaile03_get_controller ());
    ol_player_register_controller (ol_player_audacious_get_controller ());
    ol_player_register_controller (ol_player_songbird_get_controller ()); 
#ifdef ENABLE_XMMS2
    ol_player_register_controller (ol_player_xmms2_get_controller ());
#endif  /* ENABLE_XMMS2 */
    ol_player_register_controller (ol_player_rhythmbox_get_controller ());
#ifdef ENABLE_MPD
    ol_player_register_controller (ol_player_mpd_get_controller ());
#endif  /* ENABLE_MPD */
    ol_player_register_controller (ol_player_moc_get_controller ());
    ol_player_register_controller (ol_player_quodlibet_get_controller ());
  }
}

void
ol_player_unload ()
{
  if (controllers != NULL)
  {
    g_array_free (controllers, TRUE);
    controllers = NULL;
  }
}

OlPlayerController **
ol_player_get_controllers ()
{
  OlPlayerController **players = g_new0 (OlPlayerController *,
                                         controllers->len + 1);
  int i;
  for (i = 0; i < controllers->len; i++)
  {
    players[i] = g_array_index (controllers, OlPlayerController*, i);
  }
  return players;
}

OlPlayerController*
ol_player_get_active_player ()
{
  ol_log_func ();
  if (controllers == NULL)
  {
    return NULL;
  }
  int i;
  ol_debugf ("controller count:%d\n", controllers->len);
  for (i = 0; i < controllers->len; i++)
  {
    OlPlayerController *controller = g_array_index (controllers, OlPlayerController*, i);
    ol_debugf ("trying player %d\n", i);
    if (controller && controller->get_activated ())
    {
      return controller;
    }
  }
  return NULL;
}

void
ol_player_register_controller (OlPlayerController *controller)
{
  ol_assert (controller != NULL);
  ol_assert (ol_player_get_name (controller) != NULL);
  /* controller->get_activated (); */
  g_array_append_val (controllers, controller);
}

gboolean
ol_player_get_music_info (OlPlayerController *player, OlMusicInfo *info)
{
  if (player == NULL)
    return FALSE;
  return player->get_music_info (info);
}

gboolean
ol_player_get_activated (OlPlayerController *player)
{
  if (player == NULL)
    return FALSE;
  return player->get_activated ();
}

gboolean
ol_player_get_played_time (OlPlayerController *player, int *played_time)
{
  if (player == NULL)
    return FALSE;
  return player->get_played_time (played_time);
}

gboolean
ol_player_get_music_length (OlPlayerController *player, int *len)
{
  if (player == NULL)
    return FALSE;
  return player->get_music_length (len);
}

enum OlPlayerStatus
ol_player_get_status (OlPlayerController *player)
{
  if (player == NULL)
    return OL_PLAYER_ERROR;
  if (player->get_status == NULL)
    return OL_PLAYER_ERROR;
  return player->get_status ();
}

int
ol_player_get_capacity (OlPlayerController *player)
{
  if (player == NULL)
    return -1;
  if (player->get_capacity == NULL)
    return -1;
  return player->get_capacity ();
}

gboolean
ol_player_play (OlPlayerController *player)
{
  if (player == NULL)
    return FALSE;
  if (player->play == NULL)
    return FALSE;
  return player->play ();
}

gboolean
ol_player_prev (OlPlayerController *player)
{
  if (player == NULL)
    return FALSE;
  if (player->prev == NULL)
    return FALSE;
  return player->prev ();
}

gboolean
ol_player_next (OlPlayerController *player)
{
  if (player == NULL)
    return FALSE;
  if (player->next == NULL)
    return FALSE;
  return player->next ();
}

gboolean
ol_player_seek (OlPlayerController *player, int pos_ms)
{
  if (player == NULL)
    return FALSE;
  if (player->seek == NULL)
    return FALSE;
  return player->seek (pos_ms);
}

gboolean
ol_player_stop (OlPlayerController *player)
{
  if (player == NULL)
    return FALSE;
  if (player->stop == NULL)
    return FALSE;
  return player->stop ();
}

gboolean
ol_player_pause (OlPlayerController *player)
{
  if (player == NULL)
    return FALSE;
  if (player->pause == NULL)
    return FALSE;
  return player->pause ();
}

gboolean
ol_player_play_pause (OlPlayerController *player)
{
  if (player == NULL)
    return FALSE;
  if (player->get_status == NULL ||
      player->play == NULL ||
      player->pause == NULL)
    return FALSE;
  enum OlPlayerStatus status = player->get_status ();
  switch (status)
  {
  case OL_PLAYER_PLAYING:
    return player->pause ();
    break;
  case OL_PLAYER_PAUSED:
  case OL_PLAYER_STOPPED:
    return player->play ();
    break;
  default:
    return FALSE;
  }
}

OlPlayerController *
ol_player_new (const char *name)
{
  ol_assert_ret (name != NULL, NULL);
  OlPlayerController *player = g_new0 (OlPlayerController, 1);
  player->name = name;
  return player;
}

void
ol_player_free (OlPlayerController *player)
{
  ol_assert (player != NULL);
  if (player->cmdline != NULL)
    ol_error ("cmdline is not NULL, this may cause memory leak");
  g_free (player);
}

const char *
ol_player_set_cmd (OlPlayerController *player,
                              const char *cmd)
{
  ol_assert_ret (player != NULL, NULL);
  const char *old_cmd = player->cmdline;
  player->cmdline = cmd;
  return old_cmd;
}

const char *
ol_player_get_cmd (OlPlayerController *player)
{
  ol_assert_ret (player != NULL, NULL);
  return player->cmdline;
}

const char *
ol_player_get_name (OlPlayerController *player)
{
  ol_assert_ret (player != NULL, NULL);
  return player->name;
}
