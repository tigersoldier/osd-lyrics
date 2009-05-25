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

int main(int argc, char *argv[])
{
  test_long_url ();
  return 0;
}

