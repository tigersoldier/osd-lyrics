#include <stdio.h>
#include "ol_path_manage.h"
#include "ol_music_info.h"

#define BUFFER_SIZE 1024

static void
check_expand_file ()
{
  printf ("%s\n", __FUNCTION__);
  OlMusicInfo info;
  ol_music_info_init (&info);
  info.title = "Title";
  info.artist = "Artist";
  info.uri = "file:///path-to-uri/filename.ext";
  info.album = "Album";
  info.track_number = 12;
  char buffer[BUFFER_SIZE];
  char pattern[][1024] = {
    "%p - %t",
    "%n%p%t%a"
  };
  char result[][1024] = {
    "Artist - Title",
    "12ArtistTitleAlbum",
  };
  int i;
  for (i = 0; i < 2; i++)
  {
    ol_path_expand_file_pattern (pattern[i],
                                 &info,
                                 buffer,
                                 BUFFER_SIZE);
    if (strcmp (buffer, result[i]) != 0)
    {
      printf ("  Fail! %s != %s\n", buffer, result[i]);
    }
    else
    {
      printf ("  %s\n", buffer);
    }
  }
  info.uri = "file:///home/tiger/Music/Bandari/heaven_blue/11.SONG%20OF%20THE%20MAYAS.mp3";
  ol_path_expand_file_pattern ("%f", &info, buffer, BUFFER_SIZE);
  printf ("  %s\n", buffer);
}

void
check_expand_path ()
{
  printf ("%s\n", __FUNCTION__);
  const char absolute[] = "/path/to/lyric/";
  const char relative[] = "~/.lyrics/";
  const char from_uri[] = "%";
  const char empty_uri[] = "";
  char buffer[BUFFER_SIZE] = "";
  OlMusicInfo info;
  ol_music_info_init (&info);
  info.uri = "file:///home/tiger/Music/Bandari/heaven_blue/11.SONG%20OF%20THE%20MAYAS.mp3";
  ol_path_expand_path_pattern (absolute, NULL, buffer, BUFFER_SIZE);
  printf ("  %s\n", buffer);
  ol_path_expand_path_pattern (relative, NULL, buffer, BUFFER_SIZE);
  printf ("  %s\n", buffer);
  ol_path_expand_path_pattern (from_uri, &info, buffer, BUFFER_SIZE);
  printf ("  %s\n", buffer);
  ol_path_expand_path_pattern (empty_uri, &info, buffer, BUFFER_SIZE);
  printf ("  %s\n", buffer);
}

int
main ()
{
  check_expand_file ();
  check_expand_path ();
  return 0;
}
