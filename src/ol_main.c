#include <stdio.h>
#include "ol_osd_window.h"

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  OlOsdWindow *osd = OL_OSD_WINDOW (ol_osd_window_new ());
  ol_osd_window_resize (osd, 1024, 200);
  ol_osd_window_set_lyric (osd, 0, "Hellow");
  ol_osd_window_set_lyric (osd, 1, "World");
  ol_osd_window_set_alignment (osd, 0.5, 1);
  gtk_widget_show (GTK_WIDGET (osd));
  gtk_main ();
  return 0;
}
