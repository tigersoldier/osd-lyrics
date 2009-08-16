/**
 * @file   ol_keybinding.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Sun Aug 16 15:58:56 2009
 * 
 * @brief  Global Keybinding settings
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 */
#ifndef _OL_KEYBINDING_H_
#define _OL_KEYBINDING_H_

#include <gtk/gtk.h>

void ol_keybinding_init ();
GtkAccelGroup* ol_keybinding_get_accel_group ();

#endif /* _OL_KEYBINDING_H_ */
