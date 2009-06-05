#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include "ol_osd_window.h"
#include "ol_lrc_parser.h"
#include "ol_lrc_utility.h"
#include "ol_player.h"
#include "ol_utils.h"
#include "ol_lrc_fetch.h"
#include "ol_trayicon.h"

#define REFRESH_INTERVAL 100
#define MAX_PATH_LEN 1024
static const gchar *LRC_PATH = ".lyrics";

static OlPlayerController *controller = NULL;
static OlMusicInfo music_info = {0};
static OlOsdWindow *osd = NULL;
static gchar *previous_title = NULL;
static gchar *previous_artist = NULL;
static gint previous_duration = 0;
static gint lrc_id = -1;
static gint lrc_next_id = -1;
static gint current_line = 0;
static LrcQueue *lrc_file = NULL;

void ol_init_osd ();
gint refresh_music_info (gpointer data);
void change_music ();
gboolean is_file_exist (const char *filename);
void get_user_home_directory (char *dir);
/** 
 * Gets a music's full path filename
 * 
 * @param music_info The info of the music
 * @param pathname the returned full path filename
 */
void get_lyric_path_name (OlMusicInfo *music_info, char *pathname);
void update_osd (int time, int duration);
void update_next_lyric (LrcInfo *current_lrc);
gboolean download_lyric (OlMusicInfo *music_info);

/** 
 * @brief Gets the real lyric of the given lyric
 * A REAL lyric is the nearest lyric to the given lyric, whose text is not empty
 * If the given lyric text is not empty, the given lyric is a real lyric
 * If not real lyric available, returns NULL
 * @param lrc An LrcInfo
 * 
 * @return The real lyric of the lrc. returns NULL if not available
 */
LrcInfo* get_real_lyric (LrcInfo *lrc);
void
get_lyric_path_name (OlMusicInfo *music_info, char *pathname)
{
  if (pathname == NULL)
    return;
  char home_dir[MAX_PATH_LEN];
  if (lrc_file != NULL)
  {
    free (lrc_file);
    lrc_file = NULL;
  }
  if (previous_title == NULL)
    return;
  get_user_home_directory (home_dir);
  if (previous_artist == NULL)
  {
    sprintf (pathname, "%s/%s/%s.lrc", home_dir, LRC_PATH, previous_title);
  }
  else
  {
    sprintf (pathname, "%s/%s/%s-%s.lrc", home_dir, LRC_PATH, previous_artist, previous_title);
  }
  printf ("lrc file name:%s\n", pathname);
}

gboolean download_lyric (OlMusicInfo *music_info)
{
  int lrc_count;
  struct OlLrcCandidate *candidates = sogou.search (music_info, &lrc_count, "UTF-8");
  printf ("downloading...\n");
  if (lrc_count == 0 || candidates == NULL)
  {
    printf ("download failed\n");
    return FALSE;
  }
  else
  {
    char pathname[MAX_PATH_LEN];
    get_lyric_path_name (music_info, pathname);
    sogou.download (&candidates[0], pathname, "UTF-8");
    printf ("download %s success\n", pathname);
  }
}

LrcInfo*
get_real_lyric (LrcInfo *lrc)
{
  while (lrc != NULL)
  {
    if (!ol_is_string_empty (ol_lrc_parser_get_lyric_text (lrc)))
      break;
    lrc = ol_lrc_parser_get_next_of_lyric (lrc);
  }
  return lrc;
}

void
update_next_lyric (LrcInfo *current_lrc)
{
  LrcInfo *info = ol_lrc_parser_get_next_of_lyric (current_lrc);
  info = get_real_lyric (info);
  if (info == NULL)
  {
    if (lrc_next_id == -1)
    {
      return;
    }
    else
    {
      lrc_next_id = -1;
      ol_osd_window_set_lyric (osd, 1 - current_line, "");
    }
  }
  else
  {
    if (lrc_next_id == ol_lrc_parser_get_lyric_id (info))
      return;
    if (info != NULL)
    {
      lrc_next_id = ol_lrc_parser_get_lyric_id (info);
      ol_osd_window_set_lyric (osd, 1 - current_line,
                               ol_lrc_parser_get_lyric_text (info));
    }
  }
  ol_osd_window_set_percentage (osd, 1 - current_line, 0.0);
}

