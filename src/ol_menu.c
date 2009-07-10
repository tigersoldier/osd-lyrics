#include "ol_menu.h"
#include "ol_intl.h"
#include "ol_config.h"

static GtkWidget *popup_menu = NULL;
static gboolean locked = 1;

/* static void  */
/* destroy (GtkWidget *widget, */
/*          gpointer data) */
/* { */
/*   gtk_main_quit (); */
/* } */

static void
osd_window_lock_change (GtkStatusIcon *widget, gpointer data)
{
  locked = 1 - locked;
  /* ol_osd_window_set_locked ((OlOsdWindow *)data, locked); */
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  ol_config_set_bool (config, "locked", locked);
}

GtkWidget*
ol_menu_get_popup ()
{
  if (popup_menu == NULL)
  {
    GtkWidget *item;
    popup_menu = gtk_menu_new();
    item  =  gtk_check_menu_item_new_with_mnemonic (_("_Lock"));
    gtk_menu_append (popup_menu, item);
    g_signal_connect (G_OBJECT(item), "activate",
                      G_CALLBACK(osd_window_lock_change),
                      NULL);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, NULL);
    gtk_menu_append (popup_menu, item);
    /* g_signal_connect (G_OBJECT(item), "activate", */
    /*                   G_CALLBACK(destroy),  */
    /*                   NULL); */
        
    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
    gtk_menu_append (popup_menu, item);
    /* g_signal_connect (G_OBJECT(item), "activate", */
    /*                   G_CALLBACK(destroy),  */
    /*                   NULL); */
        
    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
    gtk_menu_append (popup_menu, item);
    /* g_signal_connect (G_OBJECT(item), "activate", */
    /*                   G_CALLBACK(destroy),  */
    /*                   NULL); */
    gtk_widget_show_all (popup_menu);
  }
  return popup_menu;
}
