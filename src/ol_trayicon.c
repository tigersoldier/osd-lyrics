#include "ol_trayicon.h"
#include "ol_intl.h"
#include "ol_menu.h"
#include "ol_stock.h"

static GtkStatusIcon *status_icon = NULL;

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
  GtkWidget *popup_menu = ol_menu_get_popup ();
  gtk_menu_popup (GTK_MENU(popup_menu),
                  NULL,
                  NULL,
                  gtk_status_icon_position_menu,
                  status_icon,
                  button,
                  activate_time);
}

void ol_trayicon_inital ()
{
  if (status_icon == NULL)
  {
    status_icon = gtk_status_icon_new_from_stock (OL_STOCK_TRAYICON);
    gtk_status_icon_set_visible (status_icon, TRUE);
    gtk_status_icon_set_tooltip (status_icon, _("OSD Lyrics"));

    /* Connect signals */
    g_signal_connect (G_OBJECT (status_icon), "popup-menu",
                      G_CALLBACK (popup), NULL);

    g_signal_connect (G_OBJECT (status_icon), "activate",
                      G_CALLBACK (activate), NULL);
  }
}

void ol_trayicon_free ()
{
  if (status_icon != NULL)
  {
    g_object_unref (status_icon);
    status_icon = NULL;
  }
}
