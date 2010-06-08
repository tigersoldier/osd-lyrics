#ifndef _OL_DISPLAY_MODULE_H_
#define _OL_DISPLAY_MODULE_H_

#include "ol_music_info.h"
struct OlLrc;
struct OlPlayer;
enum OlPlayerStatus;

struct OlDisplayModule
{
  const struct OlDisplayClass *klass;
  void *data;
};

typedef void* (*OlDisplayInitFunc) (struct OlDisplayModule *module);
typedef void (*OlDisplayFreeFunc) (struct OlDisplayModule *module);

struct OlDisplayClass
{
  char *name;
  OlDisplayInitFunc init;
  OlDisplayFreeFunc free;
  void (*set_music_info) (struct OlDisplayModule *module,
                          OlMusicInfo *music_info);
  void (*set_player) (struct OlDisplayModule *module,
                      struct OlPlayer *player);
  void (*set_status) (struct OlDisplayModule *module,
                      enum OlPlayerStatus status);
  void (*set_played_time) (struct OlDisplayModule *module,
                           int played_time);
  void (*set_lrc) (struct OlDisplayModule *module,
                   struct OlLrc *lrc_file);
  void (*set_duration) (struct OlDisplayModule *module,
                        int duration);
  void (*set_message) (struct OlDisplayModule *module,
                       const char *message,
                       int duration_ms);
  void (*search_message) (struct OlDisplayModule *module,
                          const char *message);
  void (*search_fail_message) (struct OlDisplayModule *module,
                               const char *message);
  void (*download_fail_message) (struct OlDisplayModule *module,
                                 const char *message);
  void (*clear_message) (struct OlDisplayModule *module);
};

/** functions for implementing concrete modules **/

/** 
 * @brief Create a new class for display module
 *
 * This should be used in ol_foo_moduel_get_class, where foo is the name of
 * the display module
 *
 * @param name The name of the new display module
 * @param init_func The function to initialize the private data
 * @param free_func The function to free the private data
 * 
 * @return A new class of display module, with all functions but init and free
 *         set to NULL
 */
struct OlDisplayClass* ol_display_class_new (const char *name,
                                             OlDisplayInitFunc init_func,
                                             OlDisplayFreeFunc free_func);

void* ol_display_module_get_data (struct OlDisplayModule *module);

/** functions for display module controlling **/

/** 
 * Initialize the display module.
 * 
 * This function must be called before any other functions
 */
void ol_display_module_init ();
void ol_display_module_unload ();
/** 
 * @brief Create a display module of given type
 * 
 * @param name The name of the type of display module, case insensitive
 * 
 * @return The new display module. If the display module with the given name
 *         not exists, a default display module is returned. The returned module
 *         must be freed with ol_display_module_free.
 */
struct OlDisplayModule *ol_display_module_new (const char *name);

void ol_display_module_free (struct OlDisplayModule *module);
void ol_display_module_set_music_info (struct OlDisplayModule *module,
                                       OlMusicInfo *music_info);
void ol_display_module_set_player (struct OlDisplayModule *module,
                                   struct OlPlayer *player);
void ol_display_module_set_status (struct OlDisplayModule *module,
                                   enum OlPlayerStatus status);
void ol_display_module_set_played_time (struct OlDisplayModule *module,
                                        int played_time);
void ol_display_module_set_lrc (struct OlDisplayModule *module,
                                struct OlLrc *lrc_file);
void ol_display_module_set_duration (struct OlDisplayModule *module,
                                     int duration);
void ol_display_module_set_message (struct OlDisplayModule *module,
                                    const char *message,
                                    int duration_ms);
void ol_display_module_search_message (struct OlDisplayModule *module,
                                       const char *message);
void ol_display_module_search_fail_message (struct OlDisplayModule *module,
                                            const char *message);
void ol_display_module_download_fail_message (struct OlDisplayModule *module,
                                              const char *message);
void ol_display_module_clear_message (struct OlDisplayModule *module);

#endif /* _OL_DISPLAY_MODULE_H_ */
