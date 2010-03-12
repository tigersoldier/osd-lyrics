#include <gtk/gtk.h>

#include "ol_osd_window.h"

gint
timeout_callback (gpointer data)
{
  static int line = 0;
  static double percentage = 0.0;
  percentage += 0.02;
  if (percentage > 1.1)
  {
    line = 1 - line;
    percentage = 0;
  }
  /* printf ("timeout: line %d, percentage: %0.2lf\n", line, percentage); */
  ol_osd_window_set_current_line (OL_OSD_WINDOW (data), line);
  ol_osd_window_set_current_percentage (OL_OSD_WINDOW (data), percentage);
/*   ol_osd_paint (OL_OSD_WINDOW (data), "asdf", .5); */
  return TRUE;
}

gboolean button_motion (GtkWidget      *widget,
                        GdkEventMotion *event,
                        gpointer        user_data)
{
  printf ("button motion\n");
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  GtkWidget *ol_osd = ol_osd_window_new ();
/*   GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL); */
/*   GtkWidget *hbox = gtk_hbox_new (TRUE, 10); */
/*   gtk_box_pack_start (GTK_BOX (hbox), ol_osd, FALSE, FALSE,0); */
/*   gtk_container_add (GTK_CONTAINER (window), hbox); */
/*   GdkPixmap *pixmap = gdk_pixmap_new (NULL, 1280, 800, 1); */
/*   gtk_widget_input_shape_combine_mask (ol_osd, pixmap, 0, 0); */
  ol_osd_window_resize (OL_OSD_WINDOW (ol_osd), 800, 140);
  gtk_widget_show (ol_osd);
  ol_osd_window_set_line_count (ol_osd, 2);
  ol_osd_window_set_translucent_on_mouse_over (ol_osd, TRUE);
  ol_osd_window_set_lyric (OL_OSD_WINDOW (ol_osd), 0, "还没好好地感受");
  ol_osd_window_set_lyric (OL_OSD_WINDOW (ol_osd), 1, "雪花绽放的气候");
/*   ol_osd_paint (OL_OSD_WINDOW (ol_osd), "还没好好地感受", "雪花绽放的气候", 0.3); */
  ol_osd_window_set_alignment (OL_OSD_WINDOW (ol_osd), 0.5, 1.0);
  ol_osd_window_set_line_alignment (OL_OSD_WINDOW (ol_osd), 0, 0.5);
  ol_osd_window_set_line_alignment (OL_OSD_WINDOW (ol_osd), 1, 1.0);
  ol_osd_window_set_current_percentage (OL_OSD_WINDOW (ol_osd), 0.3);
  ol_osd_window_set_current_percentage (OL_OSD_WINDOW (ol_osd), 0.8);
  ol_osd_window_set_locked (OL_OSD_WINDOW (ol_osd), FALSE);
/*   gtk_widget_show_all (window); */
  GtkWidget *window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_widget_show (window);
  g_timeout_add (100, timeout_callback, ol_osd);

  GtkWidget *button = gtk_button_new_with_label ("Tool button");
  printf ("before add child\n");
  gtk_container_add (GTK_CONTAINER (ol_osd), GTK_WIDGET (button));
  printf ("child added\n");
  g_signal_connect (button, "motion-notify-event",
                    G_CALLBACK (button_motion),
                    NULL);
  gtk_widget_show (button);
  printf ("child shown\n");

  printf ("shown\n");
  gtk_main ();
  return 0;
}
