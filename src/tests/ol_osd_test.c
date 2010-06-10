#include <gtk/gtk.h>

#include "ol_osd_window.h"
#include "ol_image_button.h"

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

GtkWidget *
init_osd ()
{
  GtkWidget *widget = ol_osd_window_new ();
  OlOsdWindow *ol_osd = OL_OSD_WINDOW (widget);
/*   GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL); */
/*   GtkWidget *hbox = gtk_hbox_new (TRUE, 10); */
/*   gtk_box_pack_start (GTK_BOX (hbox), ol_osd, FALSE, FALSE,0); */
/*   gtk_container_add (GTK_CONTAINER (window), hbox); */
/*   GdkPixmap *pixmap = gdk_pixmap_new (NULL, 1280, 800, 1); */
/*   gtk_widget_input_shape_combine_mask (ol_osd, pixmap, 0, 0); */
  ol_osd_window_resize (OL_OSD_WINDOW (ol_osd), 800, 140);
  gtk_widget_show (widget);
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
  GdkPixbuf *bg = gdk_pixbuf_new_from_file ("/usr/local/share/icons/hicolor/scalable/apps/osd-lyrics-osd-bg.svg", NULL);
  ol_osd_window_set_bg (ol_osd, bg);
  g_timeout_add (100, timeout_callback, ol_osd);
  return widget;
}

GtkWidget *
init_toolbar ()
{
  GtkBox *toolbar = GTK_BOX (gtk_hbox_new (FALSE, 0));
  GtkWidget *btn = ol_image_button_new ();
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file ("/usr/local/share/icons/hicolor/scalable/actions/osd-lyrics-osd-play.svg", NULL);
  ol_image_button_set_pixbuf (OL_IMAGE_BUTTON (btn), pixbuf);
  gtk_box_pack_end (toolbar, btn, FALSE, TRUE, 0);
  gtk_widget_show_all (GTK_WIDGET (toolbar));
  return GTK_WIDGET (toolbar);
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
/*   gtk_widget_show_all (window); */
  GtkWidget *ol_osd = init_osd ();
  gtk_container_add (GTK_CONTAINER (ol_osd), init_toolbar ());
  g_timeout_add (5000, gtk_main_quit, NULL);
  gtk_main ();
  g_object_unref (ol_osd);
  return 0;
}
