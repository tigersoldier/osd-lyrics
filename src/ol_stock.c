#include <gtk/gtk.h>
#include "ol_stock.h"

const char *OL_STOCK_TRAYICON = "osd-lyrics-trayicon";

static GtkIconFactory *icon_factory = NULL;

void 
ol_stock_init ()
{
  if (icon_factory == NULL)
  {
    icon_factory = gtk_icon_factory_new ();
    GtkIconSet *tray_icon_set = gtk_icon_set_new ();
    GtkIconSource *tray_icon_source = gtk_icon_source_new ();
    gtk_icon_source_set_icon_name (tray_icon_source, OL_STOCK_TRAYICON);
    gtk_icon_set_add_source (tray_icon_set, tray_icon_source);
    gtk_icon_source_free (tray_icon_source);
    gtk_icon_factory_add (icon_factory, OL_STOCK_TRAYICON, tray_icon_set);
    gtk_icon_factory_add_default (icon_factory);
  }
}

