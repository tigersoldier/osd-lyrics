#include <gtk/gtk.h>
#include "ol_option.h"
#include "ol_glade.h"
#include "ol_config.h"

typedef struct _OptionWidgets OptionWidgets;

static struct _OptionWidgets
{
  GtkWidget *ok;
  GtkWidget *cancel;
  GtkWidget *font;
  GtkWidget *width;
  GtkWidget *lrc_align[2];
} options;

void ol_option_ok_clicked (GtkWidget *widget);
void ol_option_cancel_clicked (GtkWidget *widget);
static void ol_option_update_widget (OptionWidgets *widgets);

static void
ol_option_update_widget (OptionWidgets *widgets)
{
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  /* Updates font */
  GtkFontButton *font = GTK_FONT_BUTTON (widgets->font);
  if (font != NULL)
  {
    gchar *font_family = ol_config_get_string (config, "font-family");
    gchar *font_name = g_strdup_printf ("%s %0.0lf", font_family, ol_config_get_double (config, "font-size"));
    gtk_font_button_set_font_name (font, font_name);
    g_free (font_name);
    g_free (font_family);
  }
  /* Updates Width */
  GtkSpinButton *width_widget = GTK_SPIN_BUTTON (widgets->width);
  if (width_widget != NULL)
  {
    gtk_spin_button_set_value (width_widget,
                               ol_config_get_int (config, "width"));
  }
  int i;
  for (i = 0; i < 2; i++)
  {
    GtkRange *lrc_align = GTK_RANGE (widgets->lrc_align[i]);
    if (lrc_align != NULL)
    {
      char buffer[20];
      sprintf (buffer, "lrc-align-%d", i);
      gtk_range_set_value (lrc_align,
                           ol_config_get_double (config, buffer));
    }
  }
}

void
ol_option_ok_clicked (GtkWidget *widget)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  /* Updates Font */
  OlConfig *config = ol_config_get_instance ();
  GtkFontButton *font = GTK_FONT_BUTTON (options.font);
  if (font != NULL)
  {
    const gchar *font_name = gtk_font_button_get_font_name (font);
    PangoFontDescription *font_desc = pango_font_description_from_string (font_name);
    fprintf (stderr, "font: %s-%s %d\n", font_name, pango_font_description_get_family (font_desc), pango_font_description_get_size (font_desc));
    double font_size = pango_font_description_get_size (font_desc);
    if (!pango_font_description_get_size_is_absolute (font_desc))
    {
      font_size /= PANGO_SCALE;
      fprintf (stderr, "font-size: %lf\n", font_size);
    }
    ol_config_set_string (config, "font-family",
                         pango_font_description_get_family (font_desc));
    ol_config_set_double (config, "font-size",
                          font_size);
    pango_font_description_free (font_desc);
  }
  /* Updates Width */
  GtkSpinButton *width_widget = GTK_SPIN_BUTTON (options.width);
  if (width_widget != NULL)
  {
    ol_config_set_int (config, "width", gtk_spin_button_get_value (width_widget));
                               
  }
  int i;
  for (i = 0; i < 2; i++)
  {
    GtkRange *lrc_align = GTK_RANGE (options.lrc_align[i]);
    if (lrc_align != NULL)
    {
      char buffer[20];
      sprintf (buffer, "lrc-align-%d", i);
      ol_config_set_double (config, buffer, 
                            gtk_range_get_value (lrc_align));

    }
  }
  /* Close dialog */
  GtkWidget *toplevel = gtk_widget_get_toplevel (widget);
  if (GTK_WIDGET_TOPLEVEL (toplevel))
  {
    gtk_widget_hide (toplevel);
  }
}

void
ol_option_cancel_clicked (GtkWidget *widget)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  GtkWidget *toplevel = gtk_widget_get_toplevel (widget);
  if (GTK_WIDGET_TOPLEVEL (toplevel))
  {
    gtk_widget_hide (toplevel);
  }
}

void
ol_option_show ()
{
  static GtkWidget *window = NULL;
  if (window == NULL)
  {
    window = ol_glade_get_widget ("optiondialog");
    g_signal_connect (window, "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete),
                      NULL);
    options.ok = ol_glade_get_widget ("optionok");
    options.cancel = ol_glade_get_widget ("optioncancel");
    options.font = ol_glade_get_widget ("osd-font");
    options.width = ol_glade_get_widget ("osd-width");
    options.lrc_align[0] = ol_glade_get_widget ("lrc-align-0");
    options.lrc_align[1] = ol_glade_get_widget ("lrc-align-1");
  }
  g_return_if_fail (window != NULL);
  ol_option_update_widget (&options);
  gtk_dialog_run (GTK_DIALOG (window));
}
