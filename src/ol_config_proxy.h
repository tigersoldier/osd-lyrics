/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009-2011  Tiger Soldier <tigersoldier@gmail.com>
 *
 * This file is part of OSD Lyrics.
 * 
 * OSD Lyrics is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OSD Lyrics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OSD Lyrics.  If not, see <http://www.gnu.org/licenses/>. 
 */
/**
 * @file   ol_config_proxy.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Wed Jun 17 09:56:19 2009
 * 
 * @brief  Provide a singleton config object for getting/setting config of this app
 * 
 * 
 */
#ifndef _OL_CONFIG_PROXY_H_
#define _OL_CONFIG_PROXY_H_

#include <gio/gio.h>

#define OL_TYPE_CONFIG_PROXY                    \
  (ol_config_proxy_get_type ())
#define OL_CONFIG_PROXY(obj)                                        \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OL_TYPE_CONFIG_PROXY, OlConfigProxy))
#define OL_IS_CONFIG_PROXY(obj)                             \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OL_TYPE_CONFIG_PROXY))
#define OL_CONFIG_PROXY_CLASS(klass)                                    \
  (G_TYPE_CHECK_CLASS_CAST ((klass), OL_TYPE_CONFIG_PROXY, OlConfigProxyClass))
#define OL_IS_CONFIG_PROXY_CLASS(klass)                   \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), OL_TYPE_CONFIG_PROXY))
#define OL_CONFIG_PROXY_GET_CLASS(obj)                                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), OL_TYPE_CONFIG_PROXY, OlConfigProxyClass))

typedef struct _OlConfigProxy OlConfigProxy;
typedef struct _OlConfigProxyClass OlConfigProxyClass;

struct _OlConfigProxy
{
  GDBusProxy parent;
};

struct _OlConfigProxyClass
{
  GDBusProxyClass parent;
};

GType ol_config_proxy_get_type (void);

/** 
 * @brief Gets the singleton instance of OlConfigProxy
 * 
 * 
 * @return An instance of OlConfigProxy, should not be freed
 */
OlConfigProxy* ol_config_proxy_get_instance (void);

/** 
 * @brief Sets an boolean property to the config
 * 
 * @param config An OlConfigProxy
 * @param key The key of the config value, should be in the form of "group/name", or
 *            any name started with a dot, like '.visible'. A key starting with a dot
 *            indicates that it is a temporary config entry and should not be sync
 *            to the config file.
 * @param value The value of the property
 * 
 * @return If succeed, returns TRUE
 */
gboolean ol_config_proxy_set_bool (OlConfigProxy *config,
                                   const gchar *key,
                                   gboolean value);

/** 
 * @brief Sets an int property to the config
 * 
 * @param config An OlConfigProxy
 * @param key The key of the config value, should be in the form of "group/name", or
 *            any name started with a dot, like '.visible'. A key starting with a dot
 *            indicates that it is a temporary config entry and should not be sync
 *            to the config file.
 * @param value The value of the property
 * 
 * @return If succeed, returns TRUE
 */
gboolean ol_config_proxy_set_int (OlConfigProxy *config,
                                  const gchar *key,
                                  gint value);

/** 
 * @brief Sets a double property to the config
 * 
 * @param config An OlConfigProxy
 * @param key The key of the config value, should be in the form of "group/name", or
 *            any name started with a dot, like '.visible'. A key starting with a dot
 *            indicates that it is a temporary config entry and should not be sync
 *            to the config file.
 * @param value The value of the property
 * 
 * @return If succeed, returns TRUE
 */
gboolean ol_config_proxy_set_double (OlConfigProxy *config,
                                     const gchar *key,
                                     gdouble value);

/** 
 * @brief Sets a string property to the config
 * 
 * @param config An OlConfigProxy
 * @param key The key of the config value, should be in the form of "group/name", or
 *            any name started with a dot, like '.visible'. A key starting with a dot
 *            indicates that it is a temporary config entry and should not be sync
 *            to the config file.
 * @param value The value of the property
 * 
 * @return If succeed, returns TRUE
 */
