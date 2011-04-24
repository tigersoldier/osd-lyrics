#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include "ol_singleton.h"
#include "config.h"
#include "ol_debug.h"

const char *LOCK_FILENAME = "singleton.lock";

/* Try to lock a file, From APUE */
int64_t
_lockfile (int fd)
{
  struct flock fl;
  fl.l_type = F_WRLCK;
  fl.l_start = 0;
  fl.l_whence = SEEK_SET;
  fl.l_len = 0;
  return (fcntl (fd, F_SETLK, &fl));
}

int
ol_is_running ()
{
  ol_log_func ();
  int ret = 1;
  char *dir = g_strdup_printf ("%s/%s/", g_get_user_config_dir (), PACKAGE_NAME);
  if (g_mkdir_with_parents (dir, 0755) == -1)
  {
    ol_error ("Failed to endure config dir");
  }
  else
  {
    char *path = g_strdup_printf ("%s/%s/%s", g_get_user_config_dir (), PACKAGE_NAME, LOCK_FILENAME);
    int fd = open (path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
      ol_error ("Failed to open or create singleton lock");
    }
    else
    {
      if (_lockfile (fd) < 0)
      {
        if (errno == EACCES || errno == EAGAIN)
        {
          close (fd);
        }
        ol_infof ("Can not lock file %s: %s\n", path, strerror (errno));
      }
      else
      {
        if (ftruncate (fd, 0) != 0)
        {
          ol_errorf ("Failed to truncate singleton lock: %s\n",
                     strerror (errno));
        }
        char buf[16];
        sprintf (buf, "%ld", (long)getpid ());
        if (write (fd, buf, strlen (buf)) != strlen (buf))
          ol_errorf ("Failed to write pid in singleton lock\n");
        ret = 0;
      }
    }
    g_free (path);
  }
  g_free (dir);
  return ret;
}