void
update_osd (int time, int duration)
{
  /* updates the time and lyrics */
  if (lrc_file != NULL && osd != NULL)
  {
    char current_lrc[1024];
    double percentage;
    int id, lyric_id;
    ol_lrc_utility_get_lyric_by_time (lrc_file, time, duration, current_lrc, &percentage, &lyric_id);
    LrcInfo *info = ol_lrc_parser_get_lyric_by_id (lrc_file, lyric_id);
    info = get_real_lyric (info);
    if (info == NULL)
      id = -1;
    else
      id = ol_lrc_parser_get_lyric_id (info);
    if (lrc_id != id)
    {
      if (id == -1)
        return;
      if (id != lrc_next_id)
      {
        current_line = 0;
        if (ol_lrc_parser_get_lyric_text (info) != NULL)
          ol_osd_window_set_lyric (osd, current_line, ol_lrc_parser_get_lyric_text (info));
        if (id != lyric_id)
          ol_osd_window_set_current_percentage (osd, 0.0);
        update_next_lyric (info);
      }
      else
      {
        ol_osd_window_set_percentage (osd, current_line, 1.0);
        current_line = 1 - current_line;
      }
      lrc_id = id;
      ol_osd_window_set_current_line (osd, current_line);
    }
    if (id == lyric_id && percentage > 0.5)
      update_next_lyric (info);
    if (id == lyric_id)
    {
      ol_osd_window_set_current_percentage (osd, percentage);
    }
    if (!GTK_WIDGET_MAPPED (GTK_WIDGET (osd)))
      gtk_widget_show (GTK_WIDGET (osd));
  }
  else
  {
    if (osd != NULL && GTK_WIDGET_MAPPED (GTK_WIDGET (osd)))
      gtk_widget_hide (GTK_WIDGET (osd));
  }
}

void
get_user_home_directory (char *dir)
{
  if (dir == NULL)
    return;
  struct passwd *buf;
  if (buf = getpwuid (getuid ()))
  {
    if (buf->pw_dir != NULL)
      strcpy (dir, buf->pw_dir);
  }
}

gboolean
is_file_exist (const char *filename)
{
  if (filename == NULL)
    return FALSE;
  struct stat buf;
  printf ("stat:%d\n", stat (filename, &buf));
  return stat (filename, &buf) == 0;
}

void
change_music ()
{
  printf ("%s\n",
          __FUNCTION__);
  lrc_id = -1;
  lrc_next_id = -1;
  current_line = 0;
  gchar file_name[MAX_PATH_LEN];
  get_lyric_path_name (&music_info, file_name);
  if (osd != NULL)
  {
    gtk_widget_hide (GTK_WIDGET (osd));
    ol_osd_window_set_lyric (osd, 0, NULL);
    ol_osd_window_set_lyric (osd, 1, NULL);
  }
  if (!is_file_exist (file_name))
  {
    if (!download_lyric (&music_info) || !is_file_exist (file_name))
    return;
  }
  lrc_file = ol_lrc_parser_get_lyric_info (file_name);
}

void
ol_init_osd ()
{
  osd = OL_OSD_WINDOW (ol_osd_window_new ());
  ol_osd_window_resize (osd, 1024, 100);
  ol_osd_window_set_alignment (osd, 0.5, 1);
  ol_osd_window_set_line_alignment (osd, 0, 0.0);
  ol_osd_window_set_line_alignment (osd, 1, 1.0);
  gtk_widget_show (GTK_WIDGET (osd));
}

gint
refresh_music_info (gpointer data)
{
  if (controller == NULL)
  {
    controller = ol_player_get_active_player ();
  }
  if (controller == NULL)
    return;
  if (!controller->get_music_info (&music_info))
  {
    controller = NULL;
  }
  guint time = 0;
  if (!controller->get_played_time (&time))
  {
    controller = NULL;
  }
  guint duration = 0;
  if (!controller->get_music_length (&duration))
  {
    controller = NULL;
  }
  /* checks whether the music has been changed */
  gboolean changed = FALSE;
  gboolean stop = FALSE;
  /* compares the previous title with current title */
  if (music_info.title == NULL)
  {
    if (previous_title != NULL)
    {
      g_free (previous_title);
      previous_title = NULL;
    }
    stop = TRUE;
  }
  else if (previous_title == NULL)
  {
    changed = TRUE;
    previous_title = g_strdup (music_info.title);
  }
  else if (strcmp (previous_title, music_info.title) != 0)
  {
    changed = TRUE;
    g_free (previous_title);
    previous_title = g_strdup (music_info.title);
  }
  /* compares the previous artist with current  */
  if (music_info.artist == NULL)
  {
    if (previous_artist != NULL)
    {
      g_free (previous_artist);
      previous_artist = NULL;
      changed = TRUE;
    }
  }
  else if (previous_artist == NULL)
  {
    changed = TRUE;
    previous_artist = g_strdup (music_info.artist);
  }
  else if (strcmp (previous_artist, music_info.artist) != 0)
  {
    printf ("duration\n");
    changed = TRUE;
    g_free (previous_artist);
    previous_artist = g_strdup (music_info.artist);
  }
  /* compares the previous duration */
  if (previous_duration != duration)
  {
    changed = TRUE;
    previous_duration = duration;
  }

  if (stop)
  {
    if (osd != NULL && GTK_WIDGET_MAPPED (osd))
      gtk_widget_hide (GTK_WIDGET (osd));
    return TRUE;
  }
  if (changed)
  {
    change_music ();
  }
  update_osd (time, duration);
  return TRUE;
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  ol_player_init ();
  ol_init_osd ();
  ol_trayicon_inital (osd);
  ol_get_string_from_hash_table (NULL, NULL);
  g_timeout_add (REFRESH_INTERVAL, refresh_music_info, NULL);
  gtk_main ();
  ol_player_free ();
  g_object_unref (osd);
  osd = NULL;
  return 0;
}
