#include<unistd.h>  /* getopt */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "ol_lrc_fetch.h"

#define OPTSTR "+t:s:h"
#define FALSE 0

const char *charset = "UTF-8";

/* bug #1 */
void test_long_url ()
{
  int lrc_count;
  OlMusicInfo music_info;
  music_info.title = "Little Lotte/The Mirror (Angel of Music)";
  music_info.artist = "泉こなた（平野綾），柊かがみ（加藤英美里），柊つかさ（福原香織），高良みゆき（遠藤綾）";
  struct OlLrcCandidate *candidates = sogou.search (&music_info, &lrc_count, "UTF-8");
}

void test_search ()
{
  int lrc_count;
  OlMusicInfo music_info;
  music_info.title = "红豆";
  music_info.artist = "王菲";
  struct OlLrcCandidate *candidates = sogou.search (&music_info, &lrc_count, "UTF-8");
  printf ("Count: %d\n", lrc_count);
  int i;
  for (i = 0; i < lrc_count; i++)
  {
    printf ("[%d] %s %s\n", i, candidates[i].title, candidates[i].artist);
  }
}

void test_download ()
{
  int lrc_count;
  OlMusicInfo music_info;
  music_info.title = "红豆";
  music_info.artist = "王菲";
  struct OlLrcCandidate *candidates = sogou.search (&music_info, &lrc_count, "UTF-8");
  if (lrc_count > 0)
    sogou.download (&candidates[0], "/tmp/tmplrctest.lrc", "UTF-8");
}

int main(int argc, char *argv[])
{
  test_long_url ();
  test_search ();
  test_download ();
  return 0;
}

