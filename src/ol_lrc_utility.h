#ifndef __OL_LRC_UTILITY_H__
#define __OL_LRC_UTILITY_H__

#include "ol_lrc_parser.h"

/** 
 * @brief Gets the lyric for the given time, and the playing progress of the lyric
 * The returned lyric's start time should be less or equal than the given time, while it's end time should be
 * larger than the given time.
 * If there is no lyric available, e.g, the time is earlier than the first lyric or later than the end_time,
 * an invalid lyric will be returned, with zero-length lyric text, 0% of progress, and -1 of its id.
 * @param list An LrcQueue
 * @param time The given time point
 * @param end_time The duration of the song. It's necessary because we don't know the end time of the
 *                 last lyric
 * @param lyric_text The lyric text at the given time
 * @param percentage The progress of the lyric at the given time, in percent.
 * @param lyric_id The id of the lyric. If it's an invalid lyric, its id is -1
 */
void ol_lrc_utility_get_lyric_by_time (LrcQueue *list, int time,int end_time, char *lyric_text, double *percentage,int *lyric_id);

#endif
