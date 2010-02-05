#include <gtk/gtk.h>
#include "ol_stock.h"


static GtkIconFactory *icon_factory = NULL;
const char *ICON_LIST[] = {
  OL_STOCK_TRAYICON,
  OL_STOCK_LOADING,
};

void 
ol_stock_init ()
{
  if (icon_factory == NULL)
  {
    icon_factory = gtk_icon_factory_new ();
    int i;
    for (i = 0; i < G_N_ELEMENTS (ICON_LIST); i++)
    {
      GtkIconSet *icon_set = gtk_icon_set_new ();
      GtkIconSource *icon_source = gtk_icon_source_new ();
      gtk_icon_source_set_icon_name (icon_source, 
                                     ICON_LIST[i]);
      gtk_icon_set_add_source (icon_set, icon_source);
      gtk_icon_source_free (icon_source);
      gtk_icon_factory_add (icon_factory, 
                            ICON_LIST[i], 
                            icon_set);
    }
    gtk_icon_factory_add_default (icon_factory);
  }
}

