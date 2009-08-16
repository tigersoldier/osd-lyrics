/**
 * @file   ol_menu.c
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Sun Aug 16 16:18:41 2009
 * 
 * @brief  Popup menu for OSD lyrics
 * 
 * 
 */

#include "ol_menu.h"
#include "ol_intl.h"
#include "ol_config.h"
#include "ol_about.h"
#include "ol_option.h"
#include "ol_keybindings.h"
#include "ol_commands.h"

static void ol_config_changed (OlConfig *config, gchar *name, gpointer data);
static GtkWidget *popup_menu = NULL;
enum {
  OL_MENU_LOCK,
  OL_MENU_HIDE,
  OL_MENU_PERFERENCE,
  OL_MENU_ABOUT,
  OL_MENU_QUIT,
  OL_MENU_COUNT,
};

static GtkMenuItem *items[OL_MENU_COUNT] = {0};

static void
ol_config_changed (OlConfig *config, gchar *name, gpointer data)
{
  if (strcmp (name, "locked") == 0)
  {
    gboolean locked = ol_config_get_bool (config, "locked");
    if (locked == gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (items[OL_MENU_LOCK])))
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (items[OL_MENU_LOCK]),
                                        locked);
  }
}

static void
ol_menu_lock (GtkWidget *widget, gpointer data)
{
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  ol_config_set_bool (config, "locked", !ol_config_get_bool (config, "locked"));
}

static void
ol_menu_quit (GtkWidget *widget,
              gpointer data)
{
  gtk_main_quit ();
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
    /* create accelerator group */
    GtkAccelGroup *accel = ol_keybinding_get_accel_group ();
    GtkWidget *item;
    OlConfig *config = ol_config_get_instance ();
    popup_menu = gtk_menu_new();
    gtk_menu_set_accel_group (GTK_MENU (popup_menu), accel);
    item = gtk_check_menu_item_new_with_mnemonic (_("_Lock"));
    gtk_menu_item_set_accel_path (GTK_MENU_ITEM (item),
                                  "<OSD Lyrics>/Lock");
    gtk_menu_append (popup_menu, item);
    g_signal_connect (G_OBJECT(item), "activate",
                      G_CALLBACK(ol_menu_lock),
                      NULL);
    items[OL_MENU_LOCK] = GTK_MENU_ITEM (item);
    ol_config_changed (config, "locked", NULL);

    item = gtk_check_menu_item_new_with_mnemonic (_("_Hide"));
    gtk_menu_append (popup_menu, item);
    gtk_menu_item_set_accel_path (GTK_MENU_ITEM (item),
                                  "<OSD Lyrics>/Hide");
    g_signal_connect (G_OBJECT(item), "activate",
                      G_CALLBACK(ol_osd_lock_unlock),
                      NULL);
    items[OL_MENU_HIDE] = GTK_MENU_ITEM (item);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, NULL);
    gtk_menu_append (popup_menu, item);
    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (ol_menu_option),
                      NULL);
    items[OL_MENU_PERFERENCE] = GTK_MENU_ITEM (item);
        
    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
    gtk_menu_append (popup_menu, item);
    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (ol_menu_about),
                      NULL);
    items[OL_MENU_ABOUT] = GTK_MENU_ITEM (item);
        
    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
    gtk_menu_append (popup_menu, item);
    g_signal_connect (G_OBJECT(item), "activate",
                      G_CALLBACK(ol_menu_quit),
                      NULL);
    items[OL_MENU_QUIT] = GTK_MENU_ITEM (item);
    g_signal_connect (config,
                      "changed",
                      G_CALLBACK (ol_config_changed),
                      NULL);
    gtk_widget_show_all (popup_menu);
  }
  return popup_menu;
}
