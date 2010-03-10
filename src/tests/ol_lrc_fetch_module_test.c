#include <stdio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "ol_lrc_fetch_module.h"
#include "ol_test_util.h"

#define CANDIDATE_COUNT 2

int search_id = 0;

static OlLrcCandidate*
dummy_search (const OlMusicInfo *music_info,
              int *size,
              const char *local_charset)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  static OlLrcCandidate candidates[CANDIDATE_COUNT];
  int i;
  for (i = 0; i < CANDIDATE_COUNT; i++)
  {
    sprintf (candidates[i].title, "Title %d", i);
    sprintf (candidates[i].artist, "Artist %d", i);
    sprintf (candidates[i].url, "Url %d", i);
  }
  *size = CANDIDATE_COUNT;
  sleep (2);
  return candidates;
}

/** 
 * @brief download the lrc and store it in the file system
 */
static int
dummy_download (OlLrcCandidate *candidates,
                const char *pathname,
                const char *charset)
{
  fprintf (stderr, "downloading: title: %s, artist: %s, url: %s\n",
           candidates->title, candidates->artist, candidates->url);
  sleep (2);
  return TRUE;
}

OlLrcFetchEngine engine = {
  "dummy",
  dummy_search,
  dummy_download,
};

void
print_candidate (OlLrcCandidate *cand)
{
  printf ("title:%s\n", ol_lrc_candidate_get_title (cand));
}

void
dummy_search_callback (struct OlLrcFetchResult *result,
                       void *data)
{
  printf ("Search Callback invoked\n");
  printf ("count: %d\n", result->count);
  printf ("userdata: %s\n", (char *) data);
  ol_test_expect (search_id == result->id);
  int i = 0;
  for (i = 0; i < result->count; i++)
    print_candidate (result->candidates + i);
  ol_lrc_fetch_begin_download (result->engine,
                               &result->candidates[0],
                               NULL,
                               "abc",
                               "abc");
}

void
dummy_download_callback (struct OlLrcDownloadResult *result)
{
  fprintf (stderr, "Download Callback invoked, file is %s\n", result->filepath);
  ol_test_expect (result->userdata != NULL);
  ol_test_expect (strcmp (result->filepath, (char *)result->userdata) == 0);
}

int
main (int argc, char **argv)
{
  g_thread_init (NULL);
  gtk_init (&argc, &argv);
  ol_lrc_fetch_module_init ();
  OlMusicInfo info;
  ol_music_info_init (&info);
  ol_lrc_fetch_add_async_download_callback (dummy_download_callback);
  ol_music_info_set_title (&info, "title");
  ol_music_info_set_artist (&info, "artist");
  ol_music_info_set_album (&info, "album");
  search_id = ol_lrc_fetch_begin_search (&engine, &info, dummy_search_callback, "Callback User data");
  gtk_main ();
}
