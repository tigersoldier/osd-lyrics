#ifndef __OL_LRC_PARSER_H__
#define __OL_LRC_PARSER_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#define MAX_LINE 128
#define MAX_LINE_LEN 255
typedef struct _LrcInfo {
	int lyric_time;
	char lyric_text[MAX_LINE_LEN];
    int lyric_id;
	struct _LrcInfo *prev;
	struct _LrcInfo *next;
} LrcInfo;

typedef struct {
	LrcInfo list[MAX_LINE];
	int length;
	int first;
	int last;
        int offset;
} LrcQueue;
/** 
 * Loads an Lrc file and returns its content
 * 
 * @param lyric_sourse A local file name
 * 
 * @return The content of the file, must use free() to destroy it
 */

LrcQueue* ol_lrc_parser_get_lyric_info(char *lyric_source);
/** 
 * Get the first LrcInfo from the current LrcQueue
 * 
 * @param lyric_source  The Lrc filepath
 * 
 * @return the LrcInfo, must not be freed.
 */
LrcInfo *ol_lrc_parser_get_first_Of_list(LrcQueue *list);
/** 
 * Get The last LrcInfo from the current LrcQueue
 * 
 * @Param List The Lrcqueue
 * 
 * @Return The Lrcinfo,Must Not be freed.
 */
LrcInfo *ol_lrc_parser_get_last_Of_list(LrcQueue *list);
/** 
 * Get the next LrcInfo of the current one
 * 
 * @param current_lyric The current LrcInfo
 * 
 * @return the LrcInfo, must not be freed.
 */
LrcInfo *ol_lrc_parser_get_next_of_lyric(LrcInfo *current_lyric);
/** 
 * Get the preview LrcInfo of the current one
 * 
 * @param current_lyric The current LrcInfo.
 * 
 * @return the LrcInfo, must not be freed.
 */
LrcInfo *ol_lrc_parser_get_prev_of_lyric(LrcInfo *current_lyric);
/** 
 * Get the lyric_time of the LrcInfo
 * 
 * @param current_lyric The current LrcInfo.
 * 
 * @return lyric_time The lyric_id of the current LrcInfo.
 */
int ol_lrc_parser_get_lyric_time(LrcInfo *current_lyric);
/** 
 * Get the lyric_text of the LrcInfo
 * 
 * @param current_lyric The current LrcInfo.
 * 
 * @return lyric_text The lyric_text of the current LrcInfo 
 */
char *ol_lrc_parser_get_lyric_text(LrcInfo *current_lyric);
/** 
 * Get the lyric_id of the LrcInfo
 * 
 * @param current_lyric The current LrcInfo.
 * 
 * @return lyric_id The lyric_id of the current LrcInfo 
 */
int ol_lrc_parser_get_lyric_id(LrcInfo *current_lyric);
/** 
 * Get the LrcInfo from LrcQueue by lyric id 
 * 
 * @param list The LrcQueue
 * @param lyric_id The lyric_id of the current LrcInfo 
 * 
 * @return the lrcInfo, must not be freed.
 */
LrcInfo *ol_lrc_parser_get_lyric_by_id(LrcQueue *list,int lyric_id);
/** 
 * update the Lrc_time and offset_time from LrcQueue
 *
 * @param offset The offset of which should be ajusted 
 * @param list The LrcQueue
 */
void ol_lrc_parser_set_lyric_offset(LrcQueue *list, int offset);
/** 
 * get the offset_time from LrcQueue
 *
 * @param list The LrcQueue
 *
 * @return the current offset time
 */
int ol_lrc_parser_get_lyric_offset(LrcQueue *list);
/** 
 * update the offset_time from file
 *
 * @param offset The offset of which should be ajusted 
 * @param lyric_source The Lrc filepath
 */
void ol_lrc_parser_set_lyric_file_offset (char *lyric_source,int offset);
#endif
