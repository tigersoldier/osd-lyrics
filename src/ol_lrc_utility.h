#ifndef __OL_LRC_UTILITY_H__
#define __OL_LRC_UTILITY_H__

#include "ol_lrc_parser.h"


void ol_lrc_utility_get_lyric_by_time (LrcQueue *list, int time,int end_time, char *lyric_text, double *percentage,int *lyric_id);

#endif
