/*
 * Copyright (C) 2009  Tiger Soldier
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include "config.h"
#include "ol_osd_window.h"
#include "ol_lrc_parser.h"
#include "ol_lrc_utility.h"
#include "ol_player.h"
#include "ol_utils.h"
#include "ol_lrc_fetch.h"
#include "ol_lrc_fetch_ui.h"
#include "ol_trayicon.h"
#include "ol_intl.h"
#include "ol_config.h"
#include "ol_osd_module.h"
#include "ol_keybindings.h"
#include "ol_lrc_fetch_module.h"
#include "ol_lyric_manage.h"
#include "ol_stock.h"
#include "ol_app.h"
#include "ol_debug.h"

#define REFRESH_INTERVAL 100

static guint refresh_source = 0;
static OlPlayerController *controller = NULL;
static OlMusicInfo music_info = {0};
static gchar *previous_title = NULL;
static gchar *previous_artist = NULL;
static gchar *previous_uri = NULL;
static gint previous_duration = 0;
static gint previous_position = -1;
static LrcQueue *lrc_file = NULL;
static OlOsdModule *module = NULL;
static int fetch_id = 0;

static void initialize (int argc, char **argv);
static gint refresh_music_info (gpointer data);
static void check_music_change (int time);
static void on_music_changed (void);
static check_lyric_file (void);
/** 
 * @brief Handles SIG_CHILD for download child process
 * 
 * @param signal 
 */
/* static void child_handler (int sig); */
static void internal_search_callback (struct OlLrcFetchResult *result,
                            void *userdata);
static gboolean internal_download_callback (char *filepath);

static gboolean
internal_download_callback (char *filepath)
{
  if (filepath != NULL)
    check_lyric_file ();
  else
    ol_osd_module_download_fail_message (module, _("Download failed"));
  return FALSE;
}

static void
internal_search_callback (struct OlLrcFetchResult *result,
                void *userdata)
{
  ol_log_func ();
  ol_assert (result != NULL);
  ol_assert (result->engine != NULL);
  if (result->count > 0 && result->candidates != 0)
  {
    char *filename = ol_lyric_download_path (&result->info);
    if (filename == NULL)
    {
      ol_osd_module_download_fail_message (module, _("Cannot create the lyric directory"));
    }
    else
    {
      if (module != NULL)
        ol_osd_module_clear_message (module);
      ol_lrc_fetch_ui_show (result->engine, result->candidates, result->count, filename);
      g_free (filename);
    }
  }
  else
  {
    if (module != NULL)
      ol_osd_module_search_fail_message (module, _("Lyrics not found"));
  }
}

gboolean
ol_app_download_lyric (OlMusicInfo *music_info)
{
  OlConfig *config = ol_config_get_instance ();
  char *name = ol_config_get_string (config, "Download", "download-engine");
  fprintf (stderr, "Download engine: %s\n", name);
  OlLrcFetchEngine *engine = ol_lrc_fetch_get_engine (name);
  ol_lrc_fetch_begin_search (engine, 
                             music_info, 
                             internal_search_callback,
                             NULL);
  if (module != NULL)
    ol_osd_module_search_message (module, _("Searching lyrics"));
}

LrcQueue *
ol_app_get_current_lyric ()
{
  return lrc_file;
}

static gboolean
check_lyric_file ()
{
  gboolean ret = TRUE;
  char *filename = ol_lyric_find (&music_info);
  if (filename != NULL)
  {
    if (lrc_file != NULL)
      ol_lrc_parser_free (lrc_file);
    lrc_file = ol_lrc_parser_get_lyric_info (filename);
    ol_osd_module_set_lrc (module, lrc_file);
    ret = TRUE;
  }
  else
  {
    ret = FALSE;
  }
  return ret;
}

static void
on_music_changed ()
{
  ol_log_func ();
  if (module != NULL)
  {
    ol_osd_module_set_music_info (module, &music_info);
    ol_osd_module_set_duration (module, previous_duration);
  }
  ol_osd_module_set_lrc (module, NULL);
  if (!check_lyric_file ())
    ol_app_download_lyric (&music_info);
}

static void
check_music_change (int time)
{
  /* fprintf (stderr, "%s\n", __FUNCTION__); */
  /* checks whether the music has been changed */
  gboolean changed = FALSE;
  /* fprintf (stderr, "%d-%d\n", previous_position, time); */
  if (previous_position >=0 && time >= previous_position &&
      previous_title != NULL)
    return;
  /* compares the previous title with current title */
  if (controller && !ol_player_get_music_info (controller, &music_info))
  {
    controller = NULL;
  }
  guint duration = 0;
  if (controller && !ol_player_get_music_length (controller, &duration))
  {
    controller = NULL;
  }
  if (!ol_streq (music_info.title, previous_title))
    changed = TRUE;
  ol_strptrcpy (&previous_title, music_info.title);
  /* compares the previous artist with current  */
  if (!ol_streq (music_info.artist, previous_artist))
    changed = TRUE;
  ol_strptrcpy (&previous_artist, music_info.artist);
  if (!ol_streq (music_info.uri, previous_uri))
    changed = TRUE;
  ol_strptrcpy (&previous_uri, music_info.uri);
  /* compares the previous duration */
  /* FIXME: because the a of banshee, some lyrics may return different
     duration for the same song when plays to different position, so the
     comparison is commented out temporarily */
  /* if (previous_duration != duration) */
  /* { */
  /*   printf ("change6:%d-%d\n", previous_duration, duration); */
  /*   changed = TRUE; */
  /* } */
  previous_duration = duration;
  if (changed)
  {
    on_music_changed ();
  }
}

static gint
refresh_music_info (gpointer data)
{
  /* fprintf (stderr, "%s\n", __FUNCTION__); */
  if (controller == NULL)
  {
    controller = ol_player_get_active_player ();
    if (controller == NULL)
    {
      printf (_("No supported player is running, exit.\n"));
      gtk_main_quit ();
    }
  }
  guint time = 0;
  if (controller && !ol_player_get_played_time (controller, &time))
  {
    controller = NULL;
  }
  check_music_change (time);
  previous_position = time;
  if (controller == NULL)
  {
    previous_position = -1;
    return TRUE;
  }
  ol_osd_module_set_played_time (module, time);
  return TRUE;
}

static
void initialize (int argc, char **argv)
{
#if ENABLE_NLS
  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  /* textdomain (PACKAGE); */
#endif
  /* Handler for SIGCHLD to wait lrc downloading process */
  /* signal (SIGCHLD, child_handler); */
  
  ol_logf (OL_INFO, "main\n");
  g_thread_init(NULL);
  gtk_init (&argc, &argv);
  ol_player_init ();
  module = ol_osd_module_new ();
  ol_stock_init ();
  ol_trayicon_inital ();
  ol_keybinding_init ();
  ol_lrc_fetch_module_init ();
  ol_lrc_fetch_add_async_download_callback ((GSourceFunc) internal_download_callback);
  refresh_source = g_timeout_add (REFRESH_INTERVAL, refresh_music_info, NULL);
}

OlPlayerController*
ol_app_get_controller ()
{
  return controller;
}

OlMusicInfo*
ol_app_get_current_music ()
{
  return &music_info;
}

int
main (int argc, char **argv)
{
  initialize (argc, argv);
  gtk_main ();
  ol_player_free ();
  ol_osd_module_destroy (module);
  module = NULL;
  ol_trayicon_free ();
  return 0;
}
