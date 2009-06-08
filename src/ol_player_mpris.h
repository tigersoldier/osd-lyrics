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
  gchar *name;
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

#endif /* _OL_PLAYER_MPRIS_H_ */
