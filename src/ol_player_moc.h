#ifndef _OL_PLAYER_MOC_H_
#define _OL_PLAYER_MOC_H_

#include "ol_player.h"

/** 
 * @brief Creates a controller of MOC
 * 
 * @return The controller of MOC. It's allocated by g_new, so use g_free to free the memory
 */
OlPlayerController* ol_player_moc_get_controller ();


#endif /* _OL_PLAYER_MOC_H_ */
