/**
 * @file   ol_utils_dcop.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Mon May 18 14:15:26 2009
 * 
 * @brief  Utilities for DCOP operation
 * 
 * 
 */
#ifndef _OL_UTILS_DCOP_H_
#define _OL_UTILS_DCOP_H_
/** 
 * @brief Executes the command in cmd, fetch the output as a string and return it .
 * 
 * @param cmd The DCOP command
 * @param returnval The point to the returned string, should be freed by g_free
 * 
 * @return If succeeded, return TRUE
 */
gboolean ol_dcop_get_string (const gchar *cmd, gchar **returnval);

gboolean ol_dcop_get_uint (const gchar *cmd, guint *returnval);

gboolean ol_dcop_get_boolean (const gchar *cmd, gboolean *returnval);

#endif /* _OL_UTILS_DCOP_H_ */
