#include <libnotify/notify.h>
#include "config.h"
#include "ol_notify.h"
#include "ol_intl.h"
#include "ol_debug.h"

static const char *UNKNOWN_TITLE = N_("Unknown title");
static const char *UNKNOWN_ARTIST = N_("Unknown artist");
static const char *INFO_FORMAT = "%s";
static const char *INFO_FORMAT_ALBUM = "%s\n<i>%s</i>";
static const int DEFAULT_TIMEOUT = -1;

NotifyNotification *notify = NULL;

static gboolean
internal_init ()
{
  if (notify == NULL && notify_init (PACKAGE_NAME))
  {
    notify = notify_notification_new ("",    /* summary */
                                      NULL,  /* body */
                                      NULL,  /* icon */
                                      NULL); /* attach */
  }
  if (notify == NULL)
    ol_error ("Notify init failed");
  return notify != NULL;
}

void
ol_notify_init ()
{
  internal_init ();
}

void
ol_notify_unload ()
{
  notify_uninit ();
}

void
ol_notify_music_change (OlMusicInfo *info)
{
  ol_assert (info != NULL);
  if (!internal_init ())
  {
    return;
  }
  const char *title = ol_music_info_get_title (info);
  const char *artist = ol_music_info_get_artist (info);
  if (title == NULL && artist == NULL)
    return;
  if (title == NULL)
    title = _(UNKNOWN_TITLE);
  if (artist == NULL)
    artist = _(UNKNOWN_ARTIST);
  const char *album = ol_music_info_get_album (info);
  char *body = NULL;
  if (album == NULL)
  {
    body = g_strdup_printf (INFO_FORMAT,
                            artist);
  }
  else
  {
    body = g_strdup_printf (INFO_FORMAT_ALBUM,
                            artist,
                            album);
  }
  notify_notification_update (notify, title, body, NULL);
  notify_notification_set_timeout (notify,
                                   DEFAULT_TIMEOUT);
  notify_notification_show (notify, NULL);
  g_free (body);
}
