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

import dbus.service
import osdlyrics
import osdlyrics.dbusext
import glib
import config

class PlayerSupport(dbus.service.Object):
    """ Implement osd.lyrics.Player Interface
    """

    DETECT_PLAYER_TIMEOUT = 1000
    
    def __init__(self, conn):
        """
        Arguments:
         - `conn`: DBus connection of the object
        """
        dbus.service.Object.__init__(self,
                                     conn=conn,
                                     object_path=osdlyrics.PLAYER_OBJECT_PATH)
        self._active_player = None
        self._player_proxies = {}
        self._activate_player_proxies()
        self._start_detect_player()
        self._mpris1_root = Mpris1Root(conn)
        self._mpris1_player = Mpris1Player(conn)

    def _start_detect_player(self):
        self._detect_timer = glib.timeout_add(self.DETECT_PLAYER_TIMEOUT,
                                              self._detect_player)
        
    def _detect_player(self):
        """
        Detects active players.

        This is a callback function of a timer. If active player detected, try to
        connect to the player and remove the timer.
        """
        detected = False
        for proxy in self._player_proxies.values():
            try:
                active_players = proxy.ListActivePlayers()
                for player_info in active_players:
                    if self._connect_player(proxy, player_info):
                        detected = True
                        break
                if detected:
                    break
            except:
                pass
        if detected:
            glib.source_remove(self._detect_timer)
            self._detect_timer = None
        return True

    def _activate_player_proxies(self):
        """
        Activates all player proxy services
        """
        activatable_names = self.connection.list_activatable_names()
        for bus_name in activatable_names:
            if not bus_name.startswith(osdlyrics.PLAYER_PROXY_BUS_NAME_PREFIX):
                continue
            try:
                proxy_name = bus_name[len(osdlyrics.PLAYER_PROXY_BUS_NAME_PREFIX):]
                self.connection.activate_name_owner(bus_name)
                self.connection.watch_name_owner(bus_name,
                                                 lambda name: self._proxy_name_changed(proxy_name, len(name) == 0))
            except Exception, err:
                pass

    def _connect_player(self, proxy, player_info):
        """
        Tries to connect player through proxy

        Return True if connect successful
        """
        try:
            path = proxy.ConnectPlayer(player_info['name'])
            player = self.connection.get_object(proxy.bus_name,
                                                path)
            player = dbus.Interface(player, osdlyrics.MPRIS1_INTERFACE)
            self._active_player = {'info': player_info,
                                   'player': player,
                                   'proxy': proxy}
            self._mpris1_player.connect_player(player)
            self.PlayerConnected(player_info)
            return True
        except Exception, e:
            return False

    def _player_lost_cb(self, player_name):
        if self._active_player['info']['name'] == player_name:
            self._active_player = None
            self._mpris1_player.disconnect_player()
            self.PlayerLost()
            self._start_detect_player()

    def _proxy_name_changed(self, proxy_name, lost):
        bus_name = osdlyrics.PLAYER_PROXY_BUS_NAME_PREFIX + proxy_name
        if not lost:
            print 'Get proxy %s' % proxy_name
            proxy = self.connection.get_object(bus_name,
                                               osdlyrics.PLAYER_PROXY_OBJECT_PATH_PREFIX + proxy_name)
            proxy.connect_to_signal('PlayerLost',
                                        self._player_lost_cb)
            self._player_proxies[proxy_name] = dbus.Interface(proxy, osdlyrics.PLAYER_PROXY_INTERFACE)
        else:
            if not proxy_name in self._player_proxies:
                return
            print 'Proxy %s lost' % proxy_name
            proxy = self._player_proxies[proxy_name]
            # If current player is provided by the proxy, it is lost.
            if self._active_player and self._active_player['proxy'] == proxy:
                self._player_lost_cb(self._active_player['info']['name'])
                del self._player_proxies[proxy_name]
            # Try to reactivate proxy
            try:
                self.connection.activate_name_owner(bus_name)
            except:
                pass

    @dbus.service.method(dbus_interface=osdlyrics.PLAYER_INTERFACE,
                         in_signature='',
                         out_signature='aa{sv}')
    def ListSupportedPlayers(self):
        ret = []
        for proxy in self._player_proxies.values():
            try:
                ret = ret + proxy.ListSupportedPlayers()
            except:
                pass
        return ret

    @dbus.service.method(dbus_interface=osdlyrics.PLAYER_INTERFACE,
                         in_signature='',
                         out_signature='aa{sv}')
    def ListActivatablePlayers(self):
        ret = []
        for proxy in self._player_proxies.values():
            try:
                ret = ret + proxy.ListActivatablePlayers()
            except:
                pass
        return ret

    @dbus.service.method(dbus_interface=osdlyrics.PLAYER_INTERFACE,
                         in_signature='',
                         out_signature='ba{sv}')
    def GetCurrentPlayer(self):
        if not self._active_player:
            return False, None
        return True, self._active_player['info']

    @dbus.service.signal(dbus_interface=osdlyrics.PLAYER_INTERFACE,
                         signature='')
    def PlayerLost(self):
        pass

    @dbus.service.signal(dbus_interface=osdlyrics.PLAYER_INTERFACE,
                         signature='a{sv}')
    def PlayerConnected(self, player_info):
        pass

