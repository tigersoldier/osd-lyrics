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
import xml.etree.ElementTree as xet
import glib

__all__ = (
    'Object',
    'property',
    )

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
        property_dict = {}
        for k, v in self.__class__.__dict__.items():
            if isinstance(v, dbus_prop.Property):
                property_dict.setdefault(v.interface, []).append(v)
        property_table = getattr(self, '_dbus_property_table', {})
        self._dbus_property_table = property_table
        cls = self.__class__
        property_table[cls.__module__ + '.' + cls.__name__] = property_dict

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

    def _property_set(self, prop_name, emit_with_value):
        """ Callback for properties when a new value is set

        This method is called by properties of type osdlyrics.dbus.Property
        
        Arguments:
        - `prop_name`:
        - `changed`:
        """
        self._changed_props[prop_name] = emit_with_value
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
        if isinstance(prop, dbus_prop.Property) and prop.readable and \
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
        if isinstance(prop, dbus_prop.Property) and prop.writeable and \
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
            if isinstance(v, dbus_prop.Property) and v.readable and \
                    (len(iface_name) == 0 or prop.interface == iface_name):
                ret[k] = getattr(self, k)
        return ret

    @dbus.service.signal(dbus_interface=dbus.PROPERTIES_IFACE,
                         signature='sa{sv}as')
    def PropertiesChanged(self, iface_name, changed_props, invalidated_props):
        print '%s changed: %s invalidated: %s' % (iface_name, changed_props, invalidated_props)
        pass
        
    @dbus.service.method(dbus.service.INTROSPECTABLE_IFACE, in_signature='', out_signature='s',
                         path_keyword='object_path', connection_keyword='connection')
    def Introspect(self, object_path, connection):
        """
        Patch for dbus.service.Object to add property introspection data
        """
        xml = dbus.service.Object.Introspect(self, object_path, connection)
        property_dict = self._dbus_property_table[self.__class__.__module__ + '.' + self.__class__.__name__]
        if property_dict == {}:
            return xml
        node = xet.XML(xml)
        iface_list = node.findall('interface')
        appended_iface = set()
        for iface in iface_list:
            iface_name = iface.get('name')
            if iface_name in property_dict:
                for prop in property_dict[iface_name]:
                    iface.append(_property2element(prop))
                appended_iface.add(iface_name)
        for iface_name, prop_list in property_dict.iteritems():
            if iface_name in appended_iface:
                continue
            iface = xet.Element('interface', name=iface_name)
            for prop in prop_list:
                iface.append(_property2element(prop))
            node.append(iface)
        return '<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"\n "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">\n' + \
            xet.tostring(node)


def property(type_signature,
             dbus_interface,
             emit_change=True,
             readable=True,
             writeable=True):
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

    Arguments:
    - `type_signature`: (string) The D-Bus type signature of the property
    - `dbus_interface`: (string) The D-Bus interface of the property
    - `emit_change`: (boolean or string) Whether to emit change with
                    `PropertiesChanged` D-Bus signal when the property is set.
                    Possible values are boolean value True or False, or a string
                    'invalidates'.
    - `readable`: Whether the property is able to visit with `Get` D-Bus method.
    - `writeable`: Whether the property is able to write with `Set` D-Bus method.
                   A property is writeable only when `writeable` is set to True and
                   a setter function is set.
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
                                  readable=readable,
                                  writeable=writeable,
                                  fget=fget)
    return dec_handler

def _property2element(prop):
    """Convert a osdlyrics.dbusext.Property object to xml.etree.ElementTree.ElementTree object"""
    access = ''
    if prop.readable:
        access += 'read'
    if prop.writeable and callable(prop._fset):
        access += 'write'
    elem = xet.Element('property',
                       name=prop.__name__,
                       type=prop.type_signature,
                       access=access)
    if prop.emit_change != 'true':
        annotation = xet.Element('annotation',
                                 name='org.freedesktop.DBus.Property.EmitsChangedSignal',
                                 value=prop.emit_change)
        elem.append(annotation)
    return elem

def test():
    BUS_NAME = 'org.example.test'
    IFACE = 'org.example.test'
    PATH = '/org/example/test'
    DEFAULT_VALUE = 'default value of x'
    class TestObj(Object):
        def __init__(self, loop):
            Object.__init__(self, conn=dbus.SessionBus(), object_path=PATH)
            self._loop = loop
            self._foo = DEFAULT_VALUE
            self._bar = 'yes'

        @property(type_signature='s', dbus_interface=IFACE)
        def foo(self):
            return self._foo

        @foo.setter
        def foo(self, value):
            if value != self._foo:
                self._foo = value
                return True
            else:
                return False

        @property(type_signature='s',
                  dbus_interface='another.iface',
                  readable=False, writeable=True, emit_change='invalidates')
        def bar(self):
            return self._bar

        @bar.setter
        def bar(self, value):
            self._bar = value
            
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

    def introspect_reply_handler(xml):
        print xml

    def test_timeout():
        import time
        proxy = conn.get_object(BUS_NAME, PATH)
        proxy.GetAll('', reply_handler=get_reply_handler, error_handler=error_handler)
        proxy.Get('', 'foo', reply_handler=get_reply_handler, error_handler=error_handler)
        proxy.Set('', 'foo', 'new value of x', reply_handler=set_reply_handler, error_handler=error_handler)
        proxy.Get('', 'foo', reply_handler=get_reply_handler, error_handler=error_handler)
        proxy.Introspect(reply_handler=introspect_reply_handler, error_handler=error_handler)
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
