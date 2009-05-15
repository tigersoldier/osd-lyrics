#ifndef __OL_PLAYER_H__
#define __OL_PLAYER_H__

#include <glib.h>
#include "ol_music_info.h"

typedef struct
{
  gboolean (*get_music_info) (OlMusicInfo *info);
  gboolean (*get_activated) ();
  gboolean (*get_played_time) (int *played_time);
  gboolean (*get_music_length) (int *len);
  void (*free) ();
} OlPlayerController;

/** 
 * @brief Registers a player controller
 * A player controller should be registered so that we can get information from this player.
 * @param controller The player controller to be registered
 * @param name The name of the player
 */
void ol_player_register_controller (OlPlayerController *controller, const gchar *name);

/** 
 * @brief Gets the controller of the player available
 * 
 * @return A pointer to the controller of the player. If there is not an active player, NULL will be returned
 */
OlPlayerController* ol_player_get_active_player ();

/** 
 * @brief Initialize all the player controllers, to register the controllers available.
 * This should be called before ol_get_active_player
 */
void ol_player_init ();

/** 
 * @brief Frees all the player controllers
 * This should be called after the the program exits
 */
void ol_player_free ();

#endif // __OL_PLAYER_H__
