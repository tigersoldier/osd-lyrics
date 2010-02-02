#ifndef _OL_GUI_H_
#define _OL_GUI_H_

#include <gtk/gtk.h>

/** 
 * @brief Gets a widget in Glade file by name
 * 
 * @param name name of the widget
 * 
 * @return 
 */
GtkWidget *ol_gui_get_widget (const char *name);

#endif /* _OL_GUI_H_ */
