#ifndef _LRC_FETCH_H
#define _LRC_FETCH_H
#include "ol_lrc_fetch_utils.h"
#include "ol_music_info.h"

#define OL_URL_LEN_MAX 1024
#define OL_TS_LEN_MAX 100 /* max length for title and singer */
#define BUFSIZE 512 

typedef struct _OlLrcCandidate OlLrcCandidate;
struct _OlLrcCandidate
{
  char title[OL_TS_LEN_MAX];
  char artist[OL_TS_LEN_MAX];
  char url[OL_URL_LEN_MAX];
  int rank;
};

/** 
 * @brief fetch the candidate title-singer-url list;
 *        strongly depending on the web page structure.
 */
typedef OlLrcCandidate *(*Lrc_Search) (const OlMusicInfo *music_info,
                                              int *size,
                                              const char *local_charset);

/** 
 * @brief download the lrc and store it in the file system
 */
typedef int (*Lrc_Download) (OlLrcCandidate *candidates,
                             const char *pathname,
                             const char *charset);

typedef struct _OlLrcFetchEngine OlLrcFetchEngine;
struct _OlLrcFetchEngine
{
  char *name;
  Lrc_Search search;
  Lrc_Download download;
};

/** 
 * @brief Get LRC Fetch engine with given name.
 * The engine should NOT be freed.
 * @param name The name of the engine
 * 
 * @return The engine with the given name. If not exists, return default engine.
 */
OlLrcFetchEngine *ol_lrc_fetch_get_engine (const char *name);

/** 
 * @brief Initialize LRC fetch module
 * 
 */
void ol_lrc_fetch_init ();

const char** ol_lrc_fetch_get_engine_list (int *count);

#endif /* _LRC_FETCH_H */ 
