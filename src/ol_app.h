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
/** 
 * @brief Adjust the offset of lyric by offset_ms
 *
 * The offset of the lyric will be original offset + offset_ms
 * @param offset_ms Incremental value of offset, in milliseconds
 */
void ol_app_adjust_lyric_offset (int offset_ms);
#endif /* _OL_APP_H_ */
