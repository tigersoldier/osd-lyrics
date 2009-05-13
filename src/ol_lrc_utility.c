#include "ol_lrc_utility.h"

void ol_lrc_utility_get_lyric_by_time (LrcQueue *list, int time,int end_time, char *lyric_text, double *percentage,int *lyric_id)
{
  LrcInfo *temp = ol_lrc_parser_get_first_Of_list(list);
  int i=0;
  while(temp != NULL&&i==0)
  {
    int start_time = ol_lrc_parser_get_lyric_time(temp);
    if(ol_lrc_parser_get_next_of_lyric(temp)!=NULL)
    {
      int next_time = (ol_lrc_parser_get_lyric_time(ol_lrc_parser_get_next_of_lyric(temp)));
      if(time>start_time&&time<next_time)
      {
        strcpy (lyric_text, ol_lrc_parser_get_lyric_text(temp));
        *percentage = (double)(time-start_time)/(double)(next_time-start_time);
        *lyric_id = ol_lrc_parser_get_lyric_id(temp);
        i=1;
      }
    }
    else
    {
      if(time<end_time)
      {
        strcpy(lyric_text,ol_lrc_parser_get_lyric_text(temp));
        *percentage = (double)(time-start_time)/(double)(end_time-start_time);
        *lyric_id = ol_lrc_parser_get_lyric_id(temp);
          printf("%s","no");
      }
    }
    temp = ol_lrc_parser_get_next_of_lyric(temp);
  }
}
