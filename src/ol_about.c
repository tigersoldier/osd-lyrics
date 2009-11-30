#include <gtk/gtk.h>
#include <glade/glade.h>
#include "config.h"
#include "ol_intl.h"
#include "ol_about.h"
#include "ol_glade.h"

void ol_about_close_clicked (GtkWidget *widget);
void ol_about_response (GtkDialog *dialog, gint response_id, gpointer user_data);

void
ol_about_response (GtkDialog *dialog, gint response_id, gpointer user_data)
{
  fprintf (stderr, "%s:%d\n", __FUNCTION__, response_id);
  switch (response_id)
  {
  case GTK_RESPONSE_CANCEL:     /* Close button in about dialog */
    gtk_widget_hide (GTK_WIDGET (dialog));
    break;
  }
}

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
    GdkPixbuf *logo = gdk_pixbuf_new_from_file (ICONDIR "/osd-lyrics.png", NULL);
    if (logo)
    {
      gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (window),
                                 logo);
    }
  }
  g_return_if_fail (window != NULL);
  gtk_dialog_run (GTK_DIALOG (window));
}
