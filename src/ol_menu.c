#include "ol_menu.h"
#include "ol_intl.h"
#include "ol_config.h"
#include "ol_about.h"
#include "ol_option.h"

static GtkWidget *popup_menu = NULL;

static gboolean ol_menu_hide_accel (gpointer userdata);

static gboolean
ol_menu_hide_accel (gpointer userdata)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  return FALSE;
}
  
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
    /* create accelerator group */
    static GtkAccelGroup *accel = NULL;
    GClosure *hide_closure = g_cclosure_new (ol_menu_hide_accel,
                                             NULL,
                                             NULL);
    accel = gtk_accel_group_new ();
    gtk_accel_map_add_entry ("<OSD Lyrics>/Hide",
                             gdk_keyval_from_name ("h"),
                             GDK_CONTROL_MASK | GDK_MOD1_MASK);
    gtk_accel_group_connect_by_path (accel,
                                     "<OSD Lyrics>/Hide",
                                     hide_closure);
    /* gtk_accel_group_connect (accel, gdk_keyval_from_name ("h"), */
    /*                          GDK_CONTROL_MASK | GDK_MOD1_MASK, */
    /*                          GTK_ACCEL_VISIBLE, */
    /*                          hide_closure); */
    
    GtkWidget *item;
    popup_menu = gtk_menu_new();
    gtk_menu_set_accel_group (GTK_MENU (popup_menu), accel);
    item = gtk_check_menu_item_new_with_mnemonic (_("_Lock"));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                    ol_config_get_bool (ol_config_get_instance (),
                                                        "locked"));
    gtk_menu_append (popup_menu, item);
    g_signal_connect (G_OBJECT(item), "activate",
                      G_CALLBACK(osd_window_lock_change),
                      NULL);

    item = gtk_check_menu_item_new_with_mnemonic (_("_Hide"));
    gtk_menu_append (popup_menu, item);
    gtk_menu_item_set_accel_path (GTK_MENU_ITEM (item),
                                  "<OSD Lyrics>/Hide");
    gtk_accel_groups_activate (G_OBJECT (popup_menu),
                               gdk_keyval_from_name ("h"),
                               GDK_CONTROL_MASK | GDK_MOD1_MASK);
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
