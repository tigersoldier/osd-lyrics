# -*- coding: utf-8 -*-
#
# Copyright (C) 2011  Tiger Soldier
#
# This file is part of OSD Lyrics.
# 
# OSD Lyrics is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OSD Lyrics is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OSD Lyrics.  If not, see <http://www.gnu.org/licenses/>. 
#/

__all__ = (
    'Property',
    )

class Property(object):
    """ DBus property class
    """

    def __init__(self, dbus_interface, type_signature, emit_change=True, name=None, fget=None, fset=None):
        """
        
        Arguments:
        - `type_signature`: (string) Type signature of this property. This parameter
          is used for introspection only
        - `dbus_interface`: (string) The DBus interface of this property
        """
        self._type_signature = type_signature
        # we use two underscores because dbus-python uses _dbus_interface to determine
        # whether an object is a dbus method
        self.__dbus_interface = dbus_interface
        self._fset = fset
        self._fget = fget
        self.__name__ = name
        self._emit_change = emit_change
        
    @property
    def interface(self):
        """ Return the dbus interface of this property
        """
        return self.__dbus_interface

    def __get__(self, obj, objtype=None):
        if obj is None:
            return self
        if self._fget is None:
            raise AttributeError, "unreadable attribute"
        return self._fget(obj)
        
    def __set__(self, obj, value):
        if self._fset is None:
            raise AttributeError, "can't set attribute"
        changed = self._fset(obj, value)
        if not self._emit_change:
            return
        if changed is None or changed:
            changed = True
        else:
            changed = False
        if getattr(self, '__name__', None) and getattr(obj, '_property_set', None):
            obj._property_set(self.__name__, changed)

    def setter(self, fset):
        """
        Sets the setter function of the property. Return the property object itself.

        The setter function should return a boolean value to tell if the value
        is changed. Return nothing is the same as True.

        When ``emit_change`` is True in constructor, the property will emit changes
        when value is set. If the owner object has a function named
        ``_property_set``, and the ``__name__`` attribute of the property object is
        set by the class, the function will be invoked to notify the property has
        been set when the setter is called, with the name of property as the first
        argument, and an boolean as the second argument to tell whether the value
        is changed.

        This is usually used as an decorator::

            class A(osdlyrics.dbus.Object):
            
              @osdlyrics.dbus.property(type='s', dbus_interface='example.property')
              def x(self):
                  return self._x

              @x.setter
              def x(self, value)
                  if self._x != value:
                      self._x = value
                      return True
                  else:
                      return False
        """
        self._fset = fset
        return self
