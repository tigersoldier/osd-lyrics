#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
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
  ol_lrc_free (lrc);
  ol_lrc_free (gbklrc);
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
  ol_lrc_free (lrc1);
  ol_lrc_free (lrc2);
}

/* Issue 70 */
void no_newline_test ()
{
  const char *DATAFILE = "lrc_no_newline.lrc";
  struct OlLrc *lrc = ol_lrc_new (DATAFILE);
  const struct OlLrcItem *item = ol_lrc_get_item (lrc, 0);
  ol_test_expect (ol_lrc_item_get_lyric (item) != NULL);
  ol_test_expect (strcmp (ol_lrc_item_get_lyric (item), "lyric") == 0);
  ol_lrc_free (lrc);
}

/* Issue 99 */
void begin_with_timestamp_test ()
{
  const char *DATAFILE = "lrc_tail.lrc";
  struct OlLrc *lrc = ol_lrc_new (DATAFILE);
  const struct OlLrcItem *item = ol_lrc_get_item (lrc, 0);
  ol_test_expect (ol_lrc_item_get_time (item) == 10000);
  ol_lrc_free (lrc);
}

void tail_test ()
{
  const char *DATAFILE = "lrc_tail.lrc";
  struct OlLrc *lrc = ol_lrc_new (DATAFILE);
  double per = 0;
  char *lyric = NULL;
  ol_lrc_get_lyric_by_time (lrc,
                            40000,
                            60000,
                            &lyric,
                            &per,
                            (int*)NULL);
  ol_test_expect (fabsl (per - 0.33333333) < 1e-6);
  ol_lrc_free (lrc);
}

int main ()
{
  basic_test ();
  gbk_test ();
  offset_test ();
  no_newline_test ();
  begin_with_timestamp_test ();
  tail_test ();
  return 0;
}
