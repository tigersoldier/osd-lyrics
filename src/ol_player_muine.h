#ifndef _OL_PLAYER_MUINE_H_
#define _OL_PLAYER_MUINE_H_

#include "ol_player.h"

/** 
 * @brief Creates a controller of Muine
 * 
 * @return The controller of Muine. It's allocated by g_new, so use g_free to free the memory
 */
struct OlPlayer* ol_player_muine_get ();

#endif /* _OL_PLAYER_MUINE_H_ */
