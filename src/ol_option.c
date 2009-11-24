#include <string.h>
#include <gtk/gtk.h>
#include "ol_option.h"
#include "ol_glade.h"
#include "ol_config.h"
#include "ol_osd_render.h"      /* For getting preview for OSD font and color */
#include "ol_lrc_fetch.h"
#include "ol_path_manage.h"     /* For getting preview for LRC filename */
#include "ol_intl.h"
#include "ol_cell_renderer_button.h"

#define BUFFER_SIZE 1024

static gboolean firstrun = TRUE;
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
  GtkWidget *line_count[2];
  GtkWidget *download_engine;
  GtkWidget *osd_translucent;
  GtkWidget *lrc_path;
  GtkWidget *lrc_path_text;
  GtkWidget *lrc_filename;
  GtkWidget *lrc_filename_text;
  GtkWidget *lrc_filename_sample;
} options;

static struct ListExtraWidgets
{
  GtkWidget *entry;
  GtkWidget *add_button;
  GtkWidget *remove_button;
  GtkWidget *list;
} lrc_path_widgets, lrc_filename_widgets;

enum TreeColumns {
  TEXT_COLUMN = 0,
  REMOVE_COLUMN,
  N_COLUMN,
};

void ol_option_ok_clicked (GtkWidget *widget);
void ol_option_cancel_clicked (GtkWidget *widget);
void ol_option_update_preview (GtkWidget *widget);
void ol_option_preview_expose (GtkWidget *widget,
                               GdkEventExpose *event,
                               gpointer data);
void ol_option_lrc_filename_changed (GtkEditable *editable,
                                     gpointer user_data);
static void ol_option_list_add_clicked (GtkButton *button,
                                        struct ListExtraWidgets *widgets);
static void ol_option_list_remove_clicked (GtkButton *button,
                                           struct ListExtraWidgets *widgets);
static void ol_option_list_select_changed (GtkTreeSelection *selection,
                                           gpointer data);
static void ol_option_list_entry_changed (GtkEditable *editable,
                                          gpointer user_data);
static OlColor ol_color_from_gdk_color (const GdkColor color);
static GdkColor ol_color_to_gdk_color (const OlColor color);
static void ol_option_update_widget (OptionWidgets *widgets);
static char **get_list_content (GtkTreeView *view);
static void set_list_content (GtkTreeView *view, char **list);

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
static void save_osd ();
static void load_osd ();
static void save_download ();
static void load_download ();
static void save_general ();
static void load_general ();
static void init_list (struct ListExtraWidgets *widgets);

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

static void
ol_option_list_add_clicked (GtkButton *button,
                            struct ListExtraWidgets *widgets)
{
  if (button == NULL || widgets == NULL)
    return;
  if (widgets->list == NULL)
    return;
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widgets->list));
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (widgets->list));
  GtkTreeIter iter;
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_tree_selection_select_iter (selection, &iter);
}

static void
ol_option_list_remove_clicked (GtkButton *button,
                               struct ListExtraWidgets *widgets)
{
  if (button == NULL || widgets == NULL)
    return;
  if (widgets->list == NULL)
    return;
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widgets->list));
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (widgets->list));
  GtkTreeIter iter;
  gboolean selected = gtk_tree_selection_get_selected (selection, NULL, &iter);
  if (!selected)
    return;
  gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
}

static void
ol_option_list_entry_changed (GtkEditable *editable,
                              gpointer data)
{
  if (editable == NULL || data == NULL)
    return;
  struct ListExtraWidgets *widgets = (struct ListExtraWidgets *) data;
  if (widgets->list == NULL)
    return;
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widgets->list));
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (widgets->list));
  GtkTreeIter iter;
  gboolean selected = gtk_tree_selection_get_selected (selection, NULL, &iter);
  if (!selected)
    return;
  char *text = gtk_editable_get_chars (editable, 0, -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      TEXT_COLUMN, text,
                      -1);
  g_free (text);
}

void
ol_option_lrc_filename_changed (GtkEditable *editable,
                                gpointer user_data)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  static char buffer[BUFFER_SIZE] = "";
  OlMusicInfo info;
  if (options.lrc_filename_sample == NULL)
    return;
  ol_music_info_init (&info);
  info.album = "Album";
  info.title = "Title";
  info.track_number = 1;
  info.artist = "Artist";
  info.uri = "file:///music_path/music_filename.ogg";
  char *pattern = gtk_editable_get_chars (editable, 0, -1);
  if (ol_path_get_lrc_pathname ("", pattern, &info,
                                buffer, BUFFER_SIZE) >= 0)
  {
    gtk_label_set_text (GTK_LABEL (options.lrc_filename_sample), buffer + 1);
  }
  else
  {
    gtk_label_set_text (GTK_LABEL (options.lrc_filename_sample), "");
  }
  g_free (pattern);
}

