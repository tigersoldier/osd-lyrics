#ifndef __OL_UTILS_DBUS_H__
#define __OL_UTILS_DBUS_H__

#include <glib.h>
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
gboolean ol_dbus_get_string (DBusGProxy *proxy,
                             const gchar *method,
                             gchar **returnval);
gboolean ol_dbus_get_string_with_str_arg (DBusGProxy *proxy,
                                          const gchar *method,
                                          const gchar *arg,
                                          gchar **returnval);
gboolean ol_dbus_set_string (DBusGProxy *proxy,
                             const gchar *method,
                             const gchar *value);

gboolean ol_dbus_get_uint (DBusGProxy *proxy,
                           const gchar *method,
                           guint *returnval);
gboolean ol_dbus_set_uint (DBusGProxy *proxy,
                           const gchar *method,
                           guint value);

gboolean ol_dbus_get_int (DBusGProxy *proxy,
                          const gchar *method,
                          gint *returnval);
gboolean ol_dbus_set_int (DBusGProxy *proxy,
                          const gchar *method,
                          gint value);

gboolean ol_dbus_get_int64 (DBusGProxy *proxy,
                            const gchar *method,
                            gint64 *returnval);
gboolean ol_dbus_set_int64 (DBusGProxy *proxy,
                            const gchar *method,
                            gint64 value);

gboolean ol_dbus_get_uint8 (DBusGProxy *proxy,
                            const gchar *method,
                            guint8 *returnval);
gboolean ol_dbus_set_uint8 (DBusGProxy *proxy,
                            const gchar *method,
                            guint8 value);

gboolean ol_dbus_get_bool (DBusGProxy *proxy,
                           const gchar *method,
                           gboolean *returnval);
gboolean ol_dbus_set_bool (DBusGProxy *proxy,
                           const gchar *method,
                           gboolean value);

gboolean ol_dbus_invoke (DBusGProxy *proxy, const gchar *method);

/** 
 * Connect to a dbus service, get a proxy to invoke methods
 * 
 * @param service The service to connect.
 * @param path The path of object to connect.
 * @param interface The interface.
 * @param disconnect_handler The handler when the proxy is disconnected.
 *                           You can use ol_dbus_unref_proxy, which unreferences
 *                           the proxy object on disconnected.
 *                           The first parameter of the handler is the DBusGProxy
 *                           object that emits this signal, the second parameter is 
 *                           disconnect_data, or the pointer to the proxy if
 *                           disconnect_data is NULL.
 * @param disconnect_data The value of second parameter of disconnect_handler.
 *                        If it is set to NULL, it will be treated as the proxy
 *                        parameter. Keep it NULL if you use ol_dbus_unref_proxy
 *                        as disconnect_handler.
 * @param proxy The return location of the proxy. If it doesn't point to NULL, it
 *              WON'T free the old proxy. If connect failed, it'll be set to NULL.
 * 
 * @return TRUE if connected.
 */
gboolean ol_dbus_connect (const gchar *service,
                          const gchar *path,
                          const gchar *interface,
                          GCallback disconnect_handler,
                          gpointer disconnect_data,
                          DBusGProxy **proxy);

/** 
 * Free a DBusGProxy pointer and set the pointer to NULL.
 *
 * This function is intend to be used as default disconnect handler in
 * ol_dbus_connect
 *
 * @param object Not used, just to fit the signal handler prototype
 * @param proxy The proxy to be freed. It will be set to NULL afterwards
 */
void ol_dbus_unref_proxy (GObject *object, DBusGProxy **proxy);

/** 
 * Lists owned-names on session bus.
 * 
 * @return A NULL-terminated string array of names. If failed, NULL is returned.
 *         Should be freed with g_strfreev
 * 
 * @return 
 */
char** ol_dbus_list_names ();

gboolean ol_dbus_get_property (DBusGProxy *proxy,
                               const char *name,
                               GValue *value);

gboolean ol_dbus_get_bool_property (DBusGProxy *proxy,
                                    const char *name,
                                    gboolean *returnval);

#endif // __OL_UTILS_DBUS_H__
