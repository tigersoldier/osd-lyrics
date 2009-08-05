#include <gtk/gtk.h>
#include "ol_option.h"
#include "ol_glade.h"
#include "ol_config.h"
#include "ol_osd_render.h"

typedef struct _OptionWidgets OptionWidgets;

static struct _OptionWidgets
{
  GtkWidget *ok;
  GtkWidget *cancel;
  GtkWidget *font;
  GtkWidget *width;
  GtkWidget *lrc_align[2];
  GtkWidget *active_lrc_color[OL_LINEAR_COLOR_COUNT];
  GtkWidget *inactive_lrc_color[OL_LINEAR_COLOR_COUNT];
  GtkWidget *osd_preview;
} options;

void ol_option_ok_clicked (GtkWidget *widget);
void ol_option_cancel_clicked (GtkWidget *widget);
void ol_option_update_preview (GtkWidget *widget);
void ol_option_preview_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data);
static OlColor ol_color_from_gdk_color (const GdkColor color);
static GdkColor ol_color_to_gdk_color (const OlColor color);
static void ol_option_update_widget (OptionWidgets *widgets);
/** 
 * @brief Get font family and font size from a GtkFontButton
 * 
 * @param font A GtkFontButton
 * @param font_family Ppointer to a string, should point to NULL and
 *                    freed by g_free
 * @param font_size Size of the font
 */
static void ol_option_get_font_info (GtkFontButton *font,
                                     gchar **font_family,
                                     double *font_size);

static void
ol_option_get_font_info (GtkFontButton *font,
                         gchar **font_family,
                         double *font_size)
{
  const gchar *font_name = gtk_font_button_get_font_name (font);
  PangoFontDescription *font_desc = pango_font_description_from_string (font_name);
  fprintf (stderr, "font: %s-%s %d\n", font_name, pango_font_description_get_family (font_desc), pango_font_description_get_size (font_desc));
  if (font_size != NULL)
  {
    *font_size = pango_font_description_get_size (font_desc);
    if (!pango_font_description_get_size_is_absolute (font_desc))
    {
      *font_size /= PANGO_SCALE;
      fprintf (stderr, "font-size: %lf\n", *font_size);
    }
  }
  if (font_family)
    *font_family = g_strdup (pango_font_description_get_family (font_desc));
  pango_font_description_free (font_desc);
}

static OlColor
ol_color_from_gdk_color (const GdkColor c)
{
  OlColor color;
  color.r = c.red / 65535.0;
  color.g = c.green / 65535.0;
  color.b = c.blue / 65535.0;
  return color;
}

static GdkColor
ol_color_to_gdk_color (const OlColor color)
{
  GdkColor ret;
  ret.red = color.r * 65535;
  ret.green = color.g * 65535;
  ret.blue = color.b * 65535;
  return ret;
}

void
ol_option_preview_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  g_return_if_fail (options.font != NULL);
  static const char *preview_text = "OSD Lyrics";
  cairo_t *cr = gdk_cairo_create (widget->window);
  gchar *font_family = NULL;
  double font_size;
  ol_option_get_font_info (GTK_FONT_BUTTON (options.font), &font_family, &font_size);
  static OlOsdRenderContext *render = NULL;
  render = ol_osd_render_context_new ();
  ol_osd_render_set_font_family (render, font_family);
  ol_osd_render_set_font_size (render, font_size);
  int tw, th, w, h;
  gdk_drawable_get_size (widget->window, &w, &h);
  ol_osd_render_get_pixel_size (render, preview_text, &tw, &th);
  double x = (w - tw) / 2.0;
  double y = (h - th) / 2.0;
  int i;
  for (i = 0; i < OL_LINEAR_COLOR_COUNT; i++)
  {
    if (options.active_lrc_color[i] != NULL)
    {
      OlColor color;
      GdkColor c;
      gtk_color_button_get_color (GTK_COLOR_BUTTON (options.active_lrc_color[i]),
                                  &c);
      color = ol_color_from_gdk_color (c);
      ol_osd_render_set_linear_color (render, i, color);
    }
  }
  cairo_save (cr);
  cairo_rectangle (cr, 0, 0, w / 2, h);
  cairo_clip (cr);
  ol_osd_render_paint_text (render, cr, preview_text, x, y);
  cairo_restore (cr);
  for (i = 0; i < OL_LINEAR_COLOR_COUNT; i++)
  {
    if (options.inactive_lrc_color[i] != NULL)
    {
      OlColor color;
      GdkColor c;
      gtk_color_button_get_color (GTK_COLOR_BUTTON (options.inactive_lrc_color[i]),
                                  &c);
      color.r = c.red / 65535.0;
      color.g = c.green / 65535.0;
      color.b = c.blue / 65535.0;
      ol_osd_render_set_linear_color (render, i, color);
    }
  }
  cairo_save (cr);
  cairo_rectangle (cr, w / 2, 0, w / 2, h);
  cairo_clip (cr);
  ol_osd_render_paint_text (render, cr, preview_text, x, y);
  cairo_restore (cr);
  ol_osd_render_context_destroy (render);
  cairo_destroy (cr);
  g_free (font_family);
}