static void
ol_option_list_select_changed (GtkTreeSelection *selection, gpointer data)
{
  if (selection == NULL || data == NULL)
    return;
  struct ListExtraWidgets *widgets = (struct ListExtraWidgets *) data;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean selected = gtk_tree_selection_get_selected (selection, &model, &iter);
  if (widgets->entry != NULL)
  {
    gtk_widget_set_sensitive (widgets->entry, selected);
    if (selected)
    {
      gchar *text = NULL;
      gtk_tree_model_get (model, &iter,
                          TEXT_COLUMN, &text,
                          -1);
      if (text != NULL)
      {
        gtk_entry_set_text (GTK_ENTRY (widgets->entry), text);
        g_free (text);
      }
      else
      {
        gtk_entry_set_text (GTK_ENTRY (widgets->entry), "");
      }
    }
    else
    {
      gtk_entry_set_text (GTK_ENTRY (widgets->entry), "");
    }
  }
  if (widgets->remove_button != NULL)
  {
    gtk_widget_set_sensitive (widgets->remove_button, selected);
  }
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
  load_osd ();
  load_download ();
  load_general ();
}

static void
load_osd ()
{
  int i;
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  /* Updates font */
  GtkFontButton *font = GTK_FONT_BUTTON (options.font);
  if (font != NULL)
  {
    gchar *font_family = ol_config_get_string (config,"OSD", "font-family");
    gchar *font_name = g_strdup_printf ("%s %0.0lf", font_family, ol_config_get_double (config, "OSD", "font-size"));
    gtk_font_button_set_font_name (font, font_name);
    g_free (font_name);
    g_free (font_family);
  }
  /* Updates Width */
  GtkSpinButton *width_widget = GTK_SPIN_BUTTON (options.width);
  if (width_widget != NULL)
  {
    gtk_spin_button_set_value (width_widget,
                               ol_config_get_int (config, "OSD", "width"));
  }
  /* Lrc align */
  for (i = 0; i < 2; i++)
  {
    GtkRange *lrc_align = GTK_RANGE (options.lrc_align[i]);
    if (lrc_align != NULL)
    {
      char buffer[20];
      sprintf (buffer, "lrc-align-%d", i);
      gtk_range_set_value (lrc_align,
                           ol_config_get_double (config, "OSD", buffer));
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
                                                    "OSD",
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
  /* OSD Line count */
  int line_count = ol_config_get_int (config, "OSD", "line-count");
  if (line_count < 1) line_count = 1;
  if (line_count > 2) line_count = 2;
  line_count--;
  if (options.line_count[line_count] != NULL && GTK_IS_TOGGLE_BUTTON (options.line_count[line_count]))
  {
    GtkToggleButton *line_count_widget = GTK_TOGGLE_BUTTON (options.line_count[line_count]);
    gtk_toggle_button_set_active (line_count_widget, TRUE);
  }
  /* Translucent on mouse over */
  if (options.osd_translucent != NULL)
  {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (options.osd_translucent),
                                  ol_config_get_bool (config, "OSD", "translucent-on-mouse-over"));
  }
}

static void
save_osd ()
{
  OlConfig *config = ol_config_get_instance ();
  /* Updates Font */
  GtkFontButton *font = GTK_FONT_BUTTON (options.font);
  if (font != NULL)
  {
    gchar *font_family = NULL;
    double font_size;
    ol_option_get_font_info (font, &font_family, &font_size);
    ol_config_set_string (config,
                          "OSD",
                          "font-family",
                          font_family);
    ol_config_set_double (config,
                          "OSD",
                          "font-size",
                          font_size);
    g_free (font_family);
  }
  /* Updates Width */
  GtkSpinButton *width_widget = GTK_SPIN_BUTTON (options.width);
  if (width_widget != NULL)
  {
    ol_config_set_int (config,
                       "OSD",
                       "width", gtk_spin_button_get_value (width_widget));
                               
  }
  int i;
  for (i = 0; i < 2; i++)
  {
    GtkRange *lrc_align = GTK_RANGE (options.lrc_align[i]);
    if (lrc_align != NULL)
    {
      char buffer[20];
      sprintf (buffer, "lrc-align-%d", i);
      ol_config_set_double (config,
                            "OSD",
                            buffer, 
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
                            "OSD",
                            color_props[k],
                            (const char**)lrc_color_str,
                            OL_LINEAR_COLOR_COUNT);
    g_strfreev (lrc_color_str);
  }
  /* OSD line count */
  for (i = 0; i < 2; i++)
  {
    if (options.line_count[i] != NULL && GTK_IS_TOGGLE_BUTTON (options.line_count[i]))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (options.line_count[i])))
      {
        ol_config_set_int (config,
                           "OSD",
                           "line-count", i + 1);
      }
    }
  }
  /* OSD translucent on mouse move*/
  if (options.osd_translucent != NULL)
  {
    ol_config_set_bool (config, "OSD", "translucent-on-mouse-over",
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (options.osd_translucent)));
  }
}

