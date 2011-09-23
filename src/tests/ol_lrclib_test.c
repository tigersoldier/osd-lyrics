#include <ol_lrclib.h>
#include "ol_test_util.h"
#include "ol_utils.h"

const char *DB_FILE = "/tmp/lrclib_test.db";
const char *TITLE = "Test title";
const char *ALBUM = "Test album";
const char *ARTIST = "Test artist";
const int TRACK = 19870119;
const char *URI = "Test uri";
const char *URI2 = "Test uri2";
const char *PATH = "file:///path/to/lrc";
const char *PATH2 = "file:///path/to/lrc2";
const char *PATH_NOURI = "file:///path/to/lrc/no/uri";
const char *PATH_NOURI2 = "file:///path/to/lrc/no/uri2";
const char *PATH_NOALBUM = "file:///path/to/lrc/no/artist";
const char *PATH_NOALBUM2 = "file:///path/to/lrc/no/artist2";

OlMetadata *
prepare_music ()
{
  OlMetadata *info = NULL;
  info = ol_metadata_new ();
  ol_metadata_set_title (info, TITLE);
  ol_metadata_set_artist (info, ARTIST);
  ol_metadata_set_album (info, ALBUM);
  ol_metadata_set_uri (info, URI);
  ol_metadata_set_track_number (info, TRACK);
  return info;
}

void
basic_test ()
{
  int ret = ol_lrclib_init (DB_FILE);
  ol_test_expect (ret != 0);
  OlMetadata *info = prepare_music ();
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH) != 0);
  char *path;
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  ol_metadata_set_uri (info, "URI 2");
  ol_test_expect (ol_lrclib_find (info, &path) == 0);
  printf ("path is: %s\n", path);
  ol_metadata_set_uri (info, NULL);
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH_NOURI) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH_NOURI) == 0);
  g_free (path);
  ol_lrclib_unload ();
  unlink (DB_FILE);
}

void
create_db_test ()
{
  unlink (DB_FILE);
  ol_test_expect (ol_lrclib_init (DB_FILE));
  ol_lrclib_unload ();
  unlink (DB_FILE);
}

void
db_exist_test ()
{
  OlMetadata *info = prepare_music ();
  char *path = NULL;
  /* Music info with URI */
  unlink (DB_FILE);
  ol_test_expect (ol_lrclib_init (DB_FILE));
  ol_test_expect (ol_lrclib_find (info, &path) == 0);
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  ol_lrclib_unload ();

  ol_test_expect (ol_lrclib_init (DB_FILE));
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  ol_lrclib_unload ();
  unlink (DB_FILE);
}

void
assign_lrc_test ()
{
  unlink (DB_FILE);
  ol_test_expect (ol_lrclib_init (DB_FILE));
  OlMetadata *info = prepare_music ();
  char *path = NULL;
  /* Music info with URI */
  ol_test_expect (ol_lrclib_find (info, &path) == 0);
  
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);

  ol_metadata_set_title (info, NULL);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH2) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH2) == 0);
  g_free (path);
  /* Music info without URI */
  ol_metadata_free (info);
  info = prepare_music ();
  ol_metadata_set_uri (info, NULL);
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH_NOURI) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH_NOURI) == 0);
  g_free (path);

  ol_test_expect (ol_lrclib_assign_lyric (info, PATH_NOURI2) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH_NOURI2) == 0);
  g_free (path);

  ol_test_expect (ol_lrclib_assign_lyric (info, NULL) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (path == NULL);
  /* Music info with part of members */
  ol_metadata_free (info);
  info = prepare_music ();
  ol_metadata_set_uri (info, NULL);
  ol_metadata_set_album (info, NULL);
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH_NOALBUM) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH_NOALBUM) == 0);
  g_free (path);

  ol_test_expect (ol_lrclib_assign_lyric (info, PATH_NOALBUM2) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH_NOALBUM2) == 0);
  g_free (path);

  ol_test_expect (ol_lrclib_assign_lyric (info, NULL) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (path == NULL);
  /* Music info with out title and uri */
  ol_metadata_free (info);
  info = prepare_music ();
  ol_metadata_set_uri (info, NULL);
  ol_metadata_set_title (info, NULL);
  ol_test_expect (ol_lrclib_assign_lyric (info, NULL) == 0);
  ol_test_expect (ol_lrclib_find (info, &path) == 0);
  
  ol_lrclib_unload ();
  unlink (DB_FILE);
}

void
query_by_uri_test ()
{
  unlink (DB_FILE);
  ol_test_expect (ol_lrclib_init (DB_FILE));
  OlMetadata *info = prepare_music ();
  char *path = NULL;
  /* Music info with URI */
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  ol_metadata_set_title (info, "");
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  ol_metadata_set_artist (info, "");
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  ol_metadata_set_album (info, "");
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);

  ol_metadata_free (info);
  info = prepare_music ();
  ol_metadata_set_uri (info, URI2);
  ol_test_expect (ol_lrclib_find (info, &path) == 0);

  ol_metadata_set_uri (info, NULL);
  ol_test_expect (ol_lrclib_find (info, &path) == 0);
  ol_lrclib_unload ();
  unlink (DB_FILE);
}

void
query_by_info_test ()
{
  unlink (DB_FILE);
  ol_test_expect (ol_lrclib_init (DB_FILE));
  char *path = NULL;
  OlMetadata *info = prepare_music ();
  ol_metadata_set_uri (info, NULL);
  /* Music info with URI */
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH_NOURI) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH_NOURI) == 0);
  g_free (path);

  ol_metadata_set_uri (info, URI);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH_NOURI) == 0);
  g_free (path);

  ol_metadata_set_artist (info, "no artist");
  ol_test_expect (ol_lrclib_find (info, &path) == 0);
  ol_lrclib_unload ();
  
  unlink (DB_FILE);
}

void
escape_test ()
{
  unlink (DB_FILE);
  ol_test_expect (ol_lrclib_init (DB_FILE));
  char *path = NULL;
  char *ESCAPE_PATH = "p'a't'h";
  OlMetadata *info = ol_metadata_new ();
  ol_metadata_set_uri (info, "U'R'I");
  ol_metadata_set_title (info, "t'i't'l'e");
  ol_metadata_set_artist (info, "a'r't'i's't");
  ol_metadata_set_album (info, "a'l'b'u'm");
  /* Music info with URI */
  ol_test_expect (ol_lrclib_assign_lyric (info, ESCAPE_PATH) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, ESCAPE_PATH) == 0);
  g_free (path);

  ol_metadata_set_uri (info, NULL);
  ol_test_expect (ol_lrclib_find (info, &path) == 0);
  ol_test_expect (ol_lrclib_assign_lyric (info, ESCAPE_PATH) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, ESCAPE_PATH) == 0);
  g_free (path);

  ol_lrclib_unload ();
  
  unlink (DB_FILE);
}

int main ()
{
  basic_test ();
  db_exist_test ();
  assign_lrc_test ();
  query_by_uri_test ();
  query_by_info_test ();
  escape_test ();
}
