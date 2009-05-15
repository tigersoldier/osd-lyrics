#ifndef _LRC_FETCH_H
#define _LRC_FETCH_H
#include "ol_utils_lrc_fetch.h"
#include "ol_player.h"

#define URL_LEN_MAX 255
#define TS_LEN_MAX 100 /* max length for title and singer */
#define BUFSIZE 512 

struct lrc_tsu
{
  char title[TS_LEN_MAX];
  char singer[TS_LEN_MAX];
  char url[URL_LEN_MAX];
};

/** 
 * @brief fetch the candidate title-singer-url list;
 *        strongly depending on the web page structure.
 */
typedef struct lrc_tsu *(*Lrc_Search)(const OlMusicInfo *music_info, int *size);

/** 
 * @brief download the lrc and store it in the file system
 */
typedef int (*Lrc_Download)(struct lrc_tsu *tsu, const char *pathname);

struct lrc_interface {
  Lrc_Search ol_lrc_fetch_search;
  Lrc_Download ol_lrc_fetch_download;
};

extern struct lrc_interface sogou;
#endif /* _LRC_FETCH_H */ 
