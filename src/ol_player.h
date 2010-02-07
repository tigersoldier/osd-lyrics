#ifndef __OL_PLAYER_H__
#define __OL_PLAYER_H__

#include <glib.h>
#include "ol_music_info.h"

enum OlPlayerStatus{
  OL_PLAYER_PLAYING = 0,
  OL_PLAYER_PAUSED,
  OL_PLAYER_STOPPED,
  OL_PLAYER_UNKNOWN,
  OL_PLAYER_ERROR,
};

enum OlPlayerCapacity {
  OL_PLAYER_URI =         1 << 0,
  OL_PLAYER_CONTROL =     1 << 1,
  OL_PLAYER_STATUS =      1 << 2,
  OL_PLAYER_PLAY =        1 << 3,
  OL_PLAYER_PREV =        1 << 4,
  OL_PLAYER_NEXT =        1 << 5,
  OL_PLAYER_SEEK =        1 << 6,
  OL_PLAYER_STOP =        1 << 7,
  OL_PLAYER_PAUSE =       1 << 8,
  OL_PLAYER_PLAY_PAUSE =  OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE,
};

typedef struct
{
  const char * name;
  const char * cmdline;
  gboolean (*get_music_info) (OlMusicInfo *info);
  gboolean (*get_activated) ();
  gboolean (*get_played_time) (int *played_time);
  gboolean (*get_music_length) (int *len);
  enum OlPlayerStatus (*get_status) ();
  int (*get_capacity) ();
  gboolean (*stop) ();
  gboolean (*play) ();
  gboolean (*pause) ();
  gboolean (*prev) ();
  gboolean (*next) ();
  gboolean (*seek) (int pos_ms);
  void (*free) ();
} OlPlayerController;

/** 
 * @brief Gets all registered players
 * 
 * @return Array of players, end with NULL. You should free it with g_free.
 */
OlPlayerController **ol_player_get_controllers ();

/** 
 * Create a new player controller with given name
 *
 * You can use ol_player_register_controller to register the created
 * controller into the system
 * 
 * @param name The name of controller, must not be NULL.
 *             The controller will take ownership of the string.
 * 
 * @return The new controller, or NULL if failed. The created
 *         controller should be freed with ol_player_free
 */
OlPlayerController *ol_player_new (const char *name);

/** 
 * @brief Free the player_controller
 *
 * If you have set the cmd of the player, you need to set cmd as NULL,
 * and free the returned string manually.
 * 
 * @param player 
 */
void ol_player_free (OlPlayerController *player);


/** 
 * @brief Gets the name of player
 * 
 * @param player 
 * 
 * @return The returned string is owned by player and should not be freed.
 */
const char *ol_player_get_name (OlPlayerController *player);

/** 
 * @brief Sets the command to launch the player.
 * 
 * @param player 
 * @param cmd The launch command. The controller will take ownership
 *            of it.
 * @return The old command string of player.
 */
const char *ol_player_set_cmd (OlPlayerController *player,
                                          const char *cmd);

/** 
 * @brief Gets the command to launch the player
 * 
 * @param player 
 * 
 * @return The returned string is owned by the player and should not
 *         be freed
 */
const char *ol_player_get_cmd (OlPlayerController *player);

/** 
 * @brief Registers a player controller
 *
 * A player controller should be registered so that we can get
 * information from this player.
 * @param controller The player controller to be registered
 */
void ol_player_register_controller (OlPlayerController *controller);

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
void ol_player_unload ();

/** 
 * @brief Gets the infomation of the current music
 * 
 * @param player The player to operate
 * @param info Return location of music info
 * 
 * @return TRUE if succeeded
 */
gboolean ol_player_get_music_info (OlPlayerController *player, OlMusicInfo *info);
/** 
 * @brief Checks whether the player is running.
 * 
 * @param player The player to check
 * 
 * @return TRUE if the player is running
 */
gboolean ol_player_get_activated (OlPlayerController *player);
/** 
 * @brief Gets the elapsed time of the current music
 * 
 * @param player The player to operate
 * @param played_time Return location of the elasped time, in millisecond
 * 
 * @return TRUE if succeeded
 */
gboolean ol_player_get_played_time (OlPlayerController *player, int *played_time);
/** 
 * @brief Gets the duration of the current music
 * 
 * @param player The player to operate
 * @param len Return location of the duration, in millisecond
 * 
 * @return TRUE if succeeded
 */
gboolean ol_player_get_music_length (OlPlayerController *player, int *len);
/** 
 * @brief Gets the status of the player.
 * The status of a player can be playing, paused or stopped.
 * @param player The player to operate
 * 
 * @return The status of the player, or OL_PLAYER_ERROR if failed
 */
enum OlPlayerStatus ol_player_get_status (OlPlayerController *player);
/** 
 * @brief Gets which operations are supported by the player controller
 * 
 * @param player The player to operate
 * 
 * @return A combination of OlPlayerCapacity, or -1 if failed.
 */
int ol_player_get_capacity (OlPlayerController *player);
/** 
 * @brief Starts playing music. If the player supports this operation, OL_PLAYER_PLAY will be set in its capacity
 * 
 * @param player 
 * 
 * @return FALSE if the operation failed or the player controller dosen't support this operation.
 */
gboolean ol_player_play (OlPlayerController *player);
/** 
 * @brief Pauses the current music. The elasped time will not change. If the player supports this operation, OL_PLAYER_STOP will be set in its capacity
 * 
 * @param player 
 * 
 * @return FALSE if the operation failed or the player controller dosen't support this operation.
 */
gboolean ol_player_pause (OlPlayerController *player);
/** 
 * @brief If the player is paused or stopped, resume or play the current music. If it is playing, pause it.
 * 
 * @param player 
 * 
 * @return FALSE if the operation failed or the player controller dosen't support this operation.
 */
gboolean ol_player_play_pause (OlPlayerController *player);
/** 
 * @brief Plays the previous music. If the player supports this operation, OL_PLAYER_PREV will be set in its capacity
 * 
 * @param player 
 * 
 * @return FALSE if the operation failed or the player controller dosen't support this operation.
 */
gboolean ol_player_prev (OlPlayerController *player);
/** 
 * @brief Plays the next music. If the player supports this operation, OL_PLAYER_NEXT will be set in its capacity
 * 
 * @param player 
 * 
 * @return FALSE if the operation failed or the player controller dosen't support this operation.
 */
gboolean ol_player_next (OlPlayerController *player);
/** 
 * @brief Seek the current music to a given position. If the player supports this operation, OL_PLAYER_SEEK will be set in its capacity
 * Note that the actuall time may not equals to the given posision. You may need to call ol_player_get_played_time after this function is called.
 * @param player The player to operate
 * @param pos_ms The target position in millisecond.
 * 
 * @return FALSE if the operation failed or the player controller dosen't support this operation.
 */
gboolean ol_player_seek (OlPlayerController *player, int pos_ms);
/** 
 * @brief Stop playing music. The elapsed time will be reset to 0. If the player supports this operation, OL_PLAYER_STOP will be set in its capacity
 * 
 * @param player 
 * 
 * @return FALSE if the operation failed or the player controller dosen't support this operation.
 */
gboolean ol_player_stop (OlPlayerController *player);
#endif // __OL_PLAYER_H__