void
ol_option_update_preview (GtkWidget *widget)
{
  gtk_widget_queue_draw (options.osd_preview);
}

static void
ol_option_update_widget (OptionWidgets *widgets)
{
  int i;
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
  /* Lrc align */
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
  /* [In]Active lrc color */
  GtkWidget **color_widgets[] =
    {options.active_lrc_color, options.inactive_lrc_color};
  char *color_props[] =
    {"active-lrc-color", "inactive-lrc-color"};
  int k;
  for (k = 0; k < 2; k++)
  {
    char ** lrc_color_str = ol_config_get_str_list (config,
                                                           color_props[k],
                                                           NULL);
    for (i = 0; i < OL_LINEAR_COLOR_COUNT; i++)
    {
      if (color_widgets[k][i] != NULL)
      {
        GtkColorButton *color_button = GTK_COLOR_BUTTON (color_widgets[k][i]);
        GdkColor color = ol_color_to_gdk_color (ol_color_from_string (lrc_color_str[i]));
        gtk_color_button_set_color (color_button, &color);
      }
    }
    g_strfreev (lrc_color_str);
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
    gchar *font_family = NULL;
    double font_size;
    ol_option_get_font_info (font, &font_family, &font_size);
    ol_config_set_string (config, "font-family",
                          font_family);
    ol_config_set_double (config, "font-size",
                          font_size);
    g_free (font_family);
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
  /* [In]Active lrc color */
  GtkWidget **color_widgets[] =
    {options.active_lrc_color, options.inactive_lrc_color};
  char *color_props[] =
    {"active-lrc-color", "inactive-lrc-color"};
  int k;
  OlColor colors[OL_LINEAR_COLOR_COUNT];
  for (k = 0; k < 2; k++)
  {
    for (i = 0; i < OL_LINEAR_COLOR_COUNT; i++)
    {
      if (color_widgets[k][i] != NULL)
      {
        GtkColorButton *color_button = GTK_COLOR_BUTTON (color_widgets[k][i]);
        GdkColor color;
        gtk_color_button_get_color (color_button, &color);
        colors[i] = ol_color_from_gdk_color (color);
      }
    }
    char **lrc_color_str = ol_color_to_str_list (colors, OL_LINEAR_COLOR_COUNT);
    ol_config_set_str_list (config,
                            color_props[k],
                            (const char**)lrc_color_str,
                            OL_LINEAR_COLOR_COUNT);
    g_strfreev (lrc_color_str);
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
    options.active_lrc_color[0] = ol_glade_get_widget ("active-lrc-color-0");
    options.active_lrc_color[1] = ol_glade_get_widget ("active-lrc-color-1");
    options.active_lrc_color[2] = ol_glade_get_widget ("active-lrc-color-2");
    options.inactive_lrc_color[0] = ol_glade_get_widget ("inactive-lrc-color-0");
    options.inactive_lrc_color[1] = ol_glade_get_widget ("inactive-lrc-color-1");
    options.inactive_lrc_color[2] = ol_glade_get_widget ("inactive-lrc-color-2");
    options.osd_preview = ol_glade_get_widget ("osd-preview");
  }
  g_return_if_fail (window != NULL);
  ol_option_update_widget (&options);
  gtk_dialog_run (GTK_DIALOG (window));
}
