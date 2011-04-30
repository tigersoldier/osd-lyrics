#include <string.h>
#include "ol_scroll_module.h"
#include "ol_music_info.h"
#include "ol_scroll_window.h"
#include "ol_config.h"
#include "ol_color.h"
#include "ol_stock.h"
#include "ol_image_button.h"
#include "ol_menu.h"
#include "ol_lrc.h"

typedef struct _OlScrollModule OlScrollModule;

struct OlLrc;
const int MESSAGE_TIMEOUT_MS = 5000;

struct _OlScrollModule
{
  OlMusicInfo music_info;
  gint duration;
  struct OlLrc *lrc;
  OlScrollWindow *scroll;
  guint message_timer;
};

static OlScrollModule* ol_scroll_module_new (struct OlDisplayModule *module);
static void ol_scroll_module_free (struct OlDisplayModule *module);
static void ol_scroll_module_set_music_info (struct OlDisplayModule *module,
                                             OlMusicInfo *music_info);
static void ol_scroll_module_set_played_time (struct OlDisplayModule *module,
                                              int played_time);
static void ol_scroll_module_set_lrc (struct OlDisplayModule *module,
                                      struct OlLrc *lrc_file);
static void ol_scroll_module_set_duration (struct OlDisplayModule *module,
                                           int duration);

static void ol_scroll_module_set_message (struct OlDisplayModule *module,
                                          const char *message);
static void ol_scroll_module_set_last_message (struct OlDisplayModule *module,
                                               const char *message);
static void ol_scroll_module_clear_message (struct OlDisplayModule *module);

static void _config_change_handler (OlConfig *config,
                                    gchar *group,
                                    gchar *name,
                                    gpointer userdata);
static gboolean _window_configure_cb (GtkWidget *widget,
                                      GdkEventConfigure *event,
                                      gpointer userdata);
static void _set_music_info_as_text (OlScrollModule *module);
static GtkWidget* _toolbar_new (OlScrollModule *module);
static gboolean _close_clicked_cb (GtkButton *button,
                                   gpointer userdata);
static gboolean _button_release_cb (OlScrollWindow *scroll,
                                    GdkEventButton *event,
                                    gpointer userdata);

static gboolean
_button_release_cb (OlScrollWindow *scroll,
                    GdkEventButton *event,
                    gpointer data)
{
  if (event->button == 3)
  {
    gtk_menu_popup (GTK_MENU (ol_menu_get_popup ()),
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    event->button,
                    event->time);
    return TRUE;
  }
  return FALSE;
}

static gboolean
_window_configure_cb (GtkWidget *widget,
                      GdkEventConfigure *event,
                      gpointer user_data)
{
  ol_assert_ret (GTK_IS_WINDOW (widget), FALSE);
  OlScrollModule *module = (OlScrollModule*) user_data;
  if (module == NULL)
    return FALSE;
  gint width, height;
  OlConfig *config = ol_config_get_instance ();
  gtk_window_get_size (GTK_WINDOW (widget), &width, &height);
  ol_config_set_int_no_emit (config, "ScrollMode", "width", width);
  ol_config_set_int_no_emit (config, "ScrollMode", "height", height);
  return FALSE;
}

static void
_config_change_handler (OlConfig *config,
                        gchar *group,
                        gchar *name,
                        gpointer userdata)
{
  ol_debugf ("%s:[%s]%s\n", __FUNCTION__, group, name);
  static const char *GROUP_NAME = "ScrollMode";
  OlScrollModule *module = (OlScrollModule*) userdata;
  if (module == NULL)
    return;
  OlScrollWindow *window = module->scroll;
  if (window == NULL || !OL_IS_SCROLL_WINDOW (window))
    return;
  if (strcmp (group, GROUP_NAME) != 0)
    return;
  if (strcmp (name, "font-name") == 0)
  {
    gchar *font = ol_config_get_string (config, GROUP_NAME, "font-name");
    ol_assert (font != NULL);
    ol_scroll_window_set_font_name (window, font);
    g_free (font);
  }
  else if (strcmp (name, "width") == 0 ||
           strcmp (name, "height") == 0)
  {
    gint width = ol_config_get_int (config, GROUP_NAME, "width");
    gint height = ol_config_get_int (config, GROUP_NAME, "height");
    gtk_window_resize (GTK_WINDOW (window), width, height);
  }
  else if (strcmp (name, "active-lrc-color") == 0)
  {
    char *color_str = ol_config_get_string (config, GROUP_NAME, name);
    if (color_str != NULL)
    {
      OlColor color = ol_color_from_string (color_str);
      ol_scroll_window_set_active_color (window, color);
      g_free (color_str);
    }
  }
  else if (strcmp (name, "inactive-lrc-color") == 0)
  {
    char *color_str = ol_config_get_string (config, GROUP_NAME, name);
    if (color_str != NULL)
    {
      OlColor color = ol_color_from_string (color_str);
      ol_scroll_window_set_inactive_color (window, color);
      g_free (color_str);
    }
  }
  else if (strcmp (name, "bg-color") == 0)
  {
    char *color_str = ol_config_get_string (config, GROUP_NAME, name);
    if (color_str != NULL)
    {
      OlColor color = ol_color_from_string (color_str);
      ol_scroll_window_set_bg_color (window, color);
      g_free (color_str);
    }
  }
  else if (strcmp (name, "opacity") == 0)
  {
    double opacity = ol_config_get_double (config, group, name);
    ol_scroll_window_set_bg_opacity (window, opacity);
  }
}

