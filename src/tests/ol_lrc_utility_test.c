#include "ol_lrc_utility.h"
int main()
{
  char test[100]= "mytest.lrc";
  LrcQueue *list = ol_lrc_parser_get_lyric_info(test);
  int time = 189358;
  int end_time = 213028;
  char lyric_text[255] = "";
  double percentage;
  int lyric_id;
  ol_lrc_utility_get_lyric_by_time(list,time,end_time,lyric_text,&percentage,&lyric_id);
  LrcInfo *info = ol_lrc_parser_get_lyric_by_id (list, lyric_id);
  if (info != NULL)
    printf("%d,%s,%lf",lyric_id,ol_lrc_parser_get_lyric_text (info),percentage);
  printf ("%d\n", lyric_id);
  
}
