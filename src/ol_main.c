/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009-2011  Tiger Soldier
 *
 * This file is part of OSD Lyrics.
 * OSD Lyrics is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OSD Lyrics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OSD Lyrics.  If not, see <http://www.gnu.org/licenses/>. 
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <gio/gio.h>
#include "config.h"
#include "ol_lrc.h"
#include "ol_player.h"
#include "ol_utils.h"
#include "ol_lrc_fetch.h"
#include "ol_lrc_fetch_ui.h"
#include "ol_trayicon.h"
#include "ol_intl.h"
#include "ol_config.h"
#include "ol_consts.h"
#include "ol_display_module.h"
#include "ol_keybindings.h"
#include "ol_lrc_fetch_module.h"
#include "ol_lyric_manage.h"
#include "ol_stock.h"
#include "ol_app.h"
#include "ol_notify.h"
#include "ol_lrclib.h"
#include "ol_debug.h"
#include "ol_singleton.h"
#include "ol_player_chooser.h"

#define REFRESH_INTERVAL 100
#define INFO_INTERVAL 500
#define TIMEOUT_WAIT_LAUNCH 5000
#define LRCDB_FILENAME "lrc.db"

gboolean _arg_debug_cb (const gchar *option_name,
                        const gchar *value,
                        gpointer data,
                        GError **error);
static gboolean _arg_version;

static GOptionEntry cmdargs[] =
{
  { "debug", 'd', G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_CALLBACK, _arg_debug_cb,
    N_ ("The level of debug messages to log, can be 'none', 'error', 'debug', or 'info'"), "level" },
  { "version", 'v', 0, G_OPTION_ARG_NONE, &_arg_version,
    N_ ("Show version information"), NULL},
  { NULL }
};

static guint name_watch = 0;
static guint position_timer = 0;
static OlPlayer *player = NULL;
static OlMusicInfo *metadata = NULL;
static struct OlLrc *lrc_file = NULL;
static char *display_mode = NULL;
static struct OlDisplayModule *module = NULL;
static int search_id = -1;
static gboolean initialized = FALSE;
static enum _PlayerLostAction {
  ACTION_NONE = 0,
  ACTION_LAUNCH_DEFAULT,
  ACTION_CHOOSE_PLAYER,
  ACTION_WAIT_LAUNCH,
  ACTION_QUIT,
} player_lost_action = ACTION_LAUNCH_DEFAULT;

static void _initialize (int argc, char **argv);
static void _wait_for_player_launch (void);
static void _player_chooser_response_cb (GtkDialog *dialog,
                                         gint response_id,
                                         gpointer user_data);
static void _search_callback (struct OlLrcFetchResult *result,
                            void *userdata);
static void _download_callback (struct OlLrcDownloadResult *result);
static void _on_config_changed (OlConfig *config,
                                gchar *group,
                                gchar *name,
                                gpointer userdata);
static void _init_dbus_connection (void);
static void _init_dbus_connection_done (void);
static void _name_appeared_cb (GDBusConnection *connection,
                               const gchar *name,
                               const gchar *name_owner,
                               gpointer user_data);
static void _name_vanished_cb (GDBusConnection *connection,
                               const gchar *name,
                               gpointer user_data);
static void _start_daemon_cb (GObject *source_object,
                              GAsyncResult *res,
                              gpointer user_data);
static void _track_changed_cb (void);
static void _status_changed_cb (void);

static void
_on_config_changed (OlConfig *config,
                    gchar *group,
                    gchar *name,
                    gpointer userdata)
{
  if (strcmp (name, "display-mode") == 0)
  {
    char *mode = ol_config_get_string (config, group, name);
    if (display_mode == NULL ||
        ol_stricmp (mode, display_mode, -1) != 0)
    {
      if (display_mode != NULL)
        g_free (display_mode);
      display_mode = g_strdup (mode);
      ol_display_module_free (module);
      module = ol_display_module_new (display_mode, player);
      ol_display_module_set_lrc (module, lrc_file);
    }
    g_free (mode);
  }
}

static void
_download_callback (struct OlLrcDownloadResult *result)
{
  ol_log_func ();
  if (result->filepath != NULL)
    ol_app_assign_lrcfile (result->info, result->filepath, TRUE);
  else
    ol_display_module_download_fail_message (module, _("Download failed"));
}

