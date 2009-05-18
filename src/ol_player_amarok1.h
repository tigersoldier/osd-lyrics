/**
 * @file   ol_player_amarok1.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Mon May 18 13:55:29 2009
 * 
 * @brief  Provides support for Amarok1.4
 * 
 * 
 */
#ifndef _OL_PLAYER_AMAROK1_H_
#define _OL_PLAYER_AMAROK1_H_

#include "ol_player.h"

/** 
 * @brief Creates a controller of AmarOK1.4
 * 
 * @return The controller of AmarOK1.4. It's allocated by g_new, so use g_free to free the memory
 */
OlPlayerController* ol_player_amarok1_get_controller ();


#endif /* _OL_PLAYER_AMAROK1_H_ */
