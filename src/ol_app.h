#ifndef _OL_APP_H_
#define _OL_APP_H_

#include <glib.h>
#include "ol_music_info.h"

gboolean ol_app_download_lyric (OlMusicInfo *music_info);

struct OlPlayer;
struct OlLrc;

/** 
 * @brief Gets the current music
 * 
 * @return 
 */
OlMusicInfo* ol_app_get_current_music (void);

struct OlPlayer* ol_app_get_player (void);

struct OlLrc *ol_app_get_current_lyric (void);

gboolean ol_app_assign_lrcfile (const OlMusicInfo *info,
                                const char *filepath,
                                gboolean update);
#endif /* _OL_APP_H_ */
