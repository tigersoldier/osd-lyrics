#include "ol_keybindings.h"
#include "ol_keybinding_settings.h"
#include "ol_keybinder.h"
#include "ol_commands.h"

static GtkAccelGroup *accel = NULL;

static gboolean ol_hide_accel (gpointer userdata);

static gboolean
ol_hide_accel (gpointer userdata)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  return FALSE;
}
  

void
ol_keybinding_init ()
{
  ol_keybinder_init ();
  GtkAccelGroup *accel = ol_keybinding_get_accel_group ();
  GClosure *hide_closure = g_cclosure_new (ol_hide_accel,
                                           NULL,
                                           NULL);
  gtk_accel_map_add_entry ("<OSD Lyrics>/Hide",
                           gdk_keyval_from_name ("h"),
                           GDK_CONTROL_MASK | GDK_SHIFT_MASK);
  gtk_accel_group_connect_by_path (accel,
                                   "<OSD Lyrics>/Hide",
                                   hide_closure);
  gtk_accel_map_add_entry ("<OSD Lyrics>/Lock",
                           gdk_keyval_from_name ("l"),
                           GDK_CONTROL_MASK | GDK_SHIFT_MASK);
  gtk_accel_group_connect_by_path (accel,
                                   "<OSD Lyrics>/Hide",
                                   hide_closure);
  ol_keybinder_bind ("<Ctrl><Shift>H", ol_show_hide, NULL);
  ol_keybinder_bind ("<Ctrl><Shift>L", ol_osd_lock_unlock, NULL);
}

GtkAccelGroup*
ol_keybinding_get_accel_group ()
{
  if (accel == NULL)
  {
    accel = gtk_accel_group_new ();
  }
  return accel;
}