static void
save_download ()
{
  OlConfig *config = ol_config_get_instance ();
  /* Download Engine */
  if (options.download_engine != NULL)
  {
    int index = gtk_combo_box_get_active (GTK_COMBO_BOX (options.download_engine));
    int count = 0;
    const char **engine_list = ol_lrc_fetch_get_engine_list (&count);
    if (engine_list != NULL && index < count)
    {
      ol_config_set_string (config, "Download", "download-engine", engine_list[index]);
    }
  }
}

static void
load_download ()
{
  OlConfig *config = ol_config_get_instance ();
  /* Download engine */
  if (options.download_engine != NULL)
  {
    char *download_engine = ol_config_get_string (config, "Download", "download-engine");
    GtkTreeModel *tree = gtk_combo_box_get_model (GTK_COMBO_BOX (options.download_engine));
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first (tree, &iter);
    while (valid)
    {
      char *engine_name;
      gtk_tree_model_get (tree, &iter,
                          0, &engine_name,
                          -1);
      if (ol_stricmp (engine_name,
                      _(download_engine),
                      strlen (engine_name)) == 0)
      {
        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (options.download_engine),
                                       &iter);
        g_free (engine_name);
        break;
      }
      g_free (engine_name);
      valid = gtk_tree_model_iter_next (tree, &iter);
    }
    g_free (download_engine);
  }
}

static char **
get_list_content (GtkTreeView *view)
{
  if (view == NULL)
    return NULL;
  GtkTreeModel *model = gtk_tree_view_get_model (view);
  int cnt = gtk_tree_model_iter_n_children (model, NULL);
  char **list = g_new0 (char *, cnt + 1);
  int i = 0;
  GtkTreeIter iter;
  if (gtk_tree_model_get_iter_first (model, &iter))
  {
    do
    {
      gtk_tree_model_get (model, &iter,
                          TEXT_COLUMN, &list[i],
                          -1);

      i++;
    } while (gtk_tree_model_iter_next (model, &iter));
  }
  return list;
}

static void
save_general ()
{
  OlConfig *config = ol_config_get_instance ();
  if (options.lrc_path != NULL)
  {
    GtkTreeView *view = GTK_TREE_VIEW (options.lrc_path);
    char **list = get_list_content (view);
    if (list)
    {
      ol_config_set_str_list (config,
                              "General",
                              "lrc-path",
                              (const char **)list,
                              g_strv_length (list));
      g_strfreev (list);
    }
  }
  if (options.lrc_filename != NULL)
  {
    GtkTreeView *view = GTK_TREE_VIEW (options.lrc_filename);
    char **list = get_list_content (view);
    if (list != NULL)
    {
      ol_config_set_str_list (config,
                              "General",
                              "lrc-filename",
                              (const char **)list,
                              g_strv_length (list));
      g_strfreev (list);
    }
  }
}

static void
set_list_content (GtkTreeView *view, char **list)
{
  GtkTreeModel *model = gtk_tree_view_get_model (view);
  GtkListStore *store = GTK_LIST_STORE (model);
  gtk_list_store_clear (store);
  int i;
  GtkTreeIter iter;
  for (i = 0; list[i] != NULL; i++)
  {
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
                        TEXT_COLUMN, list[i],
                        REMOVE_COLUMN, GTK_STOCK_REMOVE,
                        -1);
    GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
    
    gtk_tree_path_free (path);
  }
}

static void
load_general ()
{
  OlConfig *config = ol_config_get_instance ();
  if (options.lrc_path != NULL)
  {
    GtkTreeView *view = GTK_TREE_VIEW (options.lrc_path);
    char **list = ol_config_get_str_list (config,
                                          "General",
                                          "lrc-path",
                                          NULL);
    if (list != NULL)
    {
      set_list_content (view, list);
      g_strfreev (list);
    }
  }
  if (options.lrc_filename != NULL)
  {
    GtkTreeView *view = GTK_TREE_VIEW (options.lrc_filename);
    char **list = ol_config_get_str_list (config,
                                          "General",
                                          "lrc-filename",
                                          NULL);
    if (list != NULL)
    {
      set_list_content (view, list);
      g_strfreev (list);
    }
  }
}

