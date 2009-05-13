#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef __OL_LRC_PARSER_H__
#define __OL_LRC_PARSER_H__

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
} LrcQueue;
/** 
 * Loads an Lrc file and returns its content
 * 
 * @param FileSource A local file name
 * 
 * @return The content of the file
 */

LrcQueue* ol_lrc_get_lyric_info(char *lyric_source);
LrcInfo *ol_lrc_parser_get_first_Of_list(LrcQueue *list);
LrcInfo *ol_lrc_parser_get_last_Of_list(LrcQueue *list);
LrcInfo *ol_lrc_parser_get_next_of_lyric(LrcInfo *current_lyric);
LrcInfo *ol_lrc_parser_get_prev_of_lyric(LrcInfo *current_lyric);
int ol_lrc_parser_get_lyric_time(LrcInfo *current_lyric);
char *ol_lrc_parser_get_lyric_text(LrcInfo *current_lyric);
int ol_lrc_parser_get_lyric_id(LrcInfo *current_lyric);
LrcInfo *ol_lrc_parser_get_lyric_by_id(LrcQueue *list,int lyric_id);
#endif
