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
 * @file   ol_config.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Wed Jun 17 09:56:19 2009
 * 
 * @brief  Provide a singleton config object for getting/setting config of this app
 * 
 * 
 */
#ifndef _OL_CONFIG_H_
#define _OL_CONFIG_H_

#include <glib-object.h>

#define OL_TYPE_CONFIG                  (ol_config_get_type ())
#define OL_CONFIG(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OL_TYPE_CONFIG, OlConfig))
#define OL_IS_CONFIG(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OL_TYPE_CONFIG))
#define OL_CONFIG_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OL_TYPE_CONFIG, OlConfigClass))
#define OL_IS_CONFIG_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OL_TYPE_CONFIG))
#define OL_CONFIG_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OL_TYPE_CONFIG, OlConfigClass))

typedef struct _OlConfig OlConfig;
typedef struct _OlConfigClass OlConfigClass;

enum OlConfigSingals {
  INVALID_SIGNAL = 0,
  CHANGED,
  CONFIG_SINGAL_COUNT,
};

struct _OlConfig
{
  GObject parent;
};

struct _OlConfigClass
{
  GObjectClass parent;
  guint signals[CONFIG_SINGAL_COUNT];
};

GType ol_config_get_type (void);

/** 
 * @brief Gets the singleton instance of OlConfig
 * 
 * 
 * @return An instance of OlConfig, should not be freed
 */
OlConfig* ol_config_get_instance (void);

/** 
 * @brief Sets an boolean property of the config
 * 
 * @param config An OlConfig
 * @param group The name of group the property belongs to
 * @param name The name of the property
 * @param value The value of the property
 * 
 * @return If succeed, returns TRUE
 */
gboolean ol_config_set_bool (OlConfig *config, const char *group, const char *name, gboolean value);

/** 
 * @brief Sets an int property of the config
 * 
 * @param config An OlConfig
 * @param name The name of the property
 * @param value The value of the property
 * 
 * @return If succeed, returns TRUE
 */
gboolean ol_config_set_int (OlConfig *config, const char *group, const char *name, int value);

gboolean ol_config_set_int_no_emit (OlConfig *config,
                                    const char *group,
                                    const char *name,
                                    int value);

/** 
 * @brief Sets a double property of the config
 * 
 * @param config An OlConfig
 * @param name The name of the property
 * @param value The value of the property
 * 
 * @return If succeed, returns TRUE
 */
gboolean ol_config_set_double (OlConfig *config, const char *group, const char *name, double value);

/** 
 * @brief Sets a string property of the config
 * 
 * @param config An OlConfig
 * @param name The name of the property
 * @param value The value of the property
 * 
 * @return If succeed, returns TRUE
 */
gboolean ol_config_set_string (OlConfig *config, const char *group, const char *name, const char* value);

/** 
 * @brief Sets a string list property of the config
 * 
 * @param config An OlConfig
 * @param name The name of the property
 * @param value The value of the property
 * @param len The length of the string property
 * 
 * @return If succeed, returns TRUE
 */
gboolean ol_config_set_str_list (OlConfig *config,
                                 const char *group,
                                 const char *name,
                                 const char **value,
                                 gsize len);

/** 
 * @brief Gets a boolean property of the config
 * 
 * @param config An OlConfig
 * @param name The name of the property
 * 
 * @return If succeed, returns int value of the content. \
 *         If fail, returns FALSE.
 */
gboolean ol_config_get_bool (OlConfig *config, const char *group, const char *name);

/** 
 * @brief Gets a int property of the config
 * 
 * @param config An OlConfig
 * @param name The name of the property
 * 
 * @return If succeed, returns int value of the content. \
 *         If fail, returns 0.
 */
int ol_config_get_int (OlConfig *config, const char *group, const char *name);

/** 
 * @brief Gets a double property of the config
 * 
 * @param config An OlConfig
 * @param name The name of the property
 * 
 * @return If succeed, returns double value of the content. \
 *         If fail, returns 0.0.
 */
double ol_config_get_double (OlConfig *config, const char *group, const char *name);

/** 
 * @brief Gets a string property of the config
 * 
 * @param config An OlConfig
 * @param name The name of the property
 * 
 * @return If succeed, returns duplicated string value of the content, \
 *         must be freed by g_free. If fail, returns NULL.
 */
char* ol_config_get_string (OlConfig *config, const char *group, const char *name);

/** 
 * @brief Gets a string property of the config
 * 
 * @param config An OlConfig
 * @param name The name of the property
 * @param len return location of length of string list, or NULL
 * 
 * @return a NULL-terminated string array or NULL if the specified key cannot be found.
 * The array should be freed with g_strfreev().
 */
char** ol_config_get_str_list (OlConfig *config,
                               const char *group,
                               const char *name,
                               gsize *len);

/** 
 * @brief Flush config value into config file
 * 
 * @param config 
 */
void ol_config_save (OlConfig *config);

/** 
 * @brief Load config value from config file
 * 
 * @param config 
 */
void ol_config_load (OlConfig *config);

/** 
 * @brief Unload config module
 * 
 */
void ol_config_unload (void);

#endif /* _OL_CONFIG_H_ */
