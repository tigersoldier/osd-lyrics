#include <string.h>
#include <stdio.h>
#include "ol_lrc_fetch_ui.h"
#include "ol_lrc_fetch.h"
#define N_CANDIDATE 10

void test_show ()
{
  OlLrcFetchEngine engine;
  OlLrcCandidate candidates[N_CANDIDATE + 1];
  strcpy (candidates[0].title, "Tiger");
  strcpy (candidates[0].artist, "Soldier");
  strcpy (candidates[1].title, "All I Ask of You");
  strcpy (candidates[1].artist, "Tiger Soldier & Christine");
  int i;
  for (i = 2; i < N_CANDIDATE; i++)
  {
    sprintf (candidates[i].title, "Title #%d", i);
    sprintf (candidates[i].artist, "Artist #%d", i);
  }
  ol_lrc_fetch_ui_show (&engine,
                        candidates, N_CANDIDATE,
                        NULL,
                        "");
  gtk_main ();
}

int main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  test_show ();
  return 0;
}