gboolean ol_config_proxy_set_string (OlConfigProxy *config,
                                     const gchar *key,
                                     const gchar* value);

/** 
 * @brief Sets a string list property to the config
 * 
 * @param config An OlConfigProxy
 * @param key The key of the config value, should be in the form of "group/name", or
 *            any name started with a dot, like '.visible'. A key starting with a dot
 *            indicates that it is a temporary config entry and should not be sync
 *            to the config file.
 * @param value The value of the property
 * @param len The length of the string property. -1 to indicate the list is
 *            terminated with NULL.
 * 
 * @return If succeed, returns TRUE
 */
gboolean ol_config_proxy_set_str_list (OlConfigProxy *config,
                                 const gchar *key,
                                 const gchar * const *value,
                                 gint len);

/** 
 * @brief Gets a boolean property from the config
 * 
 * @param config An OlConfigProxy
 * @param key The key of the config value, should be in the form of "group/name", or
 *            any name started with a dot, like '.visible'. A key starting with a dot
 *            indicates that it is a temporary config entry and should not be sync
 *            to the config file.
 * @param default_value The default value to return if the config service is
 *                      unavaliable or the key is not found in the config.
 * 
 * @return If succeed, returns int value of the content. \
 *         If fail or the key does not exist, FALSE is returned.
 */
gboolean ol_config_proxy_get_bool (OlConfigProxy *config,
                                   const gchar *key);

/** 
 * @brief Gets a int property from the config
 * 
 * @param config An OlConfigProxy
 * @param key The key of the config value, should be in the form of "group/name", or
 *            any name started with a dot, like '.visible'. A key starting with a dot
 *            indicates that it is a temporary config entry and should not be sync
 *            to the config file.
 * @param default_value The default value to return if the config service is
 *                      unavaliable or the key is not found in the config.
 * 
 * @return If succeed, returns int value of the content. \
 *         If fail or the key does not exist, 0 is retuened.
 */
gint ol_config_proxy_get_int (OlConfigProxy *config,
                              const gchar *key);
/** 
 * @brief Gets a double property from the config
 * 
 * @param config An OlConfigProxy
 * @param key The key of the config value, should be in the form of "group/name", or
 *            any name started with a dot, like '.visible'. A key starting with a dot
 *            indicates that it is a temporary config entry and should not be sync
 *            to the config file.
 * @param default_value The default value to return if the config service is
 *                      unavaliable or the key is not found in the config.
 * 
 * @return If succeed, returns double value of the content. \
 *         If fail or the key does not exist, 0.0 is returned.
 */
gdouble ol_config_proxy_get_double (OlConfigProxy *config,
                                    const gchar *key);
/** 
 * @brief Gets a string property of the config
 * 
 * @param config An OlConfigProxy
 * @param key The key of the config value, should be in the form of "group/name", or
 *            any name started with a dot, like '.visible'. A key starting with a dot
 *            indicates that it is a temporary config entry and should not be sync
 *            to the config file.
 * @param default_value The default value to return if the config service is
 *                      unavaliable or the key is not found in the config.
 * 
 * @return If succeed, returns duplicated string value of the content, \
 *         must be freed by g_free. If fail or the key does not exist, returns NULL.
 */
gchar* ol_config_proxy_get_string (OlConfigProxy *config,
                                   const gchar *key);

/** 
 * @brief Gets a string property of the config
 * 
 * @param config An OlConfigProxy
 * @param key The key of the config value, should be in the form of "group/name", or
 *            any name started with a dot, like '.visible'. A key starting with a dot
 *            indicates that it is a temporary config entry and should not be sync
 *            to the config file.
 * @param len return location of length of string list, or NULL
 * 
 * @return a NULL-terminated string array or NULL if the specified key cannot
 *         be found. The array should be freed with g_strfreev().
 *         If fail or the key does not exist, NULL will be returned.
 */
gchar** ol_config_proxy_get_str_list (OlConfigProxy *config,
                                      const char *key,
                                      gsize *len);

/** 
 * @brief Unload config module
 * 
 */
void ol_config_proxy_unload (void);

#endif /* _OL_CONFIG_PROXY_H_ */
