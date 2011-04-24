
/**
 * @file   ol_player_mpris.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Sun Jun  7 16:53:56 2009
 *
 * @brief  Supports all players that provides MPRIS interface
 *
 */
#ifndef _OL_PLAYER_MPRIS_H_
#define _OL_PLAYER_MPRIS_H_

#include "ol_music_info.h"

typedef struct
{
  DBusGProxy *proxy;
  DBusGProxyCall *call_id;
  DBusGProxyCall *metadata_call_id;
  DBusGProxyCall *status_call_id;
  enum OlPlayerStatus status;
  gchar *name;
  int played_time;
  gchar *title;
  gchar *artist;
  gchar *album;
  gchar *uri;
  int track_number;
  int music_len;
} OlPlayerMpris;

/**
 * @brief Creates a new MPRIS Player context
 *
 * @param service The service name of the player on dbus
 *
 * @return The created MPRIS Player context, should be destroyed by g_free
 */
OlPlayerMpris* ol_player_mpris_new (const char *service);
gboolean ol_player_mpris_get_music_info (OlPlayerMpris *mpris, OlMusicInfo *info);
gboolean ol_player_mpris_get_played_time (OlPlayerMpris *mpris, int *played_time);
gboolean ol_player_mpris_get_music_length (OlPlayerMpris *mpris, int *len);
gboolean ol_player_mpris_get_activated (OlPlayerMpris *mpris);
int ol_player_mpris_get_capacity (OlPlayerMpris *mpris);
enum OlPlayerStatus ol_player_mpris_get_status (OlPlayerMpris *mpris);
gboolean ol_player_mpris_play (OlPlayerMpris *mpris);
gboolean ol_player_mpris_pause (OlPlayerMpris *mpris);
gboolean ol_player_mpris_stop (OlPlayerMpris *mpris);
gboolean ol_player_mpris_prev (OlPlayerMpris *mpris);
gboolean ol_player_mpris_next (OlPlayerMpris *mpris);
gboolean ol_player_mpris_seek (OlPlayerMpris *mpris, int pos_ms);
#endif /* _OL_PLAYER_MPRIS_H_ */
