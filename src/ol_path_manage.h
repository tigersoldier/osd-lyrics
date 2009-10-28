#ifndef _OL_PATH_MANAGE_H_
#define _OL_PATH_MANAGE_H_

#include <string.h>
#include "ol_music_info.h"

/** 
 * @brief Get full pathname of lyrics file according to the path pattern and file pattern
 *
 * @param path_pattern The pattern of the path
 * @param file_pattern The pattern of the file
 * @param music_info The music track
 * @param pathname Buffer of the pathname
 * @param len Length of the buffer
 *
 * @return The length of the pathname, of -1 if failed
 */
size_t ol_path_get_lrc_pathname (const char *path_pattern,
                                 const char *file_pattern,
                                 OlMusicInfo *music_info,
                                 char *pathname,
                                 size_t len);

/** 
 * @brief Expands the pattern to a file name according to the infomation of a music
 * The following are supported place holder in the pattern:
 *  - %t: Title of the music
 *  - %p: Performer (artist) of the music
 *  - %a: Album of the music
 *  - %n: Track number of the music
 *  - %f: Filename without extension of the music
 *  - %%: The `%' punctuation
 * @param pattern The pattern to be expanded
 * @param music_info The info of the music
 * @param filename The buffer of the expanded file name
 * @param len The size of the buffer
 * 
 * @return The length of the expanded file name, or -1 if failed
 */
size_t ol_path_expand_file_pattern (const char *pattern,
                                    OlMusicInfo *music_info,
                                    char *filename,
                                    size_t len);
/** 
 * @brief Expands the pattern to a directory path according to the infomation of a music
 * The pattern can be one of the three forms:
 *  - begin with `/': the path is an absolute path and will not be expanded
 *  - begin with `~/': the path is an relative path and the `~' wiil be expanded to the absolute path of the user's home directory
 *  - `%': the path will be expanded to the directory of the music file according to its URI
 * @param pattern The pattern to be expanded
 * @param music_info The info of the music, or NULL if the pattern is not `%'
 * @param filename The buffer of the expanded file name
 * @param len The size of the buffer
 * 
 * @return The length of the expanded file name, or -1 if failed
 */
size_t ol_path_expand_path_pattern (const char *pattern,
                                    OlMusicInfo *music_info,
                                    char *filename,
                                    size_t len);
#endif /* _OL_PATH_MANAGE_H_ */
