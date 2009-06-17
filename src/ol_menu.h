#ifndef _OL_MENU_H_
#define _OL_MENU_H_
#include <gtk/gtk.h>

/** 
 * @brief Gets singleton popup menu
 * The popup menu will be created at the first invoke
 * 
 * @return The popup menu for the app
 */
GtkWidget* ol_menu_get_popup ();

#endif /* _OL_MENU_H_ */
