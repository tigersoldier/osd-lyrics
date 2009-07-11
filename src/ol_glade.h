#ifndef _OL_GLADE_H_
#define _OL_GLADE_H_

#include <gtk/gtk.h>
#include <glade/glade.h>

/** 
 * @brief Gets a widget in Glade file by name
 * 
 * @param name name of the widget
 * 
 * @return 
 */
GtkWidget* ol_glade_get_widget (const char *name);

#endif /* _OL_GLADE_H_ */
