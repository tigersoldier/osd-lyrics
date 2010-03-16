#include <stdio.h>
#include <stdlib.h>

#include "ol_player_moc.h"
#include "ol_player.h"
#include "ol_utils_cmdline.h"
#include "ol_utils.h"
#include "ol_elapse_emulator.h"
#include "ol_debug.h"

static const char *ACTIVATED_CMD = "pgrep mocp";
static const char *PLAY_CMD = "mocp -p";
static const char *STOP_CMD = "mocp -s";
static const char *PAUSE_CMD = "mocp -P";
static const char *RESUME_CMD = "mocp -U";
static const char *PREV_CMD = "mocp -r";
static const char *NEXT_CMD = "mocp -f";
static const char *QUERY_CMD = "mocp -Q \"%s\"";
static const char *QUERY_STATE = "%state";
static const char *QUERY_URI = "%file";
static const char *QUERY_POS = "%cs";
static const char *QUERY_DURATION = "%ts";
static const char *QUERY_TITLE = "%song";
static const char *QUERY_ARTIST = "%artist";
static const char *QUERY_ALBUM = "%album";
static const char *STATE_PLAY = "PLAY";
static const char *STATE_STOP = "STOP";
static const char *STATE_PAUSE = "PAUSE";

static OlElapseEmulator *elapse_emulator = NULL;

static gboolean ol_player_moc_get_music_info (OlMusicInfo *info);
static gboolean ol_player_moc_get_played_time (int *played_time);
static gboolean ol_player_moc_get_music_length (int *len);
static gboolean ol_player_moc_get_activated ();
static enum OlPlayerStatus ol_player_moc_get_status ();
static int ol_player_moc_get_capacity ();
static gboolean ol_player_moc_play ();
static gboolean ol_player_moc_pause ();
static gboolean ol_player_moc_stop ();
static gboolean ol_player_moc_prev ();
static gboolean ol_player_moc_next ();
static gboolean ol_player_moc_seek (int pos_ms);
static void ensure_elapse (int elapsed_time);

static gboolean
ol_player_moc_get_activated ()
{
  /* ol_log_func (); */
  return ol_cmd_exec (ACTIVATED_CMD);
}

static gboolean
ol_player_moc_get_music_info (OlMusicInfo *info)
{
  ol_log_func ();
  ol_assert_ret (info != NULL, FALSE);
  if (!ol_player_moc_get_activated ())
    return FALSE;
  int status = ol_player_moc_get_status ();
  if (status != OL_PLAYER_PLAYING && status != OL_PLAYER_PAUSED)
  {
    ol_music_info_clear (info);
    return TRUE;
  }
  char *format = g_strdup_printf ("%s\\n%s\\n%s\\n%s",
                                  QUERY_TITLE,
                                  QUERY_ARTIST,
                                  QUERY_ALBUM,
                                  QUERY_URI);
  char *cmd = g_strdup_printf (QUERY_CMD, format);
  g_free (format);
  char *output = NULL;
  gboolean ret = ol_cmd_get_string (cmd, &output);
  g_free (cmd);
  if (output == NULL)
  {
    ret = FALSE;
  }
  else if (ret)
  {
    /* ol_debugf ("  output:\n%s", output); */
    ol_music_info_clear (info);
    char *current = output;
    char *next = ol_split_a_line (current);
    info->title = g_strdup (ol_trim_string (current));
    if (next != NULL)
    {
      current = next;
      next = ol_split_a_line (current);
      info->artist = g_strdup (ol_trim_string (current));
    }
    if (next != NULL)
    {
      current = next;
      next = ol_split_a_line (current);
      info->album = g_strdup (ol_trim_string (current));
    }
    if (next != NULL)
    {
      current = next;
      next = ol_split_a_line (current);
      info->uri = g_strdup (ol_trim_string (current));
    }
    ol_debugf("  artist:%s\n"
              "  title:%s\n"
              "  album:%s\n"
              "  track:%d\n"
              "  uri:%s\n",
              info->artist,
              info->title,
              info->album,
              info->track_number,
              info->uri);
  }
  if (output != NULL)
    g_free (output);
  return ret;
}

static gboolean
ol_player_moc_get_music_length (int *len)
{
  ol_assert_ret (len != NULL, FALSE);
  if (!ol_player_moc_get_activated ())
    return FALSE;
  int status;
  if (status != OL_PLAYER_PLAYING && status != OL_PLAYER_PAUSED)
  {
    *len = -1;
    return TRUE;
  }
  char *cmd = g_strdup_printf (QUERY_CMD, QUERY_DURATION);
  gboolean ret = ol_cmd_get_int (cmd, len);
  *len *= 1000;
  g_free (cmd);
  return ret;
}

