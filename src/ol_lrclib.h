#ifndef _OL_LRCLIB_H_
#define _OL_LRCLIB_H_

#include "ol_music_info.h"

/** 
 * @brief Initializes the Lrclib module
 * 
 * @param filename The lrclib filename. 
 *                 If not exists, a new one will be created
 * @return Non-zero if succeded. Otherwise returns 0.
 */
int ol_lrclib_init (const char *filename);

/** 
 * @brief Unload the Lrclib module.
 *
 * It will close the database. You should unload it before exit
 * 
 */
void ol_lrclib_unload ();

/** 
 * @brief Assign an LRC file to a music
 * 
 * @param info The music info to be assigned
 * @param lrcpath The LRC file, or NULL if no lyric should be assigned
 * 
 * @return Non-zero if succeeded.
 */
int ol_lrclib_assign_lyric (const OlMusicInfo *info, 
                            const char *lrcpath);

/** 
 * @brief Find the lyric assigned to a music
 * 
 * The lyric will be searched according to the file uri, then to the
 * combination of title, artist and album
 * 
 * @param info The music info to be assigned
 * @param lrcpath The return loaction to the LRC File. This may be set to 
 *                NULL. If not NULL, it should be freed with 
 * 
 * @return Non-zero if succeeded. 0 if not found or error occured
 */
int ol_lrclib_find (const OlMusicInfo *info,
                    char **lrcpath);
#endif /* _OL_LRCLIB_H_ */
