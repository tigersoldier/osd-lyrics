#include "config.h"
#if HAVE_APP_INDICATOR
#include <libappindicator/app-indicator.h>
#endif  /* HAVE_APP_INDICATOR */
#include "ol_trayicon.h"
#include "ol_app.h"
#include "ol_intl.h"
#include "ol_menu.h"
#include "ol_stock.h"
#include "ol_player.h"
#include "ol_config.h"
#include "ol_debug.h"

static const char *UNKNOWN_TITLE = N_("Unknown title");
static const char *UNKNOWN_ARTIST = N_("Unknown artist");
static const char *INFO_FORMAT = "<big><b>%s</b></big>\n"
                                 "  %s";
static const char *INFO_FORMAT_ALBUM = "<big><b>%s</b></big>\n"
                                       "  %s - <i>%s</i>";

static void
activate (GtkStatusIcon* status_icon,
          gpointer user_data)
{
  OlConfig *config = ol_config_get_instance ();
  ol_config_set_bool (config, "General", "visible",
                      !ol_config_get_bool (config, "General", "visible"));
}

static gboolean internal_query_tooltip (GtkStatusIcon *status_icon,
                                        gint           x,
                                        gint           y,
                                        gboolean       keyboard_mode,
                                        GtkTooltip    *tooltip,
                                        gpointer       user_data);
#if HAVE_APP_INDICATOR
static AppIndicator *indicator = NULL;
#else
static GtkStatusIcon *status_icon = NULL;
#endif

static gboolean
internal_query_tooltip (GtkStatusIcon *status_icon,
                        gint           x,
                        gint           y,
                        gboolean       keyboard_mode,
                        GtkTooltip    *tooltip,
                        gpointer       user_data)
{
  OlMusicInfo *info = ol_app_get_current_music ();
  if (info == NULL ||
      (ol_music_info_get_title (info) == NULL &&
       ol_music_info_get_artist (info) == NULL))
  {
    gtk_tooltip_set_text (tooltip, _("OSD Lyrics"));
  }
  else
  {
    const char *title = ol_music_info_get_title (info);
    if (title == NULL)
      title = _(UNKNOWN_TITLE);
    const char *artist = ol_music_info_get_artist (info);
    if (artist == NULL)
      artist = _(UNKNOWN_ARTIST);
    const char *album = ol_music_info_get_album (info);
    char *markup = NULL;
    if (album == NULL)
    {
      markup = g_strdup_printf (INFO_FORMAT,
                                title,
                                artist);
    }
    else
    {
      markup = g_strdup_printf (INFO_FORMAT_ALBUM,
                                title,
                                artist,
                                album);
    }
    gtk_tooltip_set_markup (tooltip, markup);
    g_free (markup);
    
    struct OlPlayer *player = ol_app_get_player ();
    const char *icon_path = ol_player_get_icon_path (player);
    GdkPixbuf *icon = NULL;
    if (icon_path != NULL)
    {
      icon = gdk_pixbuf_new_from_file_at_scale (icon_path, 48, 48, FALSE, NULL);
    }
    gtk_tooltip_set_icon (tooltip,
                          icon);
    if (icon != NULL)
      g_object_unref (icon);
  }
  return TRUE;
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

void ol_trayicon_inital ()
{
#if HAVE_APP_INDICATOR
  if (indicator == NULL)
  {
    indicator = app_indicator_new (PACKAGE,
                                   OL_STOCK_TRAYICON,
                                   APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_attention_icon (indicator, OL_STOCK_TRAYICON);
    app_indicator_set_menu (indicator, GTK_MENU (ol_menu_get_popup ()));
  }
#else

  if (status_icon == NULL)
  {
    status_icon = gtk_status_icon_new_from_stock (OL_STOCK_TRAYICON);
    gtk_status_icon_set_visible (status_icon, TRUE);
    gtk_status_icon_set_tooltip (status_icon, _("OSD Lyrics"));

    /* Connect signals */
    g_signal_connect (G_OBJECT (status_icon), "popup-menu",
                      G_CALLBACK (popup), NULL);

    g_signal_connect (G_OBJECT (status_icon), "query-tooltip",
                      G_CALLBACK (internal_query_tooltip), NULL);

    g_signal_connect (G_OBJECT (status_icon), "activate",
                      G_CALLBACK (activate), NULL);
  }
#endif  /* HAVE_APP_INDICATOR */
}

void
ol_trayicon_free ()
{
#if HAVE_APP_INDICATOR
  if (indicator != NULL)
  {
    g_object_unref (indicator);
    indicator = NULL;
  }
#else
  if (status_icon != NULL)
  {
    g_object_unref (status_icon);
    status_icon = NULL;
  }
#endif
}

void
ol_trayicon_status_changed (enum OlPlayerStatus status)
{
#if HAVE_APP_INDICATOR
  if (indicator != NULL)
    app_indicator_set_menu (indicator, GTK_MENU (ol_menu_get_popup ()));
#else
  /* Do nothing */
#endif
  
}
