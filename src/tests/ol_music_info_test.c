#include <string.h>
#include <glib.h>
#include <ol_music_info.h>
#include "ol_test_util.h"

const char title[] = "Test Title";
const char album[] = "Test Album";
const char artist[] = "Test Artist";
const char uri[] = "Test URI";
const char title2[] = "Test Title 2";
const char album2[] = "Test Album 2";
const char artist2[] = "Test Artist 2";
const char uri2[] = "Test URI 2";
const int track_number = 19870119;

void
print_music_info (OlMusicInfo *info)
{
  printf ("title: %s\n"
          "artist: %s\n"
          "album: %s\n"
          "track number: %d\n"
          "uri: %s\n",
          ol_music_info_get_title (info),
          ol_music_info_get_artist (info),
          ol_music_info_get_album (info),
          ol_music_info_get_track_number (info),
          ol_music_info_get_uri (info));
}

void
set_music_info (OlMusicInfo *info)
{
  ol_music_info_set_title (info, title);
  ol_music_info_set_album (info, album);
  ol_music_info_set_artist (info, artist);
  ol_music_info_set_uri (info, uri);
  ol_music_info_set_track_number (info, track_number);
}

void
test_clear ()
{
  OlMusicInfo info;
  ol_music_info_init (&info);
  set_music_info (&info);
  ol_music_info_clear (&info);
  ol_test_expect (ol_music_info_get_title (&info) == NULL);
  ol_test_expect (ol_music_info_get_artist (&info) == NULL);
  ol_test_expect (ol_music_info_get_album (&info) == NULL);
  ol_test_expect (ol_music_info_get_uri (&info) == NULL);
}

void
test_setter ()
{
  OlMusicInfo info;
  ol_music_info_init (&info);
  set_music_info (&info);
  ol_test_expect (strcmp (ol_music_info_get_title (&info),
                          title) == 0);
  ol_test_expect (strcmp (ol_music_info_get_artist (&info),
                          artist) == 0);
  ol_test_expect (strcmp (ol_music_info_get_album (&info),
                          album) == 0);
  ol_test_expect (strcmp (ol_music_info_get_uri (&info),
                          uri) == 0);
  ol_test_expect (ol_music_info_get_track_number (&info) == track_number);
  ol_music_info_set_title (&info, title2);
  ol_music_info_set_artist (&info, artist2);
  ol_music_info_set_album (&info, album2);
  ol_music_info_set_uri (&info, uri2);
  ol_test_expect (strcmp (ol_music_info_get_title (&info),
                          title2) == 0);
  ol_test_expect (strcmp (ol_music_info_get_artist (&info),
                          artist2) == 0);
  ol_test_expect (strcmp (ol_music_info_get_album (&info),
                          album2) == 0);
  ol_test_expect (strcmp (ol_music_info_get_uri (&info),
                          uri2) == 0);
  ol_test_expect (ol_music_info_get_track_number (&info) == track_number);
  ol_music_info_set_title (&info, NULL);
  ol_music_info_set_artist (&info, NULL);
  ol_music_info_set_album (&info, NULL);
  ol_music_info_set_uri (&info, NULL);
  ol_test_expect (ol_music_info_get_title (&info) == NULL);
  ol_test_expect (ol_music_info_get_artist (&info) == NULL);
  ol_test_expect (ol_music_info_get_album (&info) == NULL);
  ol_test_expect (ol_music_info_get_uri (&info) == NULL);

}

void
test_serialize ()
{
  OlMusicInfo info;
  ol_music_info_init (&info);
  set_music_info (&info);
  size_t size = ol_music_info_serialize (&info, NULL, 0);
  size_t buf_size = size + 10;
  char *buf = g_new0 (char, buf_size);

  size_t real_size = ol_music_info_serialize (&info, buf, buf_size);
  ol_test_expect (real_size == size);
  ol_test_expect (strlen (buf) == size);

  size_t less_size = size - 10;
  size_t real_less_size = ol_music_info_serialize (&info, buf, less_size);
  ol_test_expect (real_less_size == size);
  ol_test_expect (strlen (buf) == less_size - 1);

  g_free (buf);
}

void
test_deserialize ()
{
  OlMusicInfo info1, info2;
  ol_music_info_init (&info1);
  ol_music_info_init (&info2);
  set_music_info (&info1);
  size_t buflen = ol_music_info_serialize (&info1, NULL, 0);
  buflen++;
  char *buf = g_new (char, buflen);
  ol_music_info_serialize (&info1, buf, buflen);
  ol_test_expect (ol_music_info_deserialize (&info2, buf));
  memset (buf, 0, buflen);
  ol_music_info_serialize (&info2, buf, buflen);
  ol_test_expect (ol_music_info_equal (&info1, &info2));
}

void
test_equal ()
{
  OlMusicInfo info1, info2;
  ol_music_info_init (&info1);
  ol_music_info_init (&info2);
  set_music_info (&info1);
  ol_test_expect (ol_music_info_equal (NULL, NULL));
  ol_test_expect (!ol_music_info_equal (&info1, NULL));
  ol_test_expect (!ol_music_info_equal (NULL, &info2));
  ol_test_expect (!ol_music_info_equal (&info1, &info2));
  set_music_info (&info2);
  ol_test_expect (ol_music_info_equal (&info1, &info2));
}

void
test_copy ()
{
  OlMusicInfo info1, info2;
  ol_music_info_init (&info1);
  ol_music_info_init (&info2);
  set_music_info (&info1);
  ol_test_expect (!ol_music_info_equal (&info1, &info2));
  ol_music_info_copy (&info2, &info1);
  ol_test_expect (ol_music_info_equal (&info1, &info2));
}

int
main ()
{
  test_equal ();
  test_copy ();
  test_setter ();
  test_clear ();
  test_serialize ();
  test_deserialize ();
  return 0;
}
