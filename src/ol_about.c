#include <gtk/gtk.h>
#include <glade/glade.h>
#include "config.h"
#include "ol_intl.h"
#include "ol_about.h"
#include "ol_glade.h"

void ol_about_close_clicked (GtkWidget *widget);

void
ol_about_close_clicked (GtkWidget *widget)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  GtkWidget *toplevel = gtk_widget_get_toplevel (widget);
  if (GTK_WIDGET_TOPLEVEL (toplevel))
  {
    gtk_widget_hide (toplevel);
  }
}

void
ol_about_show ()
{
  static GtkWidget *window = NULL;
  if (window == NULL)
  {
    window = ol_glade_get_widget ("aboutdialog");
    gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (window), PACKAGE_VERSION);
    g_signal_connect (window, "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
  }
  g_return_if_fail (window != NULL);
  gtk_dialog_run (GTK_DIALOG (window));
}
