#ifndef _OL_LYRIC_MANAGE_H_
#define _OL_LYRIC_MANAGE_H_

#include "ol_music_info.h"

/** 
 * @brief Find lyric file according to music info
 * 
 * @param info 
 * 
 * @return If lyric file found, return the full path of the file.
 *         Otherwise returns NULL. 
 *         The returned path should be freed with g_free.
 */
char *ol_lyric_find (OlMusicInfo *info);

/** 
 * @brief Get the full path to save the downloaded file
 * 
 * @param info 
 * 
 * @return If a vaild path found, return the full path.
 *         Otherwise returns NULL. 
 *         The returned path should be freed with g_free.
 */
char *ol_lyric_download_path (OlMusicInfo *info);
#endif /* _OL_LYRIC_MANAGE_H_ */