static gboolean
_close_clicked_cb (GtkButton *button,
                   gpointer userdata)
{
  gtk_main_quit ();
  return FALSE;
}

static GtkWidget *
_toolbar_new (OlScrollModule *module)
{
  GtkWidget *toolbar = gtk_hbox_new (FALSE, 0);
  OlImageButton *button = OL_IMAGE_BUTTON (ol_image_button_new ());
  GtkIconTheme *icontheme = gtk_icon_theme_get_default ();
  GtkIconInfo *info = gtk_icon_theme_lookup_icon (icontheme,
                                                  OL_STOCK_SCROLL_CLOSE,
                                                  16,
                                                  0);
  
  GdkPixbuf *image = gdk_pixbuf_new_from_file (gtk_icon_info_get_filename (info),
                                               NULL);
  ol_image_button_set_pixbuf (button, image);
  g_signal_connect (button, "clicked", G_CALLBACK (_close_clicked_cb), module);
  gtk_container_add (GTK_CONTAINER (toolbar),
                     GTK_WIDGET (button));
  gtk_icon_info_free (info);

  gtk_widget_show_all (toolbar);
  return toolbar;
}

static void
ol_scroll_module_init_scroll (OlScrollModule *module)
{
  ol_assert (module != NULL);
  module->scroll = OL_SCROLL_WINDOW (ol_scroll_window_new ());
  module->message_timer = 0;
  g_object_ref_sink(module->scroll);
  if (module->scroll == NULL)
  {
    return;
  }
  GtkWidget *toolbar = _toolbar_new (module);
  ol_scroll_window_add_toolbar (module->scroll,
                                toolbar);
  OlConfig *config = ol_config_get_instance ();
  _config_change_handler (config, "ScrollMode", "width", module);
  _config_change_handler (config, "ScrollMode", "font-name", module);
  _config_change_handler (config, "ScrollMode", "active-lrc-color", module);
  _config_change_handler (config, "ScrollMode", "inactive-lrc-color", module);
  _config_change_handler (config, "ScrollMode", "bg-color", module);
  _config_change_handler (config, "ScrollMode", "opacity", module);
  g_signal_connect (module->scroll, "configure-event",
                    G_CALLBACK (_window_configure_cb),
                    module);
  g_signal_connect (module->scroll, "button-release-event",
                    G_CALLBACK (_button_release_cb),
                    module);
  g_signal_connect (config, "changed",
                    G_CALLBACK (_config_change_handler),
                    module);
  
  gtk_widget_show(GTK_WIDGET (module->scroll));
}

OlScrollModule*
ol_scroll_module_new (struct OlDisplayModule *module)
{
  OlScrollModule *priv = g_new (OlScrollModule, 1);
  priv->scroll = NULL;
  priv->lrc = NULL;
  ol_music_info_init (&priv->music_info);
  ol_scroll_module_init_scroll (priv);
  return priv;
}

void
ol_scroll_module_free (struct OlDisplayModule *module)
{
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  OlConfig *config = ol_config_get_instance ();
  g_signal_handlers_disconnect_by_func (config,
                                        _config_change_handler,
                                        priv);
  if (module == NULL)
    return;
  if (priv->scroll != NULL)
  {
    gtk_widget_destroy (GTK_WIDGET (priv->scroll));
    priv->scroll = NULL;
  }
  if (priv->message_timer > 0)
    g_source_remove (priv->message_timer);
  ol_music_info_clear (&priv->music_info);
  g_free (priv);
}

