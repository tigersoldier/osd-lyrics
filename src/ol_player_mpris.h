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

typedef struct
{
} OlPlayerMprisContext;

static DBusGProxy* ol_player_mpris_get_proxy (const char *service);
static gboolean ol_player_mpris_get_music_info (DBusGProxy *proxy, OlMusicInfo *info);
static gboolean ol_player_mpris_get_played_time (DBusGProxy *proxy, int *played_time);
static gboolean ol_player_mpris_get_music_length (DBusGProxy *proxy, int *len);
static gboolean ol_player_mpris_get_activated ();

#endif /* _OL_PLAYER_MPRIS_H_ */
