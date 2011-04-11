#include "ol_utils_dbus.h"
#include "ol_test_util.h"

void
test_list_name ()
{
  char **names = ol_dbus_list_names ();
  ol_test_expect (names != NULL);
  g_strfreev (names);
}

void
test_prop ()
{
  DBusGProxy *proxy = NULL;
  if (!ol_dbus_connect ("org.mpris.MediaPlayer2.rhythmbox",
                        "/org/mpris/MediaPlayer2",
                        "org.mpris.MediaPlayer2.Player",
                        NULL,
                        NULL,
                        &proxy))
  {
    printf ("You need to launch Rhythmbox in order to test dbus property\n");
  }
  else
  {
    gboolean canplay = FALSE;
    ol_test_expect (ol_dbus_get_bool_property (proxy, "CanPlay", &canplay));
    ol_test_expect (canplay);
    g_object_unref (proxy);
  }
}

int
main ()
{
  g_type_init ();
  test_list_name ();
  test_prop ();
  return 0;
}
