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

import dbus
import dbus.exceptions
import dbus.service
import property as dbus_prop
import glib

__all__ = (
    'Object',
    'property',
    )

# class ObjectType(dbus.service.Object.__metaclass__):
#     def __init__(cls, name, bases, dct):
#         for k, v in self.__class__.__dict__.items():
#             if isinstance(v, dbus_prop.Property):
#                 v.__name__ = k
#         super(ObjectType, cls).__init__(name, bases, dct)

class Object(dbus.service.Object):
    """ DBus object wrapper which provides DBus property support
    """
    # __metaclass__ = ObjectType
    def __init__(self, conn=None, object_path=None, bus_name=None):
        """
        Either conn or bus_name is required; object_path is also required.
        
        Arguments:
        - `conn`: (dbus.connection.Connection or None) - The connection on which
           to export this object.

           If None, use the Bus associated with the given bus_name. If there is no
           bus_name either, the object is not initially available on any Connection.

           For backwards compatibility, if an instance of dbus.service.BusName is
           passed as the first parameter, this is equivalent to passing its
           associated Bus as conn, and passing the BusName itself as bus_name.
           
        - `object_path`: (str or None) - A D-Bus object path at which to make this
           Object available immediately. If this is not None, a conn or bus_name
           must also be provided.
           
        - `bus_name`: (dbus.service.BusName or None) - Represents a well-known name
           claimed by this process. A reference to the BusName object will be held
           by this Object, preventing the name from being released during this
           Object's lifetime (unless it's released manually).
        """
        dbus.service.Object.__init__(self, conn=conn,
                                     object_path=object_path,
                                     bus_name=bus_name)
        self._changed_props = {}
        self._prop_change_timer = None

    def _prop_changed_timeout_cb(self):
        self._prop_change_timer = None
        changed_props = {}
        for k, v in self._changed_props.items():
            iface = getattr(self.__class__, k).interface
            changed_props.setdefault(iface, {'changed': {}, 'invalidated': []})
            if v:
                changed_props[iface]['changed'][k] = getattr(self, k)
            else:
                changed_props[iface]['invalidated'].append(k)
        self._changed_props = {}
        for k, v in changed_props.items():
            self.PropertiesChanged(k, v['changed'], v['invalidated'])
        return False

    def _property_set(self, prop_name, changed):
        """ Callback for properties when a new value is set

        This method is called by properties of type osdlyrics.dbus.Property
        
        Arguments:
        - `prop_name`:
        - `changed`:
        """
        changed = changed or self._changed_props.setdefault(prop_name, changed)
        self._changed_props[prop_name] = changed
        if not self._prop_change_timer:
            self._prop_change_timer = glib.idle_add(self._prop_changed_timeout_cb)
        
    @dbus.service.method(dbus_interface=dbus.PROPERTIES_IFACE,
                         in_signature='ss',
                         out_signature='v')
    def Get(self, iface_name, prop_name):
        """ DBus property getter
        
        Arguments:
        - `iface_name`:
        - `prop_name`:
        """
        prop = getattr(self.__class__, prop_name, None)
        if isinstance(prop, dbus_prop.Property) and \
                (len(iface_name) == 0 or prop.interface == iface_name):
            return getattr(self, prop_name)
        raise dbus.exceptions.DBusException('No property of %s.%s' % (iface_name, prop_name))


    @dbus.service.method(dbus_interface=dbus.PROPERTIES_IFACE,
                         in_signature='ssv',
                         out_signature='')
    def Set(self, iface_name, prop_name, value):
        """ DBus property setter
        
        Arguments:
        - `iface_name`:
        - `prop_name`:
        - `value`:
        """
        prop = getattr(self.__class__, prop_name, None)
        if isinstance(prop, dbus_prop.Property) and \
                (len(iface_name) == 0 or prop.interface == iface_name):
            setattr(self, prop_name, value)
        else:
            raise dbus.exceptions.DBusException('No property of %s.%s' % (iface_name, prop_name))

    @dbus.service.method(dbus_interface=dbus.PROPERTIES_IFACE,
                         in_signature='s',
                         out_signature='a{sv}')
    def GetAll(self, iface_name):
        """ List all DBus properties
        
        Arguments:
        - `iface_name`:
        """
        ret = {}
        for k, v in self.__class__.__dict__.items():
            if isinstance(v, dbus_prop.Property) and \
                    (len(iface_name) == 0 or prop.interface == iface_name):
                ret[k] = getattr(self, k)
        return ret

    @dbus.service.signal(dbus_interface=dbus.PROPERTIES_IFACE,
                         signature='sa{sv}as')
    def PropertiesChanged(self, iface_name, changed_props, invalidated_props):
        print '%s changed: %s invalidated: %s' % (iface_name, changed_props, invalidated_props)
        pass
        


def property(type_signature, dbus_interface, emit_change=True):
    """
    Decorator to define dbus properties as a class member.

    To use this decorator, define your class and member as below::

       class Obj(osdlyrics.dbus.Object):
            def __init__(*args, **kwargs):
                osdlyrics.dbus.Object.__init__(*args, **kwargs)
                self._x = 0

            @osdlyrics.dbus.property(type_signature='i',
                                     dbus_interface='org.example.Interface')
            def x(self):
                return self._x

            @x.setter
            def x(self, value):
                if self._x != value:
                    self._x = value
                    return True
                else:
                    return False

    You can use the dbus property member as a property of python
    """
    def dec_handler(fget):
        """
        Arguments:
        - `fget`: getter function
        """
        return dbus_prop.Property(type_signature=type_signature,
                                  dbus_interface=dbus_interface,
                                  emit_change=emit_change,
                                  name=fget.__name__,
                                  fget=fget)
    return dec_handler

def test():
    BUS_NAME = 'org.example.test'
    IFACE = 'org.example.test'
    PATH = '/org/example/test'
    DEFAULT_VALUE = 'default value of x'
    class TestObj(Object):
        def __init__(self, loop):
            Object.__init__(self, conn=dbus.SessionBus(), object_path=PATH)
            self._loop = loop
            self._x = DEFAULT_VALUE

        @property(type_signature='s', dbus_interface=IFACE)
        def x(self):
            return self._x
            
        @x.setter
        def x(self, value):
            if value != self._x:
                self._x = value
                return True
            else:
                return False

        @dbus.service.method(dbus_interface=IFACE,
                             in_signature='',
                             out_signature='')
        def Quit(self):
            self._loop.quit()

    def get_reply_handler(value):
        print 'Get value: %s' % value

    def set_reply_handler():
        print 'Set succeed'

    def error_handler(e):
        print 'Error %s' % e

    def test_timeout():
        import time
        proxy = conn.get_object(BUS_NAME, PATH)
        proxy.GetAll('', reply_handler=get_reply_handler, error_handler=error_handler)
        proxy.Get('', 'x', reply_handler=get_reply_handler, error_handler=error_handler)
        proxy.Set('', 'x', 'new value of x', reply_handler=set_reply_handler, error_handler=error_handler)
        proxy.Get('', 'x', reply_handler=get_reply_handler, error_handler=error_handler)
        return False

    import glib
    from dbus.mainloop.glib import DBusGMainLoop
    loop = glib.MainLoop()
    dbus_mainloop = DBusGMainLoop()
    conn = dbus.SessionBus(mainloop=dbus_mainloop)
    bus_name = dbus.service.BusName(BUS_NAME, conn)
    testobj = TestObj(loop)
    glib.timeout_add(100, test_timeout)
    loop.run()
    
if __name__ == '__main__':
    test()
