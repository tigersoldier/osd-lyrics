#include <stdio.h>
#include "ol_utils_dbus.h"

static DBusGConnection *connection = NULL;
static GError *error = NULL;

DBusGConnection
*ol_dbus_get_connection ()
{
  if (connection == NULL)
  {
    connection = dbus_g_bus_get (DBUS_BUS_SESSION,
                                 &error);
    if (connection == NULL)
    {
      printf ("get connection failed: %s\n", error->message);
      g_error_free(error);
      error = NULL;
    }
  }
  return connection;
}

gboolean
ol_dbus_get_string (DBusGProxy *proxy, const gchar *method, gchar **returnval)
{
  g_return_val_if_fail (proxy != NULL, FALSE);
  g_return_val_if_fail (method != NULL, FALSE);
  g_return_val_if_fail (returnval != NULL, FALSE);
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
ol_dbus_get_string_with_str_arg (DBusGProxy *proxy, const gchar *method, const gchar *arg, gchar **returnval)
{
  g_return_val_if_fail (proxy != NULL, FALSE);
  g_return_val_if_fail (method != NULL, FALSE);
  g_return_val_if_fail (returnval != NULL, FALSE);
  g_return_val_if_fail (arg != NULL, FALSE);
  if (dbus_g_proxy_call (proxy,
                         method,
                         NULL,
                         G_TYPE_STRING,
                         arg,
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
  g_return_val_if_fail (proxy != NULL, FALSE);
  g_return_val_if_fail (method != NULL, FALSE);
  g_return_val_if_fail (returnval != NULL, FALSE);
  GError *error = NULL;
  if (dbus_g_proxy_call (proxy,
                         method,
                         &error,
                         G_TYPE_INVALID,
                         G_TYPE_UINT,
                         returnval,
                         G_TYPE_INVALID))
  {
    return TRUE;
  }
  else
  {
    fprintf (stderr, "call %s failed: %s\n", method, error->message);
    g_error_free (error);
    return FALSE;
  }
}

gboolean
ol_dbus_get_int64 (DBusGProxy *proxy, const gchar *method, gint64 *returnval)
{
  g_return_val_if_fail (proxy != NULL, FALSE);
  g_return_val_if_fail (method != NULL, FALSE);
  g_return_val_if_fail (returnval != NULL, FALSE);
  GError *error = NULL;
  if (dbus_g_proxy_call (proxy,
                         method,
                         &error,
                         G_TYPE_INVALID,
                         G_TYPE_INT64,
                         returnval,
                         G_TYPE_INVALID))
  {
    return TRUE;
  }
  else
  {
    fprintf (stderr, "call %s failed: %s\n", method, error->message);
    g_error_free (error);
    return FALSE;
  }
}

gboolean
ol_dbus_get_int (DBusGProxy *proxy, const gchar *method, gint *returnval)
{
  g_return_val_if_fail (proxy != NULL, FALSE);
  g_return_val_if_fail (method != NULL, FALSE);
  g_return_val_if_fail (returnval != NULL, FALSE);
  GError *error = NULL;
  if (dbus_g_proxy_call (proxy,
                         method,
                         &error,
                         G_TYPE_INVALID,
                         G_TYPE_INT,
                         returnval,
                         G_TYPE_INVALID))
  {
    return TRUE;
  }
  else
  {
    fprintf (stderr, "call %s failed: %s\n", method, error->message);
    g_error_free (error);
    return FALSE;
  }
}

gboolean
ol_dbus_get_uint8 (DBusGProxy *proxy, const gchar *method, guint8 *returnval)
{
  g_return_val_if_fail (proxy != NULL, FALSE);
  g_return_val_if_fail (method != NULL, FALSE);
  g_return_val_if_fail (returnval != NULL, FALSE);
  return dbus_g_proxy_call (proxy,
                            method,
                            NULL,
                            G_TYPE_INVALID,
                            G_TYPE_UCHAR,
                            returnval,
                            G_TYPE_INVALID);
}

gboolean
ol_dbus_get_bool (DBusGProxy *proxy, const gchar *method, gboolean *returnval)
{
  g_return_val_if_fail (proxy != NULL, FALSE);
  g_return_val_if_fail (method != NULL, FALSE);
  g_return_val_if_fail (returnval != NULL, FALSE);
  return dbus_g_proxy_call (proxy,
                            method,
                            NULL,
                            G_TYPE_INVALID,
                            G_TYPE_BOOLEAN,
                            returnval,
                            G_TYPE_INVALID);
}

gboolean
ol_dbus_invoke (DBusGProxy *proxy, const gchar *method)
{
  g_return_val_if_fail (proxy != NULL, FALSE);
  g_return_val_if_fail (method != NULL, FALSE);
  return dbus_g_proxy_call (proxy,
                            method,
                            NULL,
                            G_TYPE_INVALID,
                            G_TYPE_INVALID);
}
