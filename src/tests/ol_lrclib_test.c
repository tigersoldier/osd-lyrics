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

OlMusicInfo *
prepare_music ()
{
  OlMusicInfo *info = NULL;
  info = ol_music_info_new ();
  ol_music_info_set_title (info, TITLE);
  ol_music_info_set_artist (info, ARTIST);
  ol_music_info_set_album (info, ALBUM);
  ol_music_info_set_uri (info, URI);
  ol_music_info_set_track_number (info, TRACK);
  return info;
}

void
basic_test ()
{
  int ret = ol_lrclib_init (DB_FILE);
  ol_test_expect (ret != 0);
  OlMusicInfo *info = prepare_music ();
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH) != 0);
  char *path;
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  ol_music_info_set_uri (info, "URI 2");
  ol_test_expect (ol_lrclib_find (info, &path) == 0);
  printf ("path is: %s\n", path);
  ol_music_info_set_uri (info, NULL);
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
  OlMusicInfo *info = prepare_music ();
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
  OlMusicInfo *info = prepare_music ();
  char *path = NULL;
  /* Music info with URI */
  ol_test_expect (ol_lrclib_find (info, &path) == 0);
  
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);

  ol_music_info_set_title (info, NULL);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH2) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH2) == 0);
  g_free (path);
  /* Music info without URI */
  ol_music_info_destroy (info);
  info = prepare_music ();
  ol_music_info_set_uri (info, NULL);
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
  ol_music_info_destroy (info);
  info = prepare_music ();
  ol_music_info_set_uri (info, NULL);
  ol_music_info_set_album (info, NULL);
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
  ol_music_info_destroy (info);
  info = prepare_music ();
  ol_music_info_set_uri (info, NULL);
  ol_music_info_set_title (info, NULL);
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
  OlMusicInfo *info = prepare_music ();
  char *path = NULL;
  /* Music info with URI */
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  ol_music_info_set_title (info, "");
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  ol_music_info_set_artist (info, "");
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);
  ol_music_info_set_album (info, "");
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH) == 0);
  g_free (path);

  ol_music_info_destroy (info);
  info = prepare_music ();
  ol_music_info_set_uri (info, URI2);
  ol_test_expect (ol_lrclib_find (info, &path) == 0);

  ol_music_info_set_uri (info, NULL);
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
  OlMusicInfo *info = prepare_music ();
  ol_music_info_set_uri (info, NULL);
  /* Music info with URI */
  ol_test_expect (ol_lrclib_assign_lyric (info, PATH_NOURI) != 0);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH_NOURI) == 0);
  g_free (path);

  ol_music_info_set_uri (info, URI);
  ol_test_expect (ol_lrclib_find (info, &path) != 0);
  ol_test_expect (strcmp (path, PATH_NOURI) == 0);
  g_free (path);

  ol_music_info_set_artist (info, "no artist");
  ol_test_expect (ol_lrclib_find (info, &path) == 0);
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
}
