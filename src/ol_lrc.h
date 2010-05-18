#ifndef _OL_LRC_H_
#define _OL_LRC_H_

struct OlLrc;
/** 
 * @brief A lyric item, represents a single sentance to be singed
 *
 * A lyric item has 3 attributes: id, timestamp, and lyric text
 * @param filename 
 * 
 * @return 
 */
struct OlLrcItem;

/** 
 * @brief Create a new OlLrc instance from an LRC file
 * 
 * @param filename 
 * 
 * @return The OlLrc instance of the file, or NULL if filename doesn't exist.
 *         You should free it with ol_lrc_free.
 */
struct OlLrc *ol_lrc_new (const char *filename);

void ol_lrc_free (struct OlLrc *lrc);

/** 
 * @brief Get the number of lyric items
 * 
 * @param lrc 
 * 
 * @return The number of lyric items
 */
int ol_lrc_item_count (struct OlLrc *lrc);

/** 
 * @brief Get the lyric item previous to the given one
 *
 * The previous lyric item is the  before the current one
 * @param item 
 * 
 * @return The previous item, or NULL if the item is the first one
 */
const struct OlLrcItem *ol_lrc_item_prev (const struct OlLrcItem *item);

/** 
 * @brief Get the lyric item next to the given one
 *
 * The next lyric item is the one just after the current one
 * @param item 
 * 
 * @return The previous item, or NULL if the item is the first one
 */
const struct OlLrcItem *ol_lrc_item_next (const struct OlLrcItem *item);

/** 
 * @brief Get the timestamp of the lyric, in milliseconds
 *
 * The time is calculated with offset of LRC
 * @param item 
 * 
 * @return The timestamp of the lyric item, or -1 if the item is invalid
 */
int ol_lrc_item_get_time (const struct OlLrcItem *item);

/** 
 * @brief Get the text of the lyric item
 * 
 * @param item 
 * 
 * @return The text of the lyric item. You shouldn't free it.
 */
const char *ol_lrc_item_get_lyric(const struct OlLrcItem *item);

/** 
 * @brief Get the id of the lyric item
 *
 * The id differs with different lyric item, and is in ascending order by
 * timestamp. Ids start from 0.
 * @param item 
 * 
 * @return The id of the item, or -1 if the item is invalid.
 */
int ol_lrc_item_get_id (const struct OlLrcItem *item);

/** 
 * @brief Find an lyric item according to id
 * 
 * @param lrc 
 * @param id 
 * 
 * @return The lyric item with the id, or NULL if not found
 */
const struct OlLrcItem *ol_lrc_get_item (struct OlLrc *lrc, int id);

/** 
 * @brief Gets the lyric for the given time, and the playing progress of the lyric.
 *
 * The returned lyric's start time should be less or equal than the given time,
 * while it's end time should be larger than the given time.
 * If there is no lyric available, e.g, the time is earlier than the first lyric
 * or later than the music_duration, an invalid lyric will be returned, with NULL
 * lyric text, 0% of progress, and -1 of its id.
 * @param list An LrcQueue
 * @param time The given time point, in milliseconds.
 * @param music_duration The duration of the song, in milliseconds.
 *                 It's necessary because we don't know the end time of the
 *                 last lyric.
 * @param text The return location of lyric text at the given time, or NULL.
 *             If the lyric is not found, the value will be set to NULL.
 *             It should be freed with g_free.
 * @param percentage The progress of the lyric at the given time, in percent.
 * @param id The id of the lyric. If it's an invalid lyric, its id is -1
 */
void ol_lrc_get_lyric_by_time (struct OlLrc *lrc,
                               int time,
                               int music_duration,
                               char **text,
                               double *percentage,
                               int *id);
#endif /* _OL_LRC_H_ */

/** 
 * Update the Lrc_time and offset_time from LrcQueue
 *
 * @param lrc
 * @param offset The offset of which should be ajusted 
 */
void ol_lrc_set_offset (struct OlLrc *lrc, int offset);

/** 
 * get the offset_time from LrcQueue
 *
 * @param lrc
 *
 * @return the current offset time
 */
int ol_lrc_get_offset (struct OlLrc *lrc);

/** 
 * @brief Get the filename of the LRC file
 * 
 * @param lrc 
 * 
 * @return The filename or NULL
 */
const char *ol_lrc_get_filename (const struct OlLrc *lrc);
