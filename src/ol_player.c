#include <stdio.h>
#include "ol_player.h"
#include "ol_player_banshee.h"
#include "ol_player_exaile.h"
#include "ol_player_amarok1.h"
#include "ol_player_amarok2.h"
#include "ol_player_audacious.h"
#include "ol_player_songbird.h"
#include "ol_player_xmms2.h"
#include "ol_player_rhythmbox.h"

static GArray *controllers = NULL;

void
ol_player_init ()
{
  if (controllers == NULL)
  {
    controllers = g_array_new (FALSE, TRUE, sizeof (OlPlayerController*));
    ol_player_register_controller (ol_player_amarok1_get_controller (), "AmarOK 1.4");
    ol_player_register_controller (ol_player_amarok2_get_controller (), "AmarOK 2.x");
    ol_player_register_controller (ol_player_banshee_get_controller (), "Banshee");
    ol_player_register_controller (ol_player_exaile_get_controller (), "Exaile");
    ol_player_register_controller (ol_player_audacious_get_controller (), "Audacious");
    /* ol_player_register_controller (ol_player_songbird_get_controller (), "Songbird"); */
    /* ol_player_register_controller (ol_player_xmms2_get_controller (), "XMMS2"); */
    ol_player_register_controller (ol_player_rhythmbox_get_controller (), "Rhythmbox");
  }  
}

void
ol_player_free ()
{
  if (controllers != NULL)
  {
    g_array_free (controllers, TRUE);
    controllers = NULL;
  }
}

OlPlayerController*
ol_player_get_active_player ()
{
  printf ("%s\n",
          __FUNCTION__);
  if (controllers == NULL)
  {
    return NULL;
  }
  int i;
  printf ("controller count:%d\n", controllers->len);
  for (i = 0; i < controllers->len; i++)
  {
    OlPlayerController *controller = g_array_index (controllers, OlPlayerController*, i);
    printf ("trying player %d\n", i);
    if (controller && controller->get_activated ())
    {
      return controller;
    }
  }
  return NULL;
}

void
ol_player_register_controller (OlPlayerController *controller, const gchar *name)
{
  if (controllers == NULL)
    return;
  controller->get_activated ();
  g_array_append_val (controllers, controller);
}