void
ol_option_ok_clicked (GtkWidget *widget)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  save_osd ();
  save_download ();
  save_general ();
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

static void
init_list (struct ListExtraWidgets *widgets)
{
  GtkTreeView *list = GTK_TREE_VIEW (widgets->list);
  if (list == NULL)
    return;
  GtkListStore *store = gtk_list_store_new (N_COLUMN,
                                            G_TYPE_STRING,
                                            G_TYPE_STRING);
  GtkCellRenderer *textcell, *btncell;
  GtkTreeViewColumn *column;
  textcell = gtk_cell_renderer_text_new ();
  btncell = ol_cell_renderer_button_new ();
  column = gtk_tree_view_column_new_with_attributes ("Pattern",
                                                     textcell,
                                                     "text", TEXT_COLUMN,
                                                     NULL);
  /* gtk_tree_view_column_pack_end (column, */
  /*                                btncell, */
  /*                                FALSE); */
  /* gtk_tree_view_column_add_attribute (column, */
  /*                                     btncell, */
  /*                                     "text", REMOVE_COLUMN); */
  gtk_tree_view_append_column (list, column);
  gtk_tree_view_set_model (list, GTK_TREE_MODEL (store));
  GtkTreeSelection *select;
  select = gtk_tree_view_get_selection (list);
  gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (select), "changed",
                    G_CALLBACK (ol_option_list_select_changed),
                    (gpointer) widgets);
  ol_option_list_select_changed (select, (gpointer) widgets);
  gtk_tree_view_set_reorderable (list, TRUE);
  /* Setup extra widgets (add, remove, text entry)*/
  if (widgets->entry != NULL)
  {
    g_signal_connect (G_OBJECT (widgets->entry), "changed",
                      G_CALLBACK (ol_option_list_entry_changed),
                      (gpointer) widgets);
  }
  if (widgets->remove_button != NULL)
  {
    g_signal_connect (G_OBJECT (widgets->remove_button), "clicked",
                      G_CALLBACK (ol_option_list_remove_clicked),
                      (gpointer) widgets);
  }
  if (widgets->add_button != NULL)
  {
    g_signal_connect (G_OBJECT (widgets->add_button), "clicked",
                      G_CALLBACK (ol_option_list_add_clicked),
                      (gpointer) widgets);
  }
}

void
ol_option_show ()
{
  static GtkWidget *window = NULL;
  if (window == NULL)
  {
    window = ol_glade_get_widget ("optiondialog");
    g_return_if_fail (window != NULL);
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
    options.line_count[0] = ol_glade_get_widget ("line-count-1");
    options.line_count[1] = ol_glade_get_widget ("line-count-2");
    options.download_engine = ol_glade_get_widget ("download-engine");
    options.osd_translucent = ol_glade_get_widget ("translucent-on-mouse-over");
    options.lrc_path = ol_glade_get_widget ("lrc-path");
    options.lrc_path_text = ol_glade_get_widget ("lrc-path-text");
    options.lrc_filename = ol_glade_get_widget ("lrc-filename");
    options.lrc_filename_text = ol_glade_get_widget ("lrc-filename-text");
    options.lrc_filename_sample = ol_glade_get_widget ("lrc-filename-sample");
    /* Init download engine combobox */
    if (options.download_engine != NULL)
    {
      int i, nengine;
      char **download_engine = ol_lrc_fetch_get_engine_list (&nengine);
      for (i = 0; i < nengine; i++)
      {
        printf ("append: %s\n", download_engine[i]);
        gtk_combo_box_append_text (GTK_COMBO_BOX (options.download_engine),
                                   _(download_engine[i]));
      }
    }
    
    lrc_path_widgets.entry = options.lrc_path_text;
    lrc_path_widgets.list = options.lrc_path;
    lrc_path_widgets.add_button = ol_glade_get_widget ("add-lrc-path");
    lrc_path_widgets.remove_button = ol_glade_get_widget ("remove-lrc-path");
    init_list (&lrc_path_widgets);
    
    lrc_filename_widgets.entry = options.lrc_filename_text;
    lrc_filename_widgets.list = options.lrc_filename;
    lrc_filename_widgets.add_button = ol_glade_get_widget ("add-lrc-filename");
    lrc_filename_widgets.remove_button = ol_glade_get_widget ("remove-lrc-filename");
    init_list (&lrc_filename_widgets);
  }
  ol_option_update_widget (&options);
  gtk_dialog_run (GTK_DIALOG (window));
}
