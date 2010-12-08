
#include <stdio.h>
#include "config.h"
#include "ol_player.h"
#include "ol_debug.h"
#include "ol_player_banshee.h"
#include "ol_player_exaile02.h"
#include "ol_player_exaile03.h"
#include "ol_player_listen.h"
#include "ol_player_clementine.h"
#include "ol_player_guayadeque02.h"
#include "ol_player_deciber.h"
#include "ol_player_gmusicbrowser.h"
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
#include "ol_player_qmmp.h"
#include "ol_player_juk.h"
#include "ol_player_muine.h"


static GArray *players = NULL;

void
ol_player_init ()
{
  if (players == NULL)
  {
    players = g_array_new (FALSE, TRUE, sizeof (struct OlPlayer*));
#ifdef ENABLE_AMAROK1
    ol_player_register (ol_player_amarok1_get ());
#endif
    ol_player_register (ol_player_amarok2_get ());
    ol_player_register (ol_player_banshee_get ());
    ol_player_register (ol_player_exaile02_get ());
    ol_player_register (ol_player_exaile03_get ());
    ol_player_register (ol_player_audacious_get ());
    ol_player_register (ol_player_songbird_get ());
    ol_player_register (ol_player_clementine_get ());
    ol_player_register (ol_player_listen_get ());
    ol_player_register (ol_player_guayadeque02_get ());
    ol_player_register (ol_player_deciber_get ());
    ol_player_register (ol_player_gmusicbrowser_get ());
#ifdef ENABLE_XMMS2
    ol_player_register (ol_player_xmms2_get ());
#endif  /* ENABLE_XMMS2 */
    ol_player_register (ol_player_rhythmbox_get ());
#ifdef ENABLE_MPD
    ol_player_register (ol_player_mpd_get ());
#endif  /* ENABLE_MPD */
    ol_player_register (ol_player_moc_get ());
    ol_player_register (ol_player_quodlibet_get ());
    ol_player_register (ol_player_qmmp_get ());
    ol_player_register (ol_player_juk_get ());
    ol_player_register (ol_player_muine_get ());


  }
}

void
ol_player_unload ()
{
  if (players != NULL)
  {
    g_array_free (players, TRUE);
    players = NULL;
  }
}

struct OlPlayer **
ol_player_get_players ()
{
  struct OlPlayer **ret = g_new0 (struct OlPlayer *,
                                  players->len + 1);
  int i;
  for (i = 0; i < players->len; i++)
  {
    ret[i] = g_array_index (players, struct OlPlayer*, i);
  }
  return ret;
}

struct OlPlayer*
ol_player_get_active_player ()
{
  ol_log_func ();
  if (players == NULL)
  {
    return NULL;
  }
  int i;
  ol_debugf ("controller count:%d\n", players->len);
  for (i = 0; i < players->len; i++)
  {
    struct OlPlayer *controller = g_array_index (players, struct OlPlayer*, i);
    ol_debugf ("trying %s\n", controller->name);
    if (controller && controller->get_activated ())
    {
      return controller;
    }
  }
  return NULL;
}

void
ol_player_register (struct OlPlayer *controller)
{
  ol_assert (controller != NULL);
  ol_assert (ol_player_get_name (controller) != NULL);
  /* controller->get_activated (); */
  g_array_append_val (players, controller);
}

gboolean
ol_player_get_music_info (struct OlPlayer *player, OlMusicInfo *info)
{
  if (player == NULL)
    return FALSE;
  gboolean s =player->get_music_info (info);
  /*ol_debugf ("title:%s\n", info->title);
  ol_debugf ("artist:%s\n", info->artist);
  ol_debugf ("album:%s\n", info->album);
  ol_debugf ("track_number:%d\n", info->track_number);
  ol_debugf ("uri:%s\n", info->uri);*/

  return s;
}

gboolean
ol_player_get_activated (struct OlPlayer *player)
{
  if (player == NULL)
    return FALSE;
  return player->get_activated ();
}

gboolean
ol_player_get_played_time (struct OlPlayer *player, int *played_time)
{
  if (player == NULL)
    return FALSE;
  gboolean s = player->get_played_time (played_time);
  ol_debugf ("played-time:%d\n", *played_time);
  return s;
}

gboolean
ol_player_get_music_length (struct OlPlayer *player, int *len)
{
  if (player == NULL)
    return FALSE;
  return player->get_music_length (len);
}

enum OlPlayerStatus
ol_player_get_status (struct OlPlayer *player)
{
  if (player == NULL)
    return OL_PLAYER_ERROR;
  if (player->get_status == NULL)
    return OL_PLAYER_ERROR;
  ol_debugf ("status:%d\n",player->get_status());
  return player->get_status ();
}

int
ol_player_get_capacity (struct OlPlayer *player)
    
{
  if (player == NULL)
    return -1;
  if (player->get_capacity == NULL)
    return -1;
  return player->get_capacity ();
}

gboolean
ol_player_play (struct OlPlayer *player)
{
  if (player == NULL)
    return FALSE;
  if (player->play == NULL)
    return FALSE;
  return player->play ();
}

gboolean
ol_player_prev (struct OlPlayer *player)
{
  if (player == NULL)
    return FALSE;
  if (player->prev == NULL)
    return FALSE;
  return player->prev ();
}

gboolean
ol_player_next (struct OlPlayer *player)
{
  if (player == NULL)
    return FALSE;
  if (player->next == NULL)
    return FALSE;
  return player->next ();
}

gboolean
ol_player_seek (struct OlPlayer *player, int pos_ms)
{
  if (player == NULL)
    return FALSE;
  if (player->seek == NULL)
    return FALSE;
  return player->seek (pos_ms);
}

gboolean
ol_player_stop (struct OlPlayer *player)
{
  if (player == NULL)
    return FALSE;
  if (player->stop == NULL)
    return FALSE;
  return player->stop ();
}

gboolean
ol_player_pause (struct OlPlayer *player)
{
  if (player == NULL)
    return FALSE;
  if (player->pause == NULL)
    return FALSE;
  return player->pause ();
}

gboolean
ol_player_play_pause (struct OlPlayer *player)
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

struct OlPlayer *
ol_player_new (const char *name)
{
  ol_assert_ret (name != NULL, NULL);
  struct OlPlayer *player = g_new0 (struct OlPlayer, 1);
  player->name = name;
  return player;
}

void
ol_player_free (struct OlPlayer *player)
{
  ol_assert (player != NULL);
  if (player->cmdline != NULL)
    ol_error ("cmdline is not NULL, this may cause memory leak");
  g_free (player);
}

const char *
ol_player_set_cmd (struct OlPlayer *player,
                              const char *cmd)
{
  ol_assert_ret (player != NULL, NULL);
  const char *old_cmd = player->cmdline;
  player->cmdline = cmd;
  return old_cmd;
}

const char *
ol_player_get_cmd (struct OlPlayer *player)
{
  ol_assert_ret (player != NULL, NULL);
  return player->cmdline;
}

const char *
ol_player_get_name (struct OlPlayer *player)
{
  ol_assert_ret (player != NULL, NULL);
  return player->name;
}

const char *
ol_player_get_icon_path (struct OlPlayer *player)
{
  if (player->get_icon_path != NULL)
    return player->get_icon_path ();
  else
    return NULL;
}
