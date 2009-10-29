#include"ol_lrc_parser.h"

int main()
{
  char test[100]= "mytest.lrc";
  ol_lrc_parser_set_lyric_file_offset (test, 102);
  LrcQueue *list;
  list = ol_lrc_parser_get_lyric_info(test);
  printf ("offset:%d\n",list->offset);

  // printf("%d,%s", ol_lrc_parser_get_lyric_by_id(list,2)->lyric_time,ol_lrc_parser_get_lyric_by_id(list,6)->lyric_text);
  //printf("%s",test);
  // printf("%s",LoadFile(test));
  
  /*char *lrcfile = LoadFile(test);
  LRC_QUEUE *List = malloc (((4+MAX_LINE_LEN+8)*MAX_LINE+6)*sizeof(char));
  memset(List,0,((4+MAX_LINE_LEN+8)*MAX_LINE+6));
  int filesize = strlen(lrcfile);
  // printf("%d",filesize);
  GetLrcInfo(lrcfile, filesize,List);
  int i=0;*/
  /* LrcInfo *temp = &list->list[list->first];
  
  int i=0;
  printf("%d\n",list->length);
  while(i<list->length)
  {
    printf("%d,%s,%d\n",temp->lyric_time,temp->lyric_text,temp->lyric_id);
    temp = temp->next;
    i++;
    }
    free (list);*/
  
}
