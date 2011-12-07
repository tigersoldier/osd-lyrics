/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2011  Tiger Soldier
 *
 * This file is part of OSD Lyrics.
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

#include <string.h>
#include "ol_config_proxy.h"
#include "ol_consts.h"
#include "ol_debug.h"

enum OlConfigProxySingals {
  SIGNAL_CHANGED = 0,
  LAST_SINGAL,
};

static guint _signals[LAST_SINGAL];
static OlConfigProxy *config_proxy = NULL;

G_DEFINE_TYPE (OlConfigProxy, ol_config_proxy, G_TYPE_DBUS_PROXY);

static OlConfigProxy *ol_config_proxy_new (void);
static void ol_config_proxy_g_signal (GDBusProxy *proxy,
                                      const gchar *sender_name,
                                      const gchar *signal_name,
                                      GVariant *parameters);
static void ol_config_proxy_value_changed_cb (OlConfigProxy *proxy,
                                              GVariant *value);

static void
ol_config_proxy_class_init (OlConfigProxyClass *klass)
{
  /* GObjectClass *gobject_class; */
  GDBusProxyClass *proxy_class;

  /* g_type_class_add_private (klass, sizeof (OlConfigProxyPrivate)); */

  /* gobject_class = G_OBJECT_CLASS (klass); */
  /* gobject_class->finalize     = ol_config_proxy_finalize; */
  /* gobject_class->get_property = org_osdlyrics_lyrics_proxy_get_property; */
  /* gobject_class->set_property = org_osdlyrics_lyrics_proxy_set_property; */

  proxy_class = G_DBUS_PROXY_CLASS (klass);
  proxy_class->g_signal = ol_config_proxy_g_signal;
  /* proxy_class->g_properties_changed = ol_lyrics_g_properties_changed; */
  _signals[SIGNAL_CHANGED] =
    g_signal_new ("value-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
                  0,            /* class_offset */
                  NULL, NULL,   /* accumulator, accu_data */
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);
}

static void
ol_config_proxy_init (OlConfigProxy *proxy)
{
}

static OlConfigProxy *
ol_config_proxy_new (void)
{
  GInitable *ret;
  ret = g_initable_new (OL_TYPE_CONFIG_PROXY,
                        NULL,   /* cancellable */
                        NULL,
                        "g-flags", G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                        "g-name", OL_SERVICE_CONFIG,
                        "g-bus-type", G_BUS_TYPE_SESSION,
                        "g-object-path", OL_OBJECT_CONFIG,
                        "g-interface-name", OL_IFACE_CONFIG,
                        NULL);
  if (ret != NULL)
    return OL_CONFIG_PROXY (ret);
  else
    return NULL;
}

static void
ol_config_proxy_g_signal (GDBusProxy *proxy,
                          const gchar *sender_name,
                          const gchar *signal_name,
                          GVariant *parameters)
{
  if (strcmp (signal_name, "ValueChanged") == 0)
  {
    ol_config_proxy_value_changed_cb (OL_CONFIG_PROXY (proxy),
                                      parameters);
  }
  else
  {
    ol_errorf ("Unknown D-Bus signal: %s\n", signal_name);
  }
}

static void
ol_config_proxy_value_changed_cb (OlConfigProxy *proxy,
                                  GVariant *parameters)
{
  GVariantIter *iter = NULL;
  g_variant_get (parameters, "(as)", &iter);
  gchar *name = NULL;
  while (g_variant_iter_loop (iter, "s", &name))
  {
    g_signal_emit (proxy,
                   _signals[SIGNAL_CHANGED],
                   g_quark_from_string (name),
                   name);
  }
  g_variant_iter_free (iter);
}

OlConfigProxy*
ol_config_proxy_get_instance (void)
{
  if (config_proxy == NULL)
    config_proxy = ol_config_proxy_new ();
  return config_proxy;
}

void
ol_config_proxy_unload (void)
{
  if (config_proxy != NULL)
  {
    g_object_unref (config_proxy);
    config_proxy = NULL;
  }
}

static gboolean
ol_config_proxy_set (OlConfigProxy *config,
                     const gchar *method,
                     GVariant *parameters)
{
  GError *error = NULL;
  GVariant *ret = g_dbus_proxy_call_sync (G_DBUS_PROXY (config),
                                          method,
                                          parameters,
                                          G_DBUS_CALL_FLAGS_NO_AUTO_START,
                                          -1,   /* timeout_secs */
                                          NULL, /* cancellable */
                                          &error);
  if (ret)
  {
    g_variant_unref (ret);
    return TRUE;
  }
  else
  {
    ol_errorf ("Cannot set config value: %s\n", error->message);
    g_error_free (error);
    return FALSE;
  }
}

gboolean
ol_config_proxy_set_bool (OlConfigProxy *config,
                          const gchar *key,
                          gboolean value)
{
  ol_assert_ret (OL_IS_CONFIG_PROXY (config), FALSE);
  ol_assert_ret (key != NULL, FALSE);
  return ol_config_proxy_set (config,
                              "SetBool",
                              g_variant_new ("(sb)", key, value));
}

gboolean
ol_config_proxy_set_int (OlConfigProxy *config,
                         const gchar *key,
                         gint value)
{
  ol_assert_ret (OL_IS_CONFIG_PROXY (config), FALSE);
  ol_assert_ret (key != NULL, FALSE);
  return ol_config_proxy_set (config,
                              "SetInt",
                              g_variant_new ("(si)", key, value));
}

