#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <gtk/gtk.h>
#include <ol_fork.h>

#define BUFFER_SIZE 1024

void
callback (void *ret_data,
          size_t ret_size,
          int status,
          void *data)
{
  printf ("callback called\n");
  printf ("return status: %d\n", WEXITSTATUS (status));
  printf ("received: %s\n", (char*)ret_data);
  if (data != NULL)
    printf ("data: %s\n", (char *) data);
}

void
test_normal ()
{
  static const char msg[] = "Fork Message";
  printf ("Forking\n");
  if (ol_fork (callback, (void*)msg, NULL) == 0)
  {
    sleep (2);
    fprintf (fret, msg);
    exit (0);
  }
  else
  {
    printf ("Fork succeeded\n");
  }
}

void
test_multiple_output ()
{
  printf ("Forking\n");
  if (ol_fork (callback, NULL, NULL) == 0)
  {
    fprintf (fret, "Before Sleep\n");
    sleep (2);
    fprintf (fret, "After Sleep\n");
    exit (0);
  }
  else
  {
    printf ("Fork succeeded\n");
  }
}

void
test_no_output ()
{
  printf ("Forking\n");
  if (ol_fork (callback, NULL, NULL) == 0)
  {
    exit (0);
  }
  else
  {
    printf ("Fork succeeded\n");
  }
}

void
test_exit_status ()
{
  printf ("Forking\n");
  if (ol_fork (callback, NULL, NULL) == 0)
  {
    exit (10);
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
  test_normal ();
  test_multiple_output ();
  test_no_output ();
  test_exit_status ();
  g_main_loop_run (loop);
  return 0;
}
