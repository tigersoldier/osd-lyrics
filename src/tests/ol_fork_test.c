#include <stdio.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <ol_fork.h>

#define BUFFER_SIZE 1024

void
callback (int fd, void *data)
{
  char buffer[BUFFER_SIZE];
  printf ("callback called\n");
  ssize_t len = read (fd, buffer, BUFFER_SIZE);
  buffer[len] = '\0';
  printf ("received: %s\n", buffer);
  printf ("expected: %s\n", (char *) data);
}

void
test1 ()
{
  static const char msg[] = "Fork Message";
  printf ("Forking\n");
  if (ol_fork (callback, msg) == 0)
  {
    sleep (2);
    printf (msg);
    exit (0);
  }
  else
  {
    printf ("Fork succeeded\n");
  }
}

int
main (int argc, char **argv)
{
  GMainLoop *loop = g_main_loop_new (NULL, FALSE);
  test1 ();
  g_main_loop_run (loop);
}
