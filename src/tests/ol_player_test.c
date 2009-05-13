#include <gtk/gtk.h>
#include "ol_player.h"

static OlPlayerController *controller = NULL;

gint
timeout_callback (gpointer data)
{
  if (controller == NULL)
  {
    controller = ol_player_get_active_player ();
  }
  if (controller == NULL)
    return;
  OlMusicInfo music = {0};
  if (!controller->get_music_info (&music))
  {
    printf ("get music info failed\n");
    controller = NULL;
  }
  guint time = 0;
  if (!controller->get_played_time (&time))
  {
    printf ("get ellasped time failed\n");
    controller = NULL;
  }
  guint duration = 0;
  if (!controller->get_music_length (&duration))
  {
    printf ("get duration failed\n");
    controller = NULL;
  }
  printf ("title: %s\n"
          "artist: %s\n"
          "album: %s\n"
          "time: %d\n"
          "duration :%d\n",
          music.title,
          music.artist,
          music.album,
          time,
          duration);
  return TRUE;
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  ol_player_init ();
  controller = ol_player_get_active_player ();
  puts ("asdf");
  if (!controller)
  {
    printf ("No controller\n");
    return 1;
  }
  g_timeout_add (100, timeout_callback, NULL);
  gtk_main ();
  return 0;
}
