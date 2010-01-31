#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include "ol_fork.h"
#include "ol_debug.h"

const size_t DEFAULT_BUF_SIZE = 1024;
int ret_fd = 0;
FILE *fret = NULL;

struct ForkSource
{
  GSource source;
  GPollFD poll_fd;
  OlForkCallback callback;
  char *ret_data;
  size_t ret_size;
  size_t buf_len;
  void *data;
};

static void ol_fork_watch_callback (GPid pid,
                                    gint status,
                                    gpointer data);


static struct ForkSource *ol_fork_watch_fd (int fd,
                                            OlForkCallback callback,
                                            void *data);
static gboolean ol_fork_prepare (GSource *source,
                             gint *timeout);
static gboolean ol_fork_check (GSource *source);
static gboolean ol_fork_dispatch (GSource *source,
                                  GSourceFunc callback,
                                  gpointer user_data);
static void ol_fork_finalize (GSource *source);

static void
ol_fork_watch_callback (GPid pid,
                        gint status,
                        gpointer data)
{
  ol_assert (data != NULL);
  struct ForkSource *source = (struct ForkSource*) data;
  if (source->callback != NULL)
    source->callback (source->ret_data,
                      source->ret_size,
                      status,
                      source->data);
  close (source->poll_fd.fd);
  g_source_unref (source);
}

static gboolean
ol_fork_prepare (GSource *source,
                 gint *timeout)
{
  ol_assert_ret (timeout != NULL, FALSE);
  *timeout = -1;
  return FALSE;
}

static gboolean
ol_fork_check (GSource *source)
{
  ol_assert_ret (source != NULL, FALSE);
  struct ForkSource *fsource = (struct ForkSource *) source;
  gint revents = fsource->poll_fd.revents;
  if (revents & (G_IO_NVAL))
  {
    g_source_remove_poll (source, &(fsource->poll_fd));
    g_source_remove (g_source_get_id (source));
    g_source_unref (source);
    return FALSE;
  }
  if ((revents & G_IO_IN))
  {
    return TRUE;
  }
  return FALSE;
}

static gboolean ol_fork_dispatch (GSource *source,
                                  GSourceFunc callback,
                                  gpointer user_data)
{
  ol_assert_ret (source != NULL, FALSE);
  struct ForkSource *fsource = NULL;
  fsource = (struct ForkSource*) source;
  gint revents = fsource->poll_fd.revents;
  if (revents & G_IO_IN)
  {
    fsource->ret_size += read (fsource->poll_fd.fd,
                               fsource->ret_data + fsource->ret_size,
                               fsource->buf_len - fsource->ret_size);
    if (fsource->ret_size < fsource->buf_len)
    {
      fsource->ret_data[fsource->ret_size] = 0;
    }
    else
    {
      /* FIXME: Extend the buffer if overflowed */
    }
    return TRUE;
  }
  return FALSE;
}

static void
ol_fork_finalize (GSource *source)
{
  ol_assert (source != NULL);
  struct ForkSource *fsource = (struct ForkSource*) source;
  if (fsource->ret_data != NULL)
    g_free (fsource->ret_data);
}

static struct ForkSource *
ol_fork_watch_fd (int fd, OlForkCallback callback, void *data)
{
  ol_assert (fd > 0);
  ol_assert (callback != NULL);
  static GSourceFuncs fork_funcs = {
    ol_fork_prepare,
    ol_fork_check,
    ol_fork_dispatch,
    ol_fork_finalize,
  };
  struct ForkSource *source = NULL;
  source = (struct ForkSource*) g_source_new (&fork_funcs,
                                              sizeof (struct ForkSource));
  source->callback = callback;
  source->data = data;
  source->poll_fd.fd = fd;
  source->poll_fd.events = G_IO_IN | G_IO_HUP |
    G_IO_ERR | G_IO_NVAL;
  source->ret_size = 0;
  source->buf_len = DEFAULT_BUF_SIZE;
  source->ret_data = (void *) g_new0 (char, source->buf_len);
  g_source_add_poll ((GSource *) source, &source->poll_fd);
  g_source_attach ((GSource *)source, NULL);
  return source;
}

pid_t
ol_fork (OlForkCallback callback, void *data)
{
  int pipefd[2] = {0, 0};
  if (pipe (pipefd) != 0)
  {
    ol_errorf ("pipe () failed: %s\n", strerror (errno));
    return -1;
  }
  pid_t pid = 0;
  if ((pid = fork ()) == 0)     /* Child process */
  {
    close (pipefd[0]);
    ret_fd = pipefd[1];
    fret = fdopen (ret_fd, "w");
    return 0;
  }
  else                          /* Parent */
  {
    close (pipefd[1]);
    struct ForkSource *source = NULL;
    source = ol_fork_watch_fd (pipefd[0], callback, data);
    g_source_ref (source);
    g_child_watch_add (pid, ol_fork_watch_callback, source);
    return pid;
  }
}
