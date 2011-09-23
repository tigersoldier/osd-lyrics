#include <stdio.h>
#include <string.h>
#include "ol_path_pattern.h"
#include "ol_metadata.h"
#include "ol_test_util.h"

#define BUFFER_SIZE 1024

static void
check_expand_file ()
{
  printf ("%s\n", __FUNCTION__);
  OlMetadata *metadata = ol_metadata_new ();
  ol_metadata_set_title (metadata, "Title");
  ol_metadata_set_artist (metadata, "Artist");
  ol_metadata_set_uri (metadata, "file:///path-to-uri/filename.ext");
  ol_metadata_set_album (metadata, "Album");
  ol_metadata_set_track_number (metadata, 12);
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
                                 metadata,
                                 buffer,
                                 BUFFER_SIZE);
    ol_test_expect (strcmp (buffer, result[i]) == 0);
  }
  ol_metadata_set_uri (metadata, "file:///home/tiger/Music/Bandari/heaven_blue/11.SONG%20OF%20THE%20MAYAS.mp3");
  ol_path_expand_file_pattern ("%f", metadata, buffer, BUFFER_SIZE);
  ol_test_expect (strcmp (buffer, "11.SONG OF THE MAYAS") == 0);
  ol_metadata_free (metadata);
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
  OlMetadata *metadata = ol_metadata_new ();
  ol_metadata_set_uri (metadata, "file:///home/tiger/Music/Bandari/heaven_blue/11.SONG%20OF%20THE%20MAYAS.mp3");
  ol_path_expand_path_pattern (absolute, NULL, buffer, BUFFER_SIZE);
  ol_test_expect (strcmp (buffer, absolute) == 0);
  ol_path_expand_path_pattern (relative, NULL, buffer, BUFFER_SIZE);
  printf ("  %s\n", buffer);
  ol_path_expand_path_pattern (from_uri, metadata, buffer, BUFFER_SIZE);
  ol_test_expect (strcmp (buffer, "/home/tiger/Music/Bandari/heaven_blue") == 0);
  ol_path_expand_path_pattern (empty_uri, metadata, buffer, BUFFER_SIZE);
  ol_test_expect (strcmp (buffer, empty_uri) == 0);
  ol_metadata_free (metadata);
}

/* issue  */
void
check_expand_nouri ()
{
  printf ("%s\n", __FUNCTION__);
  const char from_uri[] = "%";
  char buffer[BUFFER_SIZE] = "";
  OlMetadata *metadata = ol_metadata_new ();
  ol_metadata_set_uri (metadata, "/path/of/file/name");
  ol_test_expect (ol_path_expand_path_pattern (from_uri, metadata, buffer, BUFFER_SIZE) > 0);
  ol_test_expect (strcmp (buffer, "/path/of/file") == 0);
  ol_metadata_free (metadata);
}

int
main ()
{
  check_expand_file ();
  check_expand_path ();
  check_expand_nouri ();
  return 0;
}
