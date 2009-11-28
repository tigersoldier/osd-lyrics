/**
 * @file   ol_menu.c
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Sun Aug 16 16:18:41 2009
 * 
 * @brief  Popup menu for OSD lyrics
 * 
 * 
 */

#include "ol_menu.h"
#include "ol_intl.h"
#include "ol_config.h"
#include "ol_about.h"
#include "ol_option.h"
#include "ol_keybindings.h"
#include "ol_commands.h"
#include "ol_app.h"
#include "ol_glade.h"
#include "ol_debug.h"

static void ol_config_changed (OlConfig *config, gchar *group, gchar *name, gpointer data);
static GtkWidget *popup_menu = NULL;
/* enum { */
/*   OL_MENU_LOCK, */
/*   OL_MENU_HIDE, */
/*   OL_MENU_PERFERENCE, */
/*   OL_MENU_ABOUT, */
/*   OL_MENU_QUIT, */
/*   OL_MENU_COUNT, */
/* }; */

static struct Menu
{
  GtkWidget *lock;
  GtkWidget *hide;
  GtkWidget *play;
  GtkWidget *pause;
  GtkWidget *stop;
  GtkWidget *prev;
  GtkWidget *next;
  GtkWidget *preference;
  GtkWidget *about;
  GtkWidget *quit;
} menu = {0};

/* static GtkMenuItem *items[OL_MENU_COUNT] = {0}; */
void ol_menu_lock (GtkWidget *widget, gpointer data);
void ol_menu_hide (GtkWidget *widget, gpointer data);
void ol_menu_quit (GtkWidget *widget, gpointer data);
void ol_menu_about (GtkWidget *widget, gpointer data);
void ol_menu_preference (GtkWidget *widget, gpointer data);
void ol_menu_play (GtkWidget *widget, gpointer data);
void ol_menu_pause (GtkWidget *widget, gpointer data);
void ol_menu_stop (GtkWidget *widget, gpointer data);
void ol_menu_prev (GtkWidget *widget, gpointer data);
void ol_menu_next (GtkWidget *widget, gpointer data);
void ol_menu_download (GtkWidget *widget, gpointer data);

void
ol_menu_download (GtkWidget *widget, gpointer data)
{
  if (ol_app_get_controller () != NULL)
  {
    OlMusicInfo *info = ol_app_get_current_music ();
    ol_app_download_lyric (info);
  }
}

static void
ol_config_changed (OlConfig *config, gchar *group, gchar *name, gpointer data)
{
  ol_log_func ();
  ol_logf (OL_DEBUG, "  name:%s\n", name);
  if (strcmp (name, "locked") == 0)
  {
    gboolean locked = ol_config_get_bool (config, "OSD", "locked");
    if (menu.lock != NULL &&
        locked != gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menu.lock)))
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu.lock),
                                      locked);
  }
  else if (strcmp (name, "visible") == 0)
  {
    gboolean visible = ol_config_get_bool (config, "General", name);
    if (menu.hide &&
        visible == gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menu.hide)))
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu.hide),
                                      !visible);
  }
}

void
ol_menu_lock (GtkWidget *widget, gpointer data)
{
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  gboolean locked = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget));
  ol_config_set_bool (config, "OSD",  "locked", locked);
}

void
ol_menu_hide (GtkWidget *widget, gpointer data)
{
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  gboolean hide = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget));
  ol_config_set_bool (config, "General", "visible", !hide);
}

void
ol_menu_quit (GtkWidget *widget,
              gpointer data)
{
  gtk_main_quit ();
}

void
ol_menu_about (GtkWidget *widget, gpointer data)
{
  ol_about_show ();
}

void
ol_menu_preference (GtkWidget *widget, gpointer data)
{
  ol_option_show ();
}

void
ol_menu_play (GtkWidget *widget, gpointer data)
{
  OlPlayerController *player = ol_app_get_controller ();
  if (player == NULL)
    return;
  ol_log_func ();
  ol_player_play (player);
}

void
ol_menu_pause (GtkWidget *widget, gpointer data)
{
  OlPlayerController *player = ol_app_get_controller ();
  if (player == NULL)
    return;
  ol_player_pause (player);
}

void
ol_menu_stop (GtkWidget *widget, gpointer data)
{
  OlPlayerController *player = ol_app_get_controller ();
  if (player == NULL)
    return;
  ol_player_stop (player);
}

