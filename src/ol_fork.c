#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include "ol_fork.h"
#include "ol_debug.h"

const size_t DEFAULT_BUF_SIZE = 1024;
int ret_fd = 0;
FILE *fret = NULL;

struct ForkData
{
  int fd;
  OlForkCallback callback;
  char *ret_data;
  size_t ret_size;
  size_t buf_len;
  void *data;
};

static void ol_fork_watch_callback (GPid pid,
                                    gint status,
                                    gpointer data);

static void
ol_fork_watch_callback (GPid pid,
                        gint status,
                        gpointer data)
{
  ol_assert (data != NULL);
  struct ForkData *source = (struct ForkData*) data;
  do
  {
    source->ret_size += read (source->fd,
                              source->ret_data + source->ret_size,
                              source->buf_len - source->ret_size);
    if (source->ret_size < source->buf_len)
    {
      source->ret_data[source->ret_size] = 0;
      break;
    }
    else
    {
      source->buf_len *= 2;
      char *newdata = g_new (char, source->buf_len);
      memcpy (newdata, source->ret_data, source->ret_size);
      g_free (source->ret_data);
      source->ret_data = newdata;
    }
  } while (1);
  
  if (source->callback != NULL)
    source->callback (source->ret_data,
                      source->ret_size,
                      status,
                      source->data);
  g_free (source->ret_data);
  close (source->fd);
  g_free (source);
}

pid_t
ol_fork (OlForkCallback callback, void *userdata)
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
    struct ForkData *data = g_new (struct ForkData, 1);
    data->fd = pipefd[0];
    data->callback = callback;
    data->data = userdata;
    data->ret_size = 0;
    data->buf_len = DEFAULT_BUF_SIZE;
    data->ret_data = g_new0 (char, data->buf_len);
    g_child_watch_add (pid, ol_fork_watch_callback, data);
    return pid;
  }
}
