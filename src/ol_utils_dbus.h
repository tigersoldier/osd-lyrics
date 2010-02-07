#ifndef __OL_UTILS_DBUS_H__
#define __OL_UTILS_DBUS_H__

#include <dbus/dbus-glib.h>

/** 
 * @brief Gets the connection of dbus
 * 
 * 
 * @return A singleton connection, shouldn't be freed
 */
DBusGConnection *ol_dbus_get_connection ();

/** 
 * @brief Invokes a dbus method without parameter and returns a string
 * 
 * @param proxy A DBusGProxy
 * @param method The name of method to invoke
 * @param returnval The point to the returned string, should be freed by g_free
 * 
 * @return If succeeded, return TRUE
 */
gboolean ol_dbus_get_string (DBusGProxy *proxy, const gchar *method, gchar **returnval);
gboolean ol_dbus_get_string_with_str_arg (DBusGProxy *proxy, const gchar *method, const gchar *arg, gchar **returnval);

gboolean ol_dbus_get_uint (DBusGProxy *proxy, const gchar *method, guint *returnval);

gboolean ol_dbus_get_int (DBusGProxy *proxy, const gchar *method, gint *returnval);

gboolean ol_dbus_get_int64 (DBusGProxy *proxy, const gchar *method, gint64 *returnval);

gboolean ol_dbus_get_uint8 (DBusGProxy *proxy, const gchar *method, guint8 *returnval);

gboolean ol_dbus_get_bool (DBusGProxy *proxy, const gchar *method, gboolean *returnval);

gboolean ol_dbus_invoke (DBusGProxy *proxy, const gchar *method);

#endif // __OL_UTILS_DBUS_H__
