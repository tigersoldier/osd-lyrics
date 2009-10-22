#ifndef __OL_PLAYER_H__
#define __OL_PLAYER_H__

#include <glib.h>
#include "ol_music_info.h"

enum OlPlayerStatus{
  OL_PLAYER_STOPPED,
  OL_PLAYER_PLAYING,
  OL_PLAYER_PAUSED,
  OL_PLAYER_UNKNOWN,
  OL_PLAYER_ERROR,
};

enum OlPlayerCapacity {
  OL_PLAYER_URI,
  OL_PLAYER_CONTROL,
  OL_PLAYER_STATUS,
  OL_PLAYER_PLAY,
  OL_PLAYER_PREV,
  OL_PLAYER_NEXT,
  OL_PLAYER_SEEK,
  OL_PLAYER_STOP,
  OL_PLAYER_PAUSE,
};

typedef struct
{
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
