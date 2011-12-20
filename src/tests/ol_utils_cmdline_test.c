#include <stdio.h>
#include <string.h>
#include "ol_utils_cmdline.h"
#include "ol_test_util.h"

void
test_string ()
{
  char *string = g_strdup ("Tiger Soldier");
  char *strold = string;
  gboolean ret = ol_cmd_get_string ("echo Cmd test", &string);
  ol_test_expect (ret == TRUE);
  ol_test_expect (strcmp (string, "Cmd test\n") == 0);
  if (string)
    g_free (string);
  ol_test_expect (strcmp (strold, "Tiger Soldier") == 0);
  g_free (strold);
}

void
test_null_string ()
{
  gboolean ret = ol_cmd_get_string ("ls", NULL);
  ol_test_expect (ret == TRUE);
}

void
test_int ()
{
  int sum;
  const char *cmd = "expr 3 + 2";
  const int ans = 5;
  gboolean ret = ol_cmd_get_int (cmd, &sum);
  ol_test_expect (ret == TRUE);
  ol_test_expect (sum == ans);
  ret = ol_cmd_get_int ("/home/", &sum);
  ol_test_expect (ret == FALSE);
  ol_test_expect (sum == ans);
  ret = ol_cmd_get_int ("pgrep init", &sum);
  ol_test_expect (ret == TRUE);
  ret = ol_cmd_get_int ("pgrep /asdf/", &sum);
  ol_test_expect (ret == FALSE);
}

int
main ()
{
  test_string ();
  test_null_string ();
  test_int ();
  return 0;
}
