#include <string.h>
#include "ol_lrc_fetch_ui.h"
#include "ol_lrc_fetch.h"

void test_show ()
{
  OlLrcFetchEngine engine;
  OlLrcCandidate candidates[5];
  strcpy (candidates[0].title, "Tiger");
  strcpy (candidates[0].artist, "Soldier");
  strcpy (candidates[1].title, "All I Ask of You");
  strcpy (candidates[1].artist, "Tiger Soldier & Christine");
  ol_lrc_fetch_ui_show (&engine, candidates, 2, "");
  gtk_main ();
}

int main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  test_show ();
  return 0;
}
