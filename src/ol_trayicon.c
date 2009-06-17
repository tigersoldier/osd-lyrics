#include "ol_trayicon.h"
#include "ol_menu.h"

const char TRAYICON_FILE[] = ICONDIR "/trayicon.png";

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

void ol_trayicon_inital (OlOsdWindow *osd)
{
  status_icon = gtk_status_icon_new_from_file (TRAYICON_FILE);
  gtk_status_icon_set_visible (status_icon, TRUE);
  gtk_status_icon_set_tooltip (status_icon, "This is a test");

  /* Connect signals */
  g_signal_connect (G_OBJECT (status_icon), "popup-menu",
                    G_CALLBACK (popup), osd);

  g_signal_connect (G_OBJECT (status_icon), "activate",
                    G_CALLBACK (activate), NULL);
}
