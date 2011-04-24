#include <gtk/gtk.h>

#include "ol_osd_window.h"
#include "ol_test_util.h"

void
basic_test ()
{
  GtkWidget *window = ol_osd_window_new ();
  gtk_widget_show (window);
  gtk_widget_destroy (window);
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  basic_test ();
}
