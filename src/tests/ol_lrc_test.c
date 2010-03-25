#include <stdlib.h>
#include <stdio.h>
#include "ol_lrc.h"
#include "ol_test_util.h"

#define BUFLEN 1024
#define FILENAME "lrc_basic.lrc"
#define GBK_FILENAME "lrc_gbk.lrc"
#define TMPPATH "/tmp/"
#define WORK_FILE TMPPATH FILENAME
#define WORK_GBK TMPPATH GBK_FILENAME

void prepare_data ()
{
  char cmd[BUFLEN];
  snprintf (cmd, BUFLEN, "cp %s %s", FILENAME, TMPPATH);
  system (cmd);
  snprintf (cmd, BUFLEN, "cp %s %s", GBK_FILENAME, TMPPATH);
  system (cmd);
}

void basic_test ()
{
  struct OlLrc *lrc = ol_lrc_new (FILENAME);
  int i;
  int lasttime;
  for (i = 0; i < ol_lrc_item_count (lrc); i++)
  {
    const struct OlLrcItem *item = ol_lrc_get_item (lrc, i);
    ol_test_expect (item != NULL);
    ol_test_expect (ol_lrc_item_get_id (item) == i);
    if (i > 0)
      ol_test_expect (ol_lrc_item_get_time (item) >= lasttime);
    lasttime = ol_lrc_item_get_time (item);
    /* printf ("id: %d\n" */
    /*         "time: %d\n" */
    /*         "lyric: %s\n", */
    /*         ol_lrc_item_get_id (item), */
    /*         ol_lrc_item_get_time (item), */
    /*         ol_lrc_item_get_lyric (item)); */
  }
  ol_lrc_free (lrc);
}

void gbk_test ()
{
  prepare_data ();
  struct OlLrc *lrc = ol_lrc_new (WORK_FILE);
  struct OlLrc *gbklrc = ol_lrc_new (WORK_GBK);
}

void offset_test ()
{
  prepare_data ();
  struct OlLrc *lrc1 = ol_lrc_new (WORK_FILE);
  ol_lrc_set_offset (lrc1, -100);
  struct OlLrc *lrc2 = ol_lrc_new (WORK_FILE);
  ol_test_expect (ol_lrc_item_count (lrc1) == ol_lrc_item_count (lrc2));
  /* printf ("%d:%d\n", ol_lrc_item_count (lrc1), ol_lrc_item_count (lrc2)); */
  int i, cnt;
  cnt = ol_lrc_item_count (lrc1);
  for (i = 0; i < cnt; i++)
  {
    const struct OlLrcItem *item1 = ol_lrc_get_item (lrc1, i);
    const struct OlLrcItem *item2 = ol_lrc_get_item (lrc2, i);
    ol_test_expect (ol_lrc_item_get_time (item1) == ol_lrc_item_get_time (item2));
    /* printf ("%d:%d\n", ol_lrc_item_get_time (item1), ol_lrc_item_get_time (item2)); */
    ol_test_expect (strcmp (ol_lrc_item_get_lyric (item1),
                       ol_lrc_item_get_lyric (item2)) == 0);
  }
}

int main ()
{
  basic_test ();
  gbk_test ();
  offset_test ();
  return 0;
}
