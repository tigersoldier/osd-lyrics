#ifndef _OL_APP_H_
#define _OL_APP_H_

#include "ol_music_info.h"
#include "ol_player.h"
#include "ol_lrc_parser.h"

gboolean ol_app_download_lyric (OlMusicInfo *music_info);

/** 
 * @brief Gets the current music
 * 
 * @return 
 */
OlMusicInfo* ol_app_get_current_music (void);

OlPlayerController* ol_app_get_controller (void);

LrcQueue *ol_app_get_current_lyric (void);

gboolean ol_app_assign_lrcfile (const OlMusicInfo *info,
                                const char *filepath,
                                gboolean update);
#endif /* _OL_APP_H_ */
