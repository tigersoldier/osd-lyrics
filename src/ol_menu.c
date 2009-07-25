#include "ol_menu.h"
#include "ol_intl.h"
#include "ol_config.h"
#include "ol_about.h"
#include "ol_option.h"

static GtkWidget *popup_menu = NULL;

static void
ol_menu_quit (GtkWidget *widget,
              gpointer data)
{
  gtk_main_quit ();
}

static void
osd_window_lock_change (GtkStatusIcon *widget, gpointer data)
{
  /* ol_osd_window_set_locked ((OlOsdWindow *)data, locked); */
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  ol_config_set_bool (config, "locked", !ol_config_get_bool (config, "locked"));
}

static void
ol_menu_about (GtkWidget *widget, gpointer data)
{
  ol_about_show ();
}

static void
ol_menu_option (GtkWidget *widget, gpointer data)
{
  ol_option_show ();
}

GtkWidget*
ol_menu_get_popup ()
{
  if (popup_menu == NULL)
  {
    GtkWidget *item;
    popup_menu = gtk_menu_new();
    item  =  gtk_check_menu_item_new_with_mnemonic (_("_Lock"));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                    ol_config_get_bool (ol_config_get_instance (),
                                                        "locked"));
    gtk_menu_append (popup_menu, item);
    g_signal_connect (G_OBJECT(item), "activate",
                      G_CALLBACK(osd_window_lock_change),
                      NULL);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, NULL);
    gtk_menu_append (popup_menu, item);
    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (ol_menu_option),
                      NULL);
        
    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
    gtk_menu_append (popup_menu, item);
    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (ol_menu_about),
                      NULL);
        
    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
    gtk_menu_append (popup_menu, item);
    g_signal_connect (G_OBJECT(item), "activate",
                      G_CALLBACK(ol_menu_quit),
                      NULL);
    gtk_widget_show_all (popup_menu);
  }
  return popup_menu;
}
