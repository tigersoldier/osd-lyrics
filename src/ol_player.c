#include <stdio.h>
#include "ol_player.h"
#include "ol_player_banshee.h"
#include "ol_player_exaile.h"

static GArray *controllers = NULL;

void
ol_player_init ()
{
  if (controllers == NULL)
  {
    controllers = g_array_new (FALSE, TRUE, sizeof (OlPlayerController*));
    ol_player_register_controller (ol_player_banshee_get_controller (), "Banshee");
    ol_player_register_controller (ol_player_exaile_get_controller (), "Exaile");
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
    printf ("asdf %d\n", i);
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
