#include "ol_lrc_parser.h"
#include "ol_debug.h"
#include "ol_test_util.h"
#include "ol_lrc_utility.h"

void test ()
{
  ol_log_func ();
  LrcQueue *lrc = ol_lrc_parser_get_lyric_info ("lyric1.lrc");
  LrcInfo *info = ol_lrc_parser_get_first_of_list (lrc);
  char *text = ol_lrc_parser_get_lyric_text (info);
  printf ("%s\n", text);
  ol_test_expect (strcmp (text, "托す者へ～My Dear～") == 0);
  double perc;
  int lrc_id;
  text = NULL;
  ol_lrc_utility_get_lyric_by_time (lrc, 26990, 9999999, text, &perc, &lrc_id);
}

void offset_without_right_bracket ()
{
  ol_log_func ();
  LrcQueue *lrc = ol_lrc_parser_get_lyric_info ("lyric3.lrc");
}

void lack_of_right_bracket ()
{
  ol_log_func ();
  LrcQueue *lrc = ol_lrc_parser_get_lyric_info ("lyric4.lrc");
}

void
test_offset ()
{
  char *NO_OFFSET = "lyric_without_offset.lrc";
  char *WITH_OFFSET = "lyric_with_offset.lrc";
  ol_lrc_parser_set_lyric_file_offset (NO_OFFSET, 100);
  ol_lrc_parser_set_lyric_file_offset (WITH_OFFSET, -100);
}

int main()
{
  /* test (); */
  /* offset_without_right_bracket (); */
  /* lack_of_right_bracket (); */
  test_offset ();
  /* char test[100]= "mytest.lrc"; */
  /* ol_lrc_parser_set_lyric_file_offset (test, 102); */
  /* LrcQueue *list; */
  /* list = ol_lrc_parser_get_lyric_info(test); */
  /* printf ("offset:%d\n",list->offset); */

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
