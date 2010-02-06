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
OlMusicInfo* ol_app_get_current_music ();

OlPlayerController* ol_app_get_controller ();

LrcQueue *ol_app_get_current_lyric ();

#endif /* _OL_APP_H_ */
