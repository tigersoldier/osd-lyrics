#ifndef _LRC_FETCH_H
#define _LRC_FETCH_H
#include "ol_lrc_fetch_utils.h"
#include "ol_music_info.h"

#define OL_URL_LEN_MAX 255
#define OL_TS_LEN_MAX 100 /* max length for title and singer */
#define BUFSIZE 512 

struct OlLrcCandidate
{
  char title[OL_TS_LEN_MAX];
  char artist[OL_TS_LEN_MAX];
  char url[OL_URL_LEN_MAX];
};

/** 
 * @brief fetch the candidate title-singer-url list;
 *        strongly depending on the web page structure.
 */
typedef struct OlLrcCandidate *(*Lrc_Search) (const OlMusicInfo *music_info,
                                              int *size,
                                              const char *local_charset);

/** 
 * @brief download the lrc and store it in the file system
 */
typedef int (*Lrc_Download) (struct OlLrcCandidate *candidates,
                             const char *pathname,
                             const char *charset);

struct lrc_interface {
  Lrc_Search search;
  Lrc_Download download;
};

extern struct lrc_interface sogou;
#endif /* _LRC_FETCH_H */ 