class Mpris1Root(osdlyrics.dbusext.Object):
    """ Root object of MPRIS1
    """
    
    def __init__(self, conn):
        osdlyrics.dbusext.Object.__init__(self, conn=conn, object_path='/')

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='s')
    def Identity(self):
        return config.PROGRAM_NAME

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Quit(self):
        # We won't quit by this message
        pass

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='(qq)')
    def MprisVersion(self):
        return (1, 0)


class Mpris1Player(osdlyrics.dbusext.Object):
    """
    /Player object of MPRIS1
    """
    def __init__(self, conn):
        osdlyrics.dbusext.Object.__init__(self, conn=conn, object_path='/Player')
        self._player = None
        self._player_bus = ''
        self._player_path = ''
        self._signals = []

    def connect_player(self, player_proxy):
        if self._player == player_proxy:
            return
        if self._player:
            self.disconnect_player()
        self._player = player_proxy
        self._signals.append(self._player.connect_to_signal('TrackChange',
                                                            self.TrackChange))
        self._signals.append(self._player.connect_to_signal('CapsChange',
                                                            self.CapsChange))
        self._signals.append(self._player.connect_to_signal('StatusChange',
                                                            self.StatusChange))

    def disconnect_player():
        for signal in self._signals:
            signal.remove()
        self._signals = []
        del self._player
        self._player = None

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Next(self):
        if self._player:
            self._player.Next()

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Prev(self):
        if self._player:
            self._player.Prev()

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Pause(self):
        if self._player:
            self._player.Pause()

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Stop(self):
        if self._player:
            self._player.Stop()

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Play(self):
        if self._player:
            self._player.Play()

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='b',
                         out_signature='')
    def Repeat(self, repeat):
        if self._player:
            self._player.Repeat(repeat)

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='(iiii)')
    def GetStatus(self):
        if self._player:
            return self._player.GetStatus()
        else:
            return (0, 0, 0, 0)

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='a{sv}')
    def GetMetadata(self):
        if self._player:
            return self._player.GetMetadata()
        else:
            return {}

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='i')
    def GetCaps(self):
        if self._player:
            return self._player.GetCaps()
        else:
            return 0

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='i',
                         out_signature='')
    def VolumeSet(self, volume):
        if self._player:
            self._player.VolumeSet(volume)

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='i')
    def VolumeGet(self):
        if self._player:
            return self._player.VolumeGet()
        else:
            return 0

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='i',
                         out_signature='')
    def PositionSet(self, pos):
        if self._player:
            self._player.PositionSet(pos)
        else:
            return 0

    @dbus.service.method(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='i')
    def PositionGet(self):
        if self._player:
            return self._player.PositionGet()
        else:
            return 0

    @dbus.service.signal(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         signature='a{sv}')
    def TrackChange(self, track):
        pass

    @dbus.service.signal(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         signature='(iiii)')
    def StatusChange(self, status):
        pass

    @dbus.service.signal(dbus_interface=osdlyrics.MPRIS1_INTERFACE,
                         signature='i')
    def CapsChange(self, caps):
        pass

def test():
    app = osdlyrics.App('osdlyrics')
    player_support = PlayerSupport(app.connection)
    app.run()
        
if __name__ == '__main__':
    test()