gboolean
ol_config_proxy_set_double (OlConfigProxy *config,
                            const gchar *key,
                            gdouble value)
{
  ol_assert_ret (OL_IS_CONFIG_PROXY (config), FALSE);
  ol_assert_ret (key != NULL, FALSE);
  return ol_config_proxy_set (config,
                              "SetDouble",
                              g_variant_new ("(sd)", key, value));
}

gboolean
ol_config_proxy_set_string (OlConfigProxy *config,
                            const gchar *key,
                            const gchar* value)
{
  ol_assert_ret (OL_IS_CONFIG_PROXY (config), FALSE);
  ol_assert_ret (key != NULL, FALSE);
  ol_assert_ret (value != NULL, FALSE);
  return ol_config_proxy_set (config,
                              "SetString",
                              g_variant_new ("(ss)", key, value));
}

gboolean
ol_config_proxy_set_str_list (OlConfigProxy *config,
                              const gchar *key,
                              const gchar **value,
                              gint len)
{
  ol_assert_ret (OL_IS_CONFIG_PROXY (config), FALSE);
  ol_assert_ret (key != NULL, FALSE);
  ol_assert_ret (value != NULL, FALSE);
  GVariantBuilder *builder = g_variant_builder_new (G_VARIANT_TYPE ("as"));
  int i = 0;
  for (; (len < 0 || i < len) && value[i] != NULL; i++)
  {
    g_variant_builder_add (builder, "s", value[i]);
  }
  GVariant *parameters = g_variant_new ("(sas)", key, builder);
  g_variant_builder_unref (builder);
  return ol_config_proxy_set (config,
                              "SetStringList",
                              parameters);
}

static gboolean
ol_config_proxy_get (OlConfigProxy *config,
                     const gchar *method,
                     const gchar *key,
                     const gchar *format_string,
                     gpointer retval)
{
  ol_assert_ret (OL_IS_CONFIG_PROXY (config), FALSE);
  ol_assert_ret (key != NULL, FALSE);
  GError *error = NULL;
  GVariant *value = g_dbus_proxy_call_sync (G_DBUS_PROXY (config),
                                            method,
                                            g_variant_new ("(s)", key),
                                            G_DBUS_CALL_FLAGS_NO_AUTO_START,
                                            -1,   /* timeout_secs */
                                            NULL, /* cancellable */
                                            &error);
  if (!value)
  {
    if (g_dbus_error_is_remote_error (error))
    {
      gchar *error_name = g_dbus_error_get_remote_error (error);
      if (strcmp (error_name, OL_ERROR_VALUE_NOT_EXIST) == 0)
        ol_debugf ("Key %s not exists, use default value\n", key);
      else
        ol_errorf ("Failed to get config %s: %s\n", key, error->message);
      g_free (error_name);
    }
    else
    {
      ol_errorf ("%s failed. Cannot get value %s from config: %s\n",
                 method, key, error->message);
    }
    g_error_free (error);
    return FALSE;
  }
  else
  {
    g_variant_get (value, format_string, retval);
    g_variant_unref (value);
    return TRUE;
  }
}

gboolean
ol_config_proxy_get_bool (OlConfigProxy *config,
                          const gchar *key,
                          gboolean default_value)
{
  gboolean ret;
  if (ol_config_proxy_get (config, "GetBool", key, "(b)", &ret))
    return ret;
  else
    return default_value;
}

gint
ol_config_proxy_get_int (OlConfigProxy *config,
                         const gchar *key,
                         gint default_value)
{
  gint ret;
  if (ol_config_proxy_get (config, "GetInt", key, "(i)", &ret))
    return ret;
  else
    return default_value;
}

gdouble
ol_config_proxy_get_double (OlConfigProxy *config,
                            const gchar *key,
                            gdouble default_value)
{
  gint ret;
  if (ol_config_proxy_get (config, "GetDouble", key, "(d)", &ret))
    return ret;
  else
    return default_value;
}

gchar*
ol_config_proxy_get_string (OlConfigProxy *config,
                            const gchar *key,
                            const gchar *default_value)
{
  gchar *ret;
  if (ol_config_proxy_get (config, "GetDouble", key, "(d)", &ret))
    return ret;
  else
    return g_strdup (default_value);
}

gchar**
ol_config_proxy_get_str_list (OlConfigProxy *config,
                              const char *key,
                              gsize *len)
{
  ol_assert_ret (OL_IS_CONFIG_PROXY (config), NULL);
  ol_assert_ret (key != NULL, NULL);
  GError *error = NULL;
  GVariant *value = g_dbus_proxy_call_sync (G_DBUS_PROXY (config),
                                            "GetStringList",
                                            g_variant_new ("(s)", key),
                                            G_DBUS_CALL_FLAGS_NO_AUTO_START,
                                            -1,   /* timeout_secs */
                                            NULL, /* cancellable */
                                            &error);
  if (!value)
  {
    ol_errorf ("%s failed. Cannot get value %s from config: %s\n",
               "GetStringList", key, error->message);
    g_error_free (error);
    return NULL;
  }
  else
  {
    GVariantIter *iter = NULL;
    g_variant_get (value, "(as)", &iter);
    gsize list_size = g_variant_iter_n_children (iter);
    gchar **retval = g_new (gchar*, list_size + 1);
    gchar **ret_iter = retval;
    while (g_variant_iter_next (iter, "s", ret_iter))
      ret_iter++;
    retval[list_size] = NULL;
    g_variant_unref (value);
    g_variant_iter_free (iter);
    return retval;
  }
}
