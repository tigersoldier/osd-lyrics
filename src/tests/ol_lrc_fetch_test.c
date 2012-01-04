#include<unistd.h>  /* getopt */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "ol_lrc_fetch.h"
#include "ol_test_util.h"

#define OPTSTR "+t:s:h"

const char *charset = "UTF-8";
const char *enginename = "Minilyrics";
const char *title = "Candidate Title";
const char *artist = "Candidate Artist";
const char *uri = "Candidate URI";
const int rank = 19870119;

/* bug #1 */
void test_long_url ()
{
  printf ("%s\n", __FUNCTION__);
  int lrc_count;
  OlMetadata *metadata = ol_metadata_new ();
  ol_metadata_set_title (metadata,
                         "Little Lotte/The Mirror (Angel of Music)");
  ol_metadata_set_artist (metadata,
                          "泉こなた（平野綾），柊かがみ（加藤英美里），柊つかさ（福原香織），高良みゆき（遠藤綾）");
  ol_lrc_fetch_get_engine (enginename)->search (metadata, &lrc_count, "UTF-8");
  ol_metadata_free (metadata);
}

void test_search ()
{
  printf ("%s\n", __FUNCTION__);
  int lrc_count;
  OlMetadata *metadata = ol_metadata_new ();
  ol_metadata_set_title (metadata, "红豆");
  ol_metadata_set_artist (metadata, "王菲");
  OlLrcCandidate *candidates = ol_lrc_fetch_get_engine (enginename)->search (metadata, &lrc_count, "UTF-8");
  printf ("Count: %d\n", lrc_count);
  int i;
  for (i = 0; i < lrc_count; i++)
  {
    printf ("[%d] %s %s %s\n", i, candidates[i].title, candidates[i].artist, candidates[i].url);
  }
  ol_metadata_free (metadata);
}

void test_download ()
{
  printf ("%s\n", __FUNCTION__);
  int lrc_count;
  OlMetadata *metadata = ol_metadata_new ();
  ol_metadata_set_title (metadata, "虫儿飞");
  /* metadata.title = "eyes on me"; */
  ol_metadata_set_artist (metadata, "王菲");
  OlLrcCandidate *candidates = ol_lrc_fetch_get_engine (enginename)->search (metadata, &lrc_count, "UTF-8");
  if (lrc_count > 0)
  {
    size_t size;
    gchar *content = ol_lrc_fetch_get_engine (enginename)->download (&candidates[0], &size);
    g_free (content);
  }
  ol_metadata_free (metadata);
}

void
set_candidate (OlLrcCandidate *cand)
{
  if (cand == NULL)
    return;
  ol_lrc_candidate_set_title (cand, title);
  ol_lrc_candidate_set_artist (cand, artist);
  ol_lrc_candidate_set_url (cand, uri);
  ol_lrc_candidate_set_rank (cand, rank);
}

void
print_candidate (OlLrcCandidate *cand)
{
  if (cand == NULL)
    return;
  printf ("title: %s\n", ol_lrc_candidate_get_title (cand));
  printf ("artist: %s\n", ol_lrc_candidate_get_artist (cand));
  printf ("url: %s\n", ol_lrc_candidate_get_url (cand));
  printf ("randk: %d\n", ol_lrc_candidate_get_rank (cand));
}

void
test_candidate_serialize ()
{
  OlLrcCandidate *cand = ol_lrc_candidate_new ();
  set_candidate (cand);
  int size = ol_lrc_candidate_serialize (cand, NULL, 0);
  int buf_size = size + 10;
  char *buffer = malloc (buf_size);
  int real_size = ol_lrc_candidate_serialize (cand, buffer, buf_size);
  ol_test_expect (real_size == size);
  free (buffer);
  ol_lrc_candidate_free (cand);
}

void
test_candidate_deserialize ()
{
  OlLrcCandidate *cand1 = ol_lrc_candidate_new ();
  OlLrcCandidate *cand2 = ol_lrc_candidate_new ();
  set_candidate (cand1);
  int size = ol_lrc_candidate_serialize (cand1, NULL, 0);
  int buf_size = size + 10;
  char *buffer = malloc (buf_size);
  ol_lrc_candidate_serialize (cand1, buffer, buf_size);
  ol_test_expect (ol_lrc_candidate_deserialize (cand2, buffer));
  ol_test_expect (strcmp (ol_lrc_candidate_get_title (cand2),
                          title) == 0);
  ol_test_expect (strcmp (ol_lrc_candidate_get_artist (cand2),
                          artist) == 0);
  ol_test_expect (strcmp (ol_lrc_candidate_get_url (cand2),
                          uri) == 0);
  ol_test_expect (ol_lrc_candidate_get_rank (cand2) == rank);
  free (buffer);
  ol_lrc_candidate_free (cand1);
  ol_lrc_candidate_free (cand2);
}

int main(int argc, char *argv[])
{
  test_candidate_serialize ();
  test_candidate_deserialize ();
  ol_lrc_fetch_init ();
  test_long_url ();
  test_search ();
  test_download ();
  return 0;
}