static void
_search_msg_callback (int _search_id,
                      enum OlLrcSearchMsgType msg_type,
                      const char *message,
                      void *userdata)
{
  ol_assert (_search_id == search_id);
  switch (msg_type)
  {
  case OL_LRC_SEARCH_MSG_ENGINE:
    if (module != NULL)
    {
      char *msg = g_strdup_printf (_("Searching lyrics from %s"), _(message));
      ol_display_module_search_fail_message (module, msg);
      g_free (msg);
    }
    break;
  }
}


static void
_search_callback (struct OlLrcFetchResult *result,
                  void *userdata)
{
  ol_log_func ();
  ol_assert (result != NULL);
  ol_assert (result->engine != NULL);
  ol_assert (result->id == search_id);
  search_id = -1;
  if (result->count > 0 && result->candidates != 0)
  {
    char *filename = ol_lyric_download_path (&result->info);
    if (filename == NULL)
    {
      ol_display_module_download_fail_message (module, _("Cannot create the lyric directory"));
    }
    else
    {
      if (module != NULL) {
        ol_display_module_clear_message (module);
      }
      ol_lrc_fetch_ui_show (result->engine, result->candidates, result->count,
                            &result->info,
                            filename);
      g_free (filename);
    }
  }
  else
  {
    if (module != NULL)
      ol_display_module_search_fail_message (module, _("Lyrics not found"));
  }
}

gboolean
ol_app_download_lyric (OlMusicInfo *music_info)
{
  ol_log_func ();
  if (search_id > 0)
    ol_lrc_fetch_cancel_search (search_id);
  OlConfig *config = ol_config_get_instance ();
  char **engine_list = ol_config_get_str_list (config,
                                               "Download",
                                               "download-engine",
                                               NULL);
  search_id = ol_lrc_fetch_begin_search (engine_list,
                                         music_info,
                                         _search_msg_callback,
                                         _search_callback,
                                         NULL);
  g_strfreev (engine_list);
  return TRUE;
}

struct OlLrc *
ol_app_get_current_lyric ()
{
  ol_log_func ();
  return lrc_file;
}

gboolean
ol_app_assign_lrcfile (const OlMusicInfo *info,
                       const char *filepath,
                       gboolean update)
{
  ol_log_func ();
  ol_assert_ret (info != NULL, FALSE);
  ol_assert_ret (filepath == NULL || ol_path_is_file (filepath), FALSE);
  if (update)
  {
    ol_lrclib_assign_lyric (info, filepath);
  }
  if (ol_music_info_equal (metadata, info))
  {
    if (lrc_file != NULL)
    {
      ol_lrc_free (lrc_file);
      lrc_file = NULL;
    }
    if (filepath != NULL)
      lrc_file = ol_lrc_new (filepath);
    ol_display_module_set_lrc (module, lrc_file);
  }
  return TRUE;
}

static gboolean
_check_lyric_file ()
{
  ol_log_func ();
  gboolean ret = TRUE;
  char *filename = NULL;
  int code = ol_lrclib_find (metadata, &filename);
  if (code == 0)
    filename = ol_lyric_find (metadata);
 
  if (filename != NULL)
  {
    ret = ol_app_assign_lrcfile (metadata, filename, code == 0);
    g_free (filename);
  }
  else
  {
    ol_debugf("filename;%s\n", filename);
    if (code == 0) ret = FALSE;
  }
  return ret;
}

static void
_track_changed_cb (void)
{
  ol_log_func ();
  ol_display_module_set_lrc (module, NULL);
  ol_player_get_metadata (player, metadata);
  if (!_check_lyric_file () &&
      !ol_is_string_empty (ol_music_info_get_title (metadata)))
    ol_app_download_lyric (metadata);
  OlConfig *config = ol_config_get_instance ();
  if (ol_config_get_bool (config, "General", "notify-music"))
    ol_notify_music_change (metadata, ol_player_get_icon_path (player));
}

static void
_status_changed_cb (void)
{
  ol_log_func ();
  ol_trayicon_status_changed (ol_player_get_status (player));
}

static gboolean
_player_launch_timeout (gpointer userdata)
{
  if (player_lost_action == ACTION_WAIT_LAUNCH)
    player_lost_action = ACTION_CHOOSE_PLAYER;
  return FALSE;
}

static void
_wait_for_player_launch (void)
{
  player_lost_action = ACTION_WAIT_LAUNCH;
  g_timeout_add (TIMEOUT_WAIT_LAUNCH, _player_launch_timeout, NULL);
}

