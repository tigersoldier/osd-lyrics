#ifndef __OL_PLAYER_BANSHEE_H__
#define __OL_PLAYER_BANSHEE_H__

#include "ol_player.h"

/** 
 * @brief Creates a controller of Banshee
 * 
 * @return The controller of Banshee. It's allocated by g_new, so use g_free to free the memory
 */
struct OlPlayer* ol_player_banshee_get ();

#endif // __OL_PLAYER_BANSHEE_H__