void
ol_menu_prev (GtkWidget *widget, gpointer data)
{
  OlPlayerController *player = ol_app_get_controller ();
  if (player == NULL)
    return;
  ol_player_prev (player);
}

void
ol_menu_next (GtkWidget *widget, gpointer data)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  OlPlayerController *player = ol_app_get_controller ();
  if (player == NULL)
    return;
  ol_player_next (player);
}

static void
ol_menu_init ()
{
  /* create accelerator group */
  GtkAccelGroup *accel = ol_keybinding_get_accel_group ();
  GtkWidget *item;
  OlConfig *config = ol_config_get_instance ();
  popup_menu = ol_glade_get_widget ("pop-menu");
  gtk_menu_set_accel_group (GTK_MENU (popup_menu), accel);
  menu.lock = ol_glade_get_widget ("menu-lock");
  if (menu.lock)
  {
    gtk_menu_item_set_accel_path (GTK_MENU_ITEM (menu.lock),
                                  "<OSD Lyrics>/Lock");
  }
  ol_config_changed (config, "OSD", "locked", NULL);

  menu.hide = ol_glade_get_widget ("menu-hide");
  if (menu.hide)
  {
    gtk_menu_item_set_accel_path (GTK_MENU_ITEM (menu.hide),
                                  "<OSD Lyrics>/Hide");
  }
  ol_config_changed (config, "General", "visible", NULL);

  menu.preference = ol_glade_get_widget ("menu-prefernce");
  menu.about = ol_glade_get_widget ("menu-about");
  menu.quit = ol_glade_get_widget ("menu-quit");

  menu.play = ol_glade_get_widget ("menu-play");
  menu.pause = ol_glade_get_widget ("menu-pause");
  menu.stop = ol_glade_get_widget ("menu-stop");
  menu.prev = ol_glade_get_widget ("menu-prev");
  menu.next = ol_glade_get_widget ("menu-next");
  
  g_signal_connect (config,
                    "changed",
                    G_CALLBACK (ol_config_changed),
                    NULL);
  gtk_widget_show_all (popup_menu);
}

static void
ol_menu_update_player_control ()
{
  ol_log_func ();
  OlPlayerController *controller = ol_app_get_controller ();
  if (menu.play)
  {
    if (controller != NULL)
    {
      if ((ol_player_get_capacity (controller) & OL_PLAYER_STATUS) &&
          ol_player_get_status (controller) == OL_PLAYER_PLAYING)
      {
        gtk_widget_hide (menu.play);
      }
      else
      {
        gtk_widget_show (menu.play);
        gtk_widget_set_sensitive (menu.play,
                                  ol_player_get_capacity (controller) & OL_PLAYER_PLAY);
      }
    }
    else
    {
      gtk_widget_show (menu.play);
      gtk_widget_set_sensitive (menu.play, FALSE);
    }
  }
  if (menu.pause)
  {
    if (controller != NULL)
    {
      if ((ol_player_get_capacity (controller) & OL_PLAYER_STATUS))
      {
        if (ol_player_get_status (controller) == OL_PLAYER_PLAYING)
          gtk_widget_show (menu.pause);
        else
          gtk_widget_hide (menu.pause);
      }
      else
      {
        gtk_widget_show(menu.pause);
      }
      gtk_widget_set_sensitive (menu.pause,
                                ol_player_get_capacity (controller) & OL_PLAYER_PAUSE);
    }
    else
    {
      gtk_widget_hide (menu.pause);
    }
  }
  if (menu.stop)
    gtk_widget_set_sensitive (menu.stop,
                              controller != NULL &&
                              (ol_player_get_capacity (controller) & OL_PLAYER_STOP));
  if (menu.prev)
    gtk_widget_set_sensitive (menu.prev,
                              controller != NULL &&
                              (ol_player_get_capacity (controller) & OL_PLAYER_PREV));
  if (menu.next)
    gtk_widget_set_sensitive (menu.next,
                              controller != NULL &&
                              (ol_player_get_capacity (controller) & OL_PLAYER_NEXT));
}

GtkWidget*
ol_menu_get_popup ()
{
  ol_log_func ();
  if (popup_menu == NULL)
  {
    ol_menu_init ();
  }
  ol_menu_update_player_control ();
  return popup_menu;
}
