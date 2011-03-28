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

static gboolean inited = FALSE;
static NotifyNotification *notify = NULL;

static gboolean _init ();
static NotifyNotification *_get_notify (const char *summary,
                                        const char *body,
                                        const char *icon);

static NotifyNotification *
_get_notify (const char *summary,
             const char *body,
             const char *icon)
{
  ol_debugf ("summary: %s\n"
             "body: %s\n"
             "icon: %s\n",
             summary,
             body,
             icon);
  if (notify == NULL)
  {
#ifdef HAVE_LIBNOTIFY_0_7
    notify = notify_notification_new (summary,
                                      body,
                                      icon);
#else
    notify = notify_notification_new (summary,
                                      body,
                                      icon,
                                      NULL);
#endif
  }
  else
  {
    notify_notification_update (notify,
                                summary,
                                body,
                                icon);
  }
  return notify;
}

static gboolean
_init ()
{
  if (!notify_is_initted())
    notify_init (PACKAGE_NAME);
  return TRUE;
}

void
ol_notify_init ()
{
  _init ();
}

void
ol_notify_unload ()
{
  if (notify != NULL)
    g_object_unref (notify);
  notify_uninit ();
}

void
ol_notify_music_change (OlMusicInfo *info, const char *icon)
{
  ol_assert (info != NULL);
  if (!_init ())
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
  NotifyNotification *music_notify = _get_notify (title, body, icon);
  notify_notification_set_timeout (music_notify,
                                   DEFAULT_TIMEOUT);
  notify_notification_show (music_notify, NULL);
  g_free (body);
}
