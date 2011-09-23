#include <string.h>
#include <glib.h>
#include "ol_metadata.h"
#include "ol_test_util.h"

const char title[] = "Test Title";
const char album[] = "Test Album";
const char artist[] = "Test Artist";
const char uri[] = "Test URI";
const char art[] = "Test Art";
const int duration = 19860829;
const char title2[] = "Test Title 2";
const char album2[] = "Test Album 2";
const char artist2[] = "Test Artist 2";
const char uri2[] = "Test URI 2";
const int track_number = 19870119;

void
print_metadata (OlMetadata *info)
{
  printf ("title: %s\n"
          "artist: %s\n"
          "album: %s\n"
          "track number: %d\n"
          "uri: %s\n",
          ol_metadata_get_title (info),
          ol_metadata_get_artist (info),
          ol_metadata_get_album (info),
          ol_metadata_get_track_number (info),
          ol_metadata_get_uri (info));
}

void
set_metadata (OlMetadata *info)
{
  ol_metadata_set_title (info, title);
  ol_metadata_set_album (info, album);
  ol_metadata_set_artist (info, artist);
  ol_metadata_set_uri (info, uri);
  ol_metadata_set_track_number (info, track_number);
  ol_metadata_set_duration (info, duration);
  ol_metadata_set_art (info, art);
}

void
test_clear ()
{
  OlMetadata *metadata = ol_metadata_new ();
  set_metadata (metadata);
  ol_metadata_clear (metadata);
  ol_test_expect (ol_metadata_get_title (metadata) == NULL);
  ol_test_expect (ol_metadata_get_artist (metadata) == NULL);
  ol_test_expect (ol_metadata_get_album (metadata) == NULL);
  ol_test_expect (ol_metadata_get_uri (metadata) == NULL);
  ol_test_expect (ol_metadata_get_art (metadata) == NULL);
  ol_metadata_free (metadata);
}

void
test_setter ()
{
  OlMetadata *metadata = ol_metadata_new ();
  set_metadata (metadata);
  ol_test_expect (strcmp (ol_metadata_get_title (metadata),
                          title) == 0);
  ol_test_expect (strcmp (ol_metadata_get_artist (metadata),
                          artist) == 0);
  ol_test_expect (strcmp (ol_metadata_get_album (metadata),
                          album) == 0);
  ol_test_expect (strcmp (ol_metadata_get_uri (metadata),
                          uri) == 0);
  ol_test_expect (ol_metadata_get_track_number (metadata) == track_number);
  ol_test_expect (ol_metadata_get_duration (metadata) == duration);
  ol_test_expect (strcmp (ol_metadata_get_art (metadata),
                          art) == 0);
  ol_metadata_set_title (metadata, title2);
  ol_metadata_set_artist (metadata, artist2);
  ol_metadata_set_album (metadata, album2);
  ol_metadata_set_uri (metadata, uri2);
  ol_test_expect (strcmp (ol_metadata_get_title (metadata),
                          title2) == 0);
  ol_test_expect (strcmp (ol_metadata_get_artist (metadata),
                          artist2) == 0);
  ol_test_expect (strcmp (ol_metadata_get_album (metadata),
                          album2) == 0);
  ol_test_expect (strcmp (ol_metadata_get_uri (metadata),
                          uri2) == 0);
  ol_test_expect (ol_metadata_get_track_number (metadata) == track_number);
  ol_metadata_set_title (metadata, NULL);
  ol_metadata_set_artist (metadata, NULL);
  ol_metadata_set_album (metadata, NULL);
  ol_metadata_set_uri (metadata, NULL);
  ol_test_expect (ol_metadata_get_title (metadata) == NULL);
  ol_test_expect (ol_metadata_get_artist (metadata) == NULL);
  ol_test_expect (ol_metadata_get_album (metadata) == NULL);
  ol_test_expect (ol_metadata_get_uri (metadata) == NULL);
  ol_metadata_free (metadata);
}

void
test_serialize ()
{
  OlMetadata *metadata = ol_metadata_new ();
  set_metadata (metadata);
  size_t size = ol_metadata_serialize (metadata, NULL, 0);
  size_t buf_size = size + 10;
  char *buf = g_new0 (char, buf_size);

  size_t real_size = ol_metadata_serialize (metadata, buf, buf_size);
  ol_test_expect (real_size == size);
  ol_test_expect (strlen (buf) == size);

  size_t less_size = size - 10;
  size_t real_less_size = ol_metadata_serialize (metadata, buf, less_size);
  ol_test_expect (real_less_size == size);
  ol_test_expect (strlen (buf) == less_size - 1);

  g_free (buf);
  ol_metadata_free (metadata);
}

void
test_deserialize ()
{
  OlMetadata *metadata1 = ol_metadata_new ();
  OlMetadata *metadata2 = ol_metadata_new ();
  set_metadata (metadata1);
  size_t buflen = ol_metadata_serialize (metadata1, NULL, 0);
  buflen++;
  char *buf = g_new (char, buflen);
  ol_metadata_serialize (metadata1, buf, buflen);
  ol_test_expect (ol_metadata_deserialize (metadata2, buf));
  memset (buf, 0, buflen);
  ol_metadata_serialize (metadata2, buf, buflen);
  ol_test_expect (ol_metadata_equal (metadata1, metadata2));
  ol_metadata_free (metadata1);
  ol_metadata_free (metadata2);
}

void
test_equal ()
{
  OlMetadata *metadata1, *metadata2;
  metadata1 = ol_metadata_new ();
  metadata2 = ol_metadata_new ();
  set_metadata (metadata1);
  ol_test_expect (ol_metadata_equal (NULL, NULL));
  ol_test_expect (!ol_metadata_equal (metadata1, NULL));
  ol_test_expect (!ol_metadata_equal (NULL, metadata2));
  ol_test_expect (!ol_metadata_equal (metadata1, metadata2));
  set_metadata (metadata2);
  ol_test_expect (ol_metadata_equal (metadata1, metadata2));
  ol_metadata_free (metadata1);
  ol_metadata_free (metadata2);
}

void
test_copy ()
{
  OlMetadata *metadata1, *metadata2;
  metadata1 = ol_metadata_new ();
  metadata2 = ol_metadata_new ();
  set_metadata (metadata1);
  ol_test_expect (!ol_metadata_equal (metadata1, metadata2));
  ol_metadata_copy (metadata2, metadata1);
  ol_test_expect (ol_metadata_equal (metadata1, metadata2));
  ol_metadata_free (metadata1);
  ol_metadata_free (metadata2);
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