static void
_player_chooser_response_cb (GtkDialog *dialog,
                             gint response_id,
                             gpointer user_data)
{
  ol_assert (GTK_IS_DIALOG (dialog));
  switch (response_id)
  {
  case OL_PLAYER_CHOOSER_RESPONSE_LAUNCH:
    _wait_for_player_launch ();
    break;
  case GTK_RESPONSE_DELETE_EVENT:
  case GTK_RESPONSE_CLOSE:
    gtk_main_quit ();
    break;
  default:
    ol_errorf ("Unknown response id: %d\n", response_id);
  }
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
_player_lost_cb (void)
{
  switch (player_lost_action)
  {
  case ACTION_LAUNCH_DEFAULT:
  case ACTION_CHOOSE_PLAYER:
  {
    if (player_lost_action == ACTION_LAUNCH_DEFAULT)
    {
      OlConfig *config = ol_config_get_instance ();
      char *player_cmd = ol_config_get_string (config,
                                               "General",
                                               "startup-player");
      if (!ol_is_string_empty (player_cmd))
      {
        ol_debugf ("Running %s\n", player_cmd);
        ol_launch_app (player_cmd);
        _wait_for_player_launch ();
        g_free (player_cmd);
        break;
      }
      else
      {
        g_free (player_cmd);
      }
    }
    /* TODO: */
    /* GList *supported_players = ol_player_get_support_players (); */
    GList *supported_players = NULL;
    GtkWidget *player_chooser = ol_player_chooser_new (supported_players);
    g_signal_connect (player_chooser,
                      "response",
                      G_CALLBACK (_player_chooser_response_cb),
                      NULL);
    gtk_widget_show (player_chooser);
    player_lost_action = ACTION_NONE;
    break;
  }
  case ACTION_QUIT:
    printf (_("No supported player is running, exit.\n"));
    gtk_main_quit ();
    break;
  default:
    break;
  }
}

static gint
_update_position (gpointer data)
{
  ol_log_func ();
  guint64 time = 0;
  ol_player_get_position (player, &time);
  ol_display_module_set_played_time (module, time);
  return TRUE;
}

OlPlayer*
ol_app_get_player ()
{
  return player;
}

OlMusicInfo*
ol_app_get_current_music ()
{
  return metadata;
}

void
ol_app_adjust_lyric_offset (int offset_ms)
{
  ol_log_func ();
  struct OlLrc *lrc = ol_app_get_current_lyric ();
  if (lrc == NULL)
    return;
  int old_offset = ol_lrc_get_offset (lrc);
  int new_offset = old_offset - offset_ms;
  ol_lrc_set_offset (lrc, new_offset);
}

static void
_parse_cmd_args (int *argc, char ***argv)
{
  ol_log_func ();
  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new ("- Display your lyrics");
  g_option_context_add_main_entries (context, cmdargs, PACKAGE);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  if (!g_option_context_parse (context, argc, argv, &error))
  {
    ol_errorf ("option parsing failed: %s\n", error->message);
  }
  if (_arg_version)
  {
    printf ("%s %s\n", PROGRAM_NAME, VERSION);
    exit (0);
  }
  g_option_context_free (context);
}

gboolean
_arg_debug_cb (const gchar *option_name,
               const gchar *value,
               gpointer data,
               GError **error)
{
  if (value == NULL)
    value = "debug";
  if (strcmp (value, "none") == 0)
  {
    ol_log_set_level (OL_LOG_NONE);
  }
  else if (strcmp (value, "error") == 0)
  {
    ol_log_set_level (OL_ERROR);
  }
  else if (strcmp (value, "debug") == 0)
  {
    ol_log_set_level (OL_DEBUG);
  }
  else if (strcmp (value, "info") == 0)
  {
    ol_log_set_level (OL_INFO);
  }
  else
  {
    g_set_error_literal (error, g_quark_from_string (PACKAGE_NAME), 1,
                         N_ ("debug level should be one of ``none'', ``error'', ``debug'', or ``info''"));
    return FALSE;
  }
  return TRUE;
}

static void
_initialize (int argc, char **argv)
{
  ol_log_func ();
#if ENABLE_NLS
  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  /* textdomain (PACKAGE); */
#endif

  g_thread_init(NULL);
  gtk_init (&argc, &argv);
  _parse_cmd_args (&argc, &argv);
  g_set_prgname (_(PROGRAM_NAME));
  if (ol_is_running ())
  {
    printf ("%s\n", _("Another OSD Lyrics is running, exit."));
    exit (0);
  }
  OlConfig *config = ol_config_get_instance ();
  g_signal_connect (config, "changed",
                    G_CALLBACK (_on_config_changed),
                    NULL);
  initialized = FALSE;
  metadata = ol_music_info_new ();
  _init_dbus_connection ();
  ol_stock_init ();
  ol_display_module_init ();
  display_mode = ol_config_get_string (config, "General", "display-mode");
  module = ol_display_module_new (display_mode, player);
  ol_trayicon_init ();
  ol_notify_init ();
  ol_keybinding_init ();
  ol_lrc_fetch_module_init ();
  char *lrcdb_file = g_strdup_printf ("%s/%s/%s",
                                      g_get_user_config_dir (),
                                      PACKAGE_NAME,
                                      LRCDB_FILENAME);
  if (!ol_lrclib_init (lrcdb_file))
  {
    ol_error ("Initialize lrclib failed");
  }
  g_free (lrcdb_file);
  ol_lrc_fetch_add_async_download_callback (_download_callback);
}

static void
_name_appeared_cb (GDBusConnection *connection,
                   const gchar *name,
                   const gchar *name_owner,
                   gpointer user_data)
{
  ol_debug ("Daemon appeared");
  if (!initialized)
    _init_dbus_connection_done ();
}

static void
_name_vanished_cb (GDBusConnection *connection,
                   const gchar *name,
                   gpointer user_data)
{
  ol_debugf ("Daemon lost, try to activate it\n");
  g_dbus_connection_call (connection,
                          "org.freedesktop.DBus",  /* bus name */
                          "/org/freedesktop/DBus", /* object path */
                          "org.freedesktop.DBus",  /* interface name */
                          "StartServiceByName",    /* method name */
                          g_variant_new ("(su)", OL_SERVICE_DAEMON, 0),
                          G_VARIANT_TYPE ("(u)"),
                          G_DBUS_CALL_FLAGS_NONE,
                          -1,
                          NULL,
                          (GAsyncReadyCallback) _start_daemon_cb,
                          NULL);
  ol_debugf ("Daemon appeared\n");
}

static void
_start_daemon_cb (GObject *source_object,
                  GAsyncResult *res,
                  gpointer user_data)
{
  GVariant *result;
  GError *error;
  result = g_dbus_connection_call_finish (G_DBUS_CONNECTION (source_object),
                                          res,
                                          &error);
  if (result)
  {
    guint32 start_service_result;
    g_variant_get (result, "(u)", &start_service_result);

    if (start_service_result != 1 || /* DBUS_START_REPLY_SUCCESS */
        start_service_result != 2) /* DBUS_START_REPLY_ALREADY_RUNNING */
    {
      ol_errorf ("Unexpected reply %d from StartServiceByName() method",
                 start_service_result);
    }
    /* We should do nothing, _name_appeared_cb will be called by name watch */
  }
  else
  {
    ol_errorf ("Unable to start daemon: %s\n", error->message);
    g_error_free (error);
    /* TODO: notify the user that there is a fatal error */
  }
}

static void
_init_dbus_connection (void)
{
  player = ol_player_new ();
  g_object_ref_sink (player);
  g_signal_connect (player,
                    "track-changed",
                    _track_changed_cb,
                    NULL);
  g_signal_connect (player,
                    "status-changed",
                    _status_changed_cb,
                    NULL);
  /* Activate the daemon */
  name_watch = g_bus_watch_name (G_BUS_TYPE_SESSION,
                                 OL_SERVICE_DAEMON,
                                 G_BUS_NAME_WATCHER_FLAGS_NONE,
                                 _name_appeared_cb,
                                 _name_vanished_cb,
                                 NULL,  /* user_data */
                                 NULL); /* user_data_free_func */
}

static void
_init_dbus_connection_done (void)
{
  if (ol_player_is_connected (player))
  {
    player_lost_action = ACTION_QUIT;
  }
  else
  {
    _player_lost_cb ();
  }
  initialized = TRUE;
  position_timer = g_timeout_add (REFRESH_INTERVAL,
                                  _update_position,
                                  NULL);
  _track_changed_cb ();
  _status_changed_cb ();
}

int
main (int argc, char **argv)
{
  _initialize (argc, argv);
  gtk_main ();
  g_signal_handlers_disconnect_by_func (player, _status_changed_cb, NULL);
  g_signal_handlers_disconnect_by_func (player, _track_changed_cb, NULL);
  g_object_unref (player);
  player = NULL;
  ol_music_info_free (metadata);
  metadata = NULL;
  ol_notify_unload ();
  ol_display_module_free (module);
  display_mode = NULL;
  module = NULL;
  ol_display_module_unload ();
  ol_trayicon_free ();
  ol_lrclib_unload ();
  ol_config_unload ();
  ol_lrc_fetch_module_unload (); 
  return 0;
}
