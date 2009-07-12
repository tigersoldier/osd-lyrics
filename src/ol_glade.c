#include "ol_glade.h"
#include "config.h"

static GladeXML* ol_glade_get_xml ();

static GladeXML*
ol_glade_get_xml ()
{
  static GladeXML* xml = NULL;
  if (xml == NULL)
  {
    xml = glade_xml_new (GLADEDIR "/dialogs.glade", NULL, PACKAGE);
    if (xml != NULL)
    {
      glade_xml_signal_autoconnect (xml);
    }
  }
  return xml;
}

GtkWidget*
ol_glade_get_widget (const char *name)
{
  GladeXML *xml = ol_glade_get_xml ();
  g_return_val_if_fail (xml != NULL, NULL);
  return glade_xml_get_widget (xml, name);
}
