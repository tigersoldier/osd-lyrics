#include "ol_utils_dbus.h"

gboolean
ol_dbus_get_string (DBusGProxy *proxy, const gchar *method, gchar **returnval)
{
  if (dbus_g_proxy_call (proxy,
                         method,
                         NULL,
                         G_TYPE_INVALID,
                         G_TYPE_STRING,
                         returnval,
                         G_TYPE_INVALID))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

gboolean
ol_dbus_get_uint (DBusGProxy *proxy, const gchar *method, guint *returnval)
{
  if (dbus_g_proxy_call (proxy,
                         method,
                         NULL,
                         G_TYPE_INVALID,
                         G_TYPE_UINT,
                         returnval,
                         G_TYPE_INVALID))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

gboolean
ol_dbus_get_uint8 (DBusGProxy *proxy, const gchar *method, guint8 *returnval)
{
  if (dbus_g_proxy_call (proxy,
                         method,
                         NULL,
                         G_TYPE_INVALID,
                         G_TYPE_UCHAR,
                         returnval,
                         G_TYPE_INVALID))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