static void
_set_music_info_as_text (OlScrollModule *module)
{
  ol_assert (module != NULL);
  char *text = NULL;
  if (module->scroll != NULL)
  {
    OlMusicInfo *info = &module->music_info;
    if (ol_music_info_get_title (info) != NULL)
    {
      if (ol_music_info_get_artist (info) != NULL)
        text = g_strdup_printf ("%s\n%s",
                                ol_music_info_get_title (info),
                                ol_music_info_get_artist (info));
      else
        text = g_strdup_printf ("%s", ol_music_info_get_title (info));
    }
  }
  ol_scroll_window_set_text (module->scroll, text);
  if (text != NULL)
    g_free (text);
}

void
ol_scroll_module_set_music_info (struct OlDisplayModule *module, OlMusicInfo *music_info)
{
  ol_assert (music_info != NULL); 
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  ol_music_info_copy (&priv->music_info, music_info);
  _set_music_info_as_text (priv);
}

void
ol_scroll_module_set_played_time (struct OlDisplayModule *module, int played_time)
{
  ol_assert (module != NULL);
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (priv->lrc != NULL && priv->scroll != NULL)
  {
    double percentage;
    int lyric_id;
    ol_lrc_get_lyric_by_time (priv->lrc,
                              played_time,
                              priv->duration,
                              NULL,
                              &percentage,
                              &lyric_id);
    ol_scroll_window_set_progress (priv->scroll, lyric_id, percentage);
  }
}

void
ol_scroll_module_set_lrc (struct OlDisplayModule *module, struct OlLrc *lrc_file)
{
  ol_log_func ();
  ol_assert (module != NULL);
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  priv->lrc = lrc_file;
  if(priv->scroll == NULL)
  {
      priv->scroll = OL_SCROLL_WINDOW (ol_scroll_window_new ());
  }
  if (lrc_file == NULL)
    ol_scroll_window_set_whole_lyrics(priv->scroll, NULL);
  else
  {
    /*dump the whole lyrics of a song*/
    int count = ol_lrc_item_count (lrc_file);
    GPtrArray *whole_lyrics = g_ptr_array_new_with_free_func (g_free);
    const struct OlLrcItem *info = NULL;
    int i;
    for (i = 0; i < count; i++)
    {
      info = ol_lrc_get_item (lrc_file, i);
      g_ptr_array_add (whole_lyrics, g_strdup (ol_lrc_item_get_lyric (info)));
    }
    ol_scroll_window_set_whole_lyrics(priv->scroll, whole_lyrics);
    g_ptr_array_unref (whole_lyrics);
  }
}

static void
ol_scroll_module_set_duration (struct OlDisplayModule *module, int duration)
{
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (module == NULL)
    return;
  priv->duration = duration;
}

static void
ol_scroll_module_set_message (struct OlDisplayModule *module,
                              const char *message)
{
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (priv->scroll != NULL && message != NULL)
  {
    ol_scroll_window_set_text (priv->scroll, message);
  }
}

static void
ol_scroll_module_set_last_message (struct OlDisplayModule *module,
                                   const char *message)
{
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  ol_scroll_module_set_message (module, message);
  if (priv->message_timer != 0)
    g_source_remove (priv->message_timer);
  priv->message_timer = g_timeout_add (MESSAGE_TIMEOUT_MS,
                                       (GSourceFunc) ol_scroll_module_clear_message,
                                       module);
}

static void
ol_scroll_module_clear_message (struct OlDisplayModule *module)
{
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  _set_music_info_as_text (priv);
  if (priv->message_timer > 0)
  {
    g_source_remove (priv->message_timer);
    priv->message_timer = 0;
  }
}

struct OlDisplayClass*
ol_scroll_module_get_class ()
{
  struct OlDisplayClass *klass = ol_display_class_new ("scroll",
                                                       (OlDisplayInitFunc) ol_scroll_module_new,
                                                       ol_scroll_module_free);
  klass->clear_message = ol_scroll_module_clear_message;
  klass->download_fail_message = ol_scroll_module_set_last_message;
  klass->search_fail_message = ol_scroll_module_set_last_message;
  klass->search_message = ol_scroll_module_set_message;
  klass->set_duration = ol_scroll_module_set_duration;
  klass->set_lrc = ol_scroll_module_set_lrc;
  /* klass->set_message = ol_scroll_module_set_message; */
  klass->set_music_info = ol_scroll_module_set_music_info;
  klass->set_played_time = ol_scroll_module_set_played_time;
  /* klass->set_player = ol_scroll_module_set_player; */
  /* klass->set_status = ol_scroll_module_set_status; */
  return klass;
}