static void
ensure_elapse (int elapsed_time)
{
  if (elapse_emulator == NULL)
  {
    elapse_emulator = g_new (OlElapseEmulator, 1);
    if (elapse_emulator != NULL)
      ol_elapse_emulator_init (elapse_emulator, elapsed_time, 1000);
  }
}

static gboolean
ol_player_moc_get_played_time (int *played_time)
{
  /* ol_log_func (); */
  ol_assert_ret (played_time != NULL, FALSE);
  if (!ol_player_moc_get_activated ())
    return FALSE;
  int status = ol_player_moc_get_status ();
  if (status != OL_PLAYER_PLAYING && status != OL_PLAYER_PAUSED)
  {
    *played_time = -1;
    return TRUE;
  }
  char *cmd = g_strdup_printf (QUERY_CMD, QUERY_POS);
  int moc_time;
  gboolean ret = ol_cmd_get_int (cmd, &moc_time);
  moc_time *= 1000;
  ensure_elapse (moc_time);
  if (status == OL_PLAYER_PLAYING)
    *played_time = ol_elapse_emulator_get_real_ms (elapse_emulator,
                                                   moc_time);
  else if (status == OL_PLAYER_PAUSED)
    *played_time = ol_elapse_emulator_get_last_ms (elapse_emulator,
                                                   moc_time);
  g_free (cmd);
  return ret;
}

static enum OlPlayerStatus
ol_player_moc_get_status ()
{
  if (!ol_player_moc_get_activated ())
    return OL_PLAYER_ERROR;
  char *output = NULL;
  char *cmd = g_strdup_printf (QUERY_CMD, QUERY_STATE);
  int result = ol_cmd_get_string (cmd, &output);
  int ret;
  if (output != NULL && result)
  {
    ol_split_a_line (output);
    char *stat = ol_trim_string (output);
    if (stat != NULL)
    {
      /* ol_debugf ("stat = %s\n", stat); */
      if (strcmp (stat, STATE_PLAY) == 0)
        ret = OL_PLAYER_PLAYING;
      else if (strcmp (stat, STATE_PAUSE) == 0)
        ret = OL_PLAYER_PAUSED;
      else if (strcmp (stat, STATE_STOP) == 0)
        ret = OL_PLAYER_STOPPED;
      else
        ret = OL_PLAYER_UNKNOWN;
    }
    else
    {
      ret = OL_PLAYER_ERROR;
    }
  }
  else
  {
    ret = OL_PLAYER_ERROR;
  }
  if (output != NULL)
    g_free (output);
  return ret;
}

static int
ol_player_moc_get_capacity ()
{
  return OL_PLAYER_STATUS | OL_PLAYER_PLAY | OL_PLAYER_PAUSE |
    /* OL_PLAYER_STOP | */ OL_PLAYER_PREV | OL_PLAYER_NEXT;
}

static gboolean
ol_player_moc_play ()
{
  if (!ol_player_moc_get_activated ())
    return FALSE;
  int status = ol_player_moc_get_status ();
  if (status == OL_PLAYER_PAUSED)
    return ol_cmd_exec (RESUME_CMD);
  else
    return ol_cmd_exec (PLAY_CMD);
}

static gboolean
ol_player_moc_pause ()
{
  if (!ol_player_moc_get_activated ())
    return FALSE;
  return ol_cmd_exec (PAUSE_CMD);
}

static gboolean
ol_player_moc_stop ()
{
  if (!ol_player_moc_get_activated ())
    return FALSE;
  return ol_cmd_exec (STOP_CMD);
}

static gboolean
ol_player_moc_prev ()
{
  if (!ol_player_moc_get_activated ())
    return FALSE;
  return ol_cmd_exec (PREV_CMD);
}

static gboolean
ol_player_moc_next ()
{
  if (!ol_player_moc_get_activated ())
    return FALSE;
  return ol_cmd_exec (NEXT_CMD);
}

struct OlPlayer*
ol_player_moc_get ()
{
  ol_log_func ();
  struct OlPlayer *controller = ol_player_new ("MOC");
  ol_player_set_cmd (controller, "mocp -S");
  controller->get_music_info = ol_player_moc_get_music_info;
  controller->get_activated = ol_player_moc_get_activated;
  controller->get_played_time = ol_player_moc_get_played_time;
  controller->get_music_length = ol_player_moc_get_music_length;
  controller->get_capacity = ol_player_moc_get_capacity;
  controller->get_status = ol_player_moc_get_status;
  controller->play = ol_player_moc_play;
  controller->pause = ol_player_moc_pause;
  controller->stop = ol_player_moc_stop;
  controller->prev = ol_player_moc_prev;
  controller->next = ol_player_moc_next;
  controller->seek = NULL;
  return controller;
}
