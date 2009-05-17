#include"ol_osd_trayicon.h"

static GtkWidget *my_menu = NULL;
static GtkStatusIcon *status_icon = NULL;
static gboolean locked = 1;

static void 
destroy (GtkWidget *widget,
		gpointer data)
{
	gtk_main_quit ();
}

static void
osd_window_lock_change (GtkStatusIcon *widget,gpointer data)
{
  locked = 1 - locked;
  ol_osd_window_set_locked ((OlOsdWindow *)data,locked);
  gtk_status_icon_set_blinking(GTK_STATUS_ICON(status_icon), 1-locked);
}


static void
activate (GtkStatusIcon* status_icon,
		gpointer user_data)
{
	g_debug ("'activate' signal triggered");
}

static void 
popup (GtkStatusIcon *status_icon,
		guint button,
		guint activate_time,
		gpointer data)
{
	if (!my_menu)
	{
		GtkWidget *item;
		my_menu = gtk_menu_new();
        item  =  gtk_check_menu_item_new_with_label ("locked");
		gtk_menu_append (my_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
				G_CALLBACK(osd_window_lock_change),
                          (OlOsdWindow *)data);

		item = gtk_menu_item_new_with_label ("Setting");
		gtk_menu_append (my_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
				G_CALLBACK(destroy), 
				NULL);
        
		item = gtk_menu_item_new_with_label ("About");
		gtk_menu_append (my_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
				G_CALLBACK(destroy), 
				NULL);
        
		item = gtk_menu_item_new_with_label ("Quit");
		gtk_menu_append (my_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
				G_CALLBACK(destroy), 
				NULL);
	}
	
	gtk_widget_show_all (my_menu);

	gtk_menu_popup (GTK_MENU(my_menu),
			NULL,
			NULL,
			gtk_status_icon_position_menu,
			status_icon,
			button,
			activate_time);
}

void ol_osd_trayicon_inital (OlOsdWindow *osd)
{

	status_icon = gtk_status_icon_new_from_file ("4.png");
	gtk_status_icon_set_visible (status_icon, TRUE);
	gtk_status_icon_set_tooltip (status_icon, "This is a test");

	/* Connect signals */
	g_signal_connect (G_OBJECT (status_icon), "popup-menu",
			  G_CALLBACK (popup), osd);

	g_signal_connect (G_OBJECT (status_icon), "activate",
			  G_CALLBACK (activate), NULL);
}
