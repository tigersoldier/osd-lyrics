#include "ol_gui.h"
#include "ol_intl.h"
#include "ol_debug.h"

const char *BUILDER_FILE = GUIDIR "/dialogs.glade";
static GtkBuilder *builder = NULL;

static void internal_init ();

static void
internal_init ()
{
  if (builder == NULL)
  {
    builder = gtk_builder_new ();
    ol_assert (builder != NULL);
    gtk_builder_set_translation_domain (builder, PACKAGE);
    gtk_builder_add_from_file (builder, BUILDER_FILE, NULL);
    gtk_builder_connect_signals (builder, NULL);
  }
}

GtkWidget* 
ol_gui_get_widget (const char *name)
{
  ol_assert_ret (name != NULL, NULL);
  internal_init ();
  ol_assert_ret (builder != NULL, NULL);
  GObject *obj = gtk_builder_get_object (builder, name);
  if (obj != NULL && GTK_IS_WIDGET (obj))
    return GTK_WIDGET (obj);
  else
    return NULL;
}
