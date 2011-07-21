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
import dbus.service
import dbus.types
import osdlyrics
import osdlyrics.dbus

from dbus.mainloop.glib import DBusGMainLoop

PROXY_NAME = 'Mpris2'
BUS_NAME = osdlyrics.PLAYER_PROXY_BUS_NAME_PREFIX + PROXY_NAME
PROXY_IFACE = osdlyrics.PLAYER_PROXY_INTERFACE
PROXY_PATH = osdlyrics.PLAYER_PROXY_OBJECT_PATH_PREFIX + PROXY_NAME
MPRIS2_PREFIX = 'org.mpris.MediaPlayer2.'
MPRIS2_IFACE = 'org.mpris.MediaPlayer2.Player'
MPRIS2_PATH = '/org/mpris/MediaPlayer2'
MPRIS1_IFACE = osdlyrics.MPRIS1_INTERFACE

MPRIS_CAPS_GO_NEXT = 1 << 0
MPRIS_CAPS_GO_PREV = 1 << 1
MPRIS_CAPS_PAUSE = 1 << 2
MPRIS_CAPS_PLAY = 1 << 3
MPRIS_CAPS_SEEK = 1 << 4
MPRIS_CAPS_PROVIDE_METADATA = 1 << 5
MPRIS_CAPS_HAS_TRACKLIST = 1 << 6

def player_info_from_name(name):
    """ Returns a dict representing a player info by the given name
    """
    return { 'name': name,
             'appname': name,
             'binname': name,
             'cmd': name,
             'icon': name
             }


class ProxyObject(dbus.service.Object):
    """ The DBus object for MPRIS2 player proxy
    """
    
    def __init__(self, bus_name):
        """
        
        Arguments:
        - `bus_name`: A well-known bus name object
        """
        self._bus = bus_name.get_bus()
        self._bus_name = bus_name
        self._connected_players = {}
        dbus.service.Object.__init__(self,
                                     bus_name = self._bus_name,
                                     object_path = PROXY_PATH)

    def _get_player_from_bus_names(self, names):
        """ Returns list of player info dicts according to names.

        The bus names in names with prefix of MPRIS2_PREFIX will be treated as MPRIS2
        players. The suffix of these names will be treated as player name
        
        Arguments:
        - `names`: list of bus names
        """
        return [player_info_from_name(name[len(MPRIS2_PREFIX):]) for name in names
                if name.startswith(MPRIS2_PREFIX)]

    @dbus.service.method(dbus_interface=PROXY_IFACE,
                         in_signature='',
                         out_signature='aa{sv}')
    def ListActivePlayers(self):
        return self._get_player_from_bus_names(self.connection.list_names());

    @dbus.service.method(dbus_interface=PROXY_IFACE,
                         in_signature='',
                         out_signature='aa{sv}')
    def ListSupportedPlayers(self):
        return self.ListActivatablePlayers()

    @dbus.service.method(dbus_interface=PROXY_IFACE,
                         in_signature='',
                         out_signature='aa{sv}')
    def ListActivatablePlayers(self):
        players = self._get_player_from_bus_names(self.connection.list_activatable_names())
        return players

    @dbus.service.method(dbus_interface=PROXY_IFACE,
                         in_signature='s',
                         out_signature='o')
    def ConnectPlayer(self, player_name):
        if self._connected_players.setdefault(player_name, None):
            return self._connected_players[player_name].object_path
        player = PlayerObject(bus=self.connection,
                              player_name=player_name,
                              disconnect_cb=self._player_lost_cb)
        if player.connected:
            self._connected_players[player_name] = player
            return player.object_path
        else:
            raise Exception('%s cannot be connected' % player_name)

    def _player_lost_cb(self, player):
        if player.name in self._connected_players:
            del self._connected_players[player.name]
            self.PlayerLost(player.name)

    @dbus.service.signal(dbus_interface=PROXY_IFACE,
                         signature='s')
    def PlayerLost(self, player_name):
        print 'name lost %s' % player_name
        pass

class PlayerObject(osdlyrics.dbus.Object):
    def __init__(self, player_name, bus, disconnect_cb=None):
        self._object_path = PROXY_PATH + '/' + player_name
        dbus.service.Object.__init__(self,
                                     conn=bus,
                                     object_path=self._object_path)
        self._bus = bus
        self._disconnect_cb = disconnect_cb
        self._name = player_name
        try:
            self._player = dbus.Interface(self._bus.get_object(MPRIS2_PREFIX + player_name,
                                                               MPRIS2_PATH),
                                          MPRIS2_IFACE)
            mpris2_object_path = MPRIS2_PREFIX + player_name
            self._player_prop = dbus.Interface(self._bus.get_object(mpris2_object_path,
                                                                    MPRIS2_PATH),
                                               dbus.PROPERTIES_IFACE)
            self._player_prop.connect_to_signal('PropertiesChanged',
                                                self._player_properties_changed)
            self.connection.watch_name_owner(mpris2_object_path,
                                             self._name_lost)
            self._connected = True
        except:
            self.disconnect()

    @property
    def name(self):
        return self._name
    
    def _name_lost(self, name):
        if len(name) > 0:
            return
        if callable(self._disconnect_cb):
            self._disconnect_cb(self)
        self.disconnect()
            
    def disconnect(self):
        if self._connected:
            self._connected = False
            self.remove_from_connection()

    def _player_properties_changed(self, iface, changed, invalidated):
        caps_props = ['CanGoNext', 'CanGoPrevious', 'CanPlay', 'CanPause', 'CanSeek']
        status_props = ['PlaybackStatus', 'LoopStatus', 'Shuffle']
        for caps in caps_props:
            if caps in changed:
                self.CapsChange(self._get_caps())
                break
        for status in status_props:
            if status in changed:
                self.StatusChange(self._get_status())
        if 'Metadata' in changed:
            self.TrackChange(self._get_metadata())

    @property
    def object_path(self):
        return self._object_path
            
    @property
    def connected(self):
        return self._connected

    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='',
                         out_signature='')
    def Next(self):
        self._player.Next()

    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='',
                         out_signature='')
    def Prev(self):
        self._player.Previous()

    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='',
                         out_signature='')
    def Pause(self):
        self._player.Pause()

    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='',
                         out_signature='')
    def Stop(self):
        self._player.Stop()
    
    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='',
                         out_signature='')
    def Play(self):
        self._player.Play()
    
    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='b',
                         out_signature='')
    def Repeat(self, repeat):
        try:
            if repeat:
                self._player_prop.Set(MPRIS2_IFACE, 'LoopStatus', 'Playlist')
            else:
                self._player_prop.Set(MPRIS2_IFACE, 'LoopStatus', 'None')
        except:
            pass

    def _get_status(self):
        playback_status = 0
        shuffle_status = 0
        repeat_current = 0
        rewind = 0
        try:
            playback_dict = {'Playing': 0,
                             'Paused': 1,
                             'Stopped': 2}
            playback_status = status_dict.setdefault(
                self._player_prop.Get(MPRIS2_IFACE, 'PlaybackStatus'), 0)
        except:
            pass

        try:
            if self._player_prop.Get(MPRIS2_IFACE, 'Shuffle'):
                shuffle_status = 1
        except:
            pass
        
        try:
            loop = self._player_prop.Get(MPRIS2_IFACE, 'LoopStatus')
            if loop == 'Track':
                repeat_current = 1
            elif loop == 'Playlist':
                rewind = 1
        except:
            pass
        return (playback_status, shuffle_status, repeat_current, rewind)
    
    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='',
                         out_signature='(iiii)')
    def GetStatus(self):
        return self._get_status()

    def _get_metadata(self):
        metadata = self._player_prop.Get(MPRIS2_IFACE, 'Metadata')
        print metadata
        ret = {}
        string_dict = {'title': 'xesam:title',
                       'album': 'xesam:album',
                       'arturl': 'mpris:artUrl',
                       'location': 'xesam:url',
                       }
        string_list_dict = {'artist': 'xesam:artist',
                            'genre': 'xesam:genre',
                            'comment': 'xesam:comment',
                            }
        for k, v in string_dict.items():
            if v in metadata:
                ret[k] = metadata[v]
        for k, v in string_list_dict.items():
            if v in metadata:
                ret[k] = ', '.join(metadata[v])

        if 'xesam:trackNumber' in metadata:
            ret['tracknumber'] = str(metadata['xesam:trackNumber'])
        ret['time'] = dbus.types.UInt32(metadata.setdefault('mpris:length', 0) / 1000000)
        ret['mtime'] = dbus.types.UInt32(metadata.setdefault('mpris:length', 0) / 1000)
        return ret

    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='',
                         out_signature='a{sv}')
    def GetMetadata(self):
        return self._get_metadata()

    def _get_caps(self):
        caps = 0
        caps_dict = { 'CanGoNext': MPRIS_CAPS_GO_NEXT,
                      'CanGoPrevious': MPRIS_CAPS_GO_PREV,
                      'CanPlay': MPRIS_CAPS_PLAY,
                      'CanPause': MPRIS_CAPS_PAUSE,
                      'CanSeek': MPRIS_CAPS_SEEK,
            }
        for k, v in caps_dict.items():
            if self._player_prop.Get(MPRIS2_IFACE, k):
                caps = caps | v

        caps = caps | MPRIS_CAPS_PROVIDE_METADATA
        return caps

    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='',
                         out_signature='i')
    def GetCaps(self):
        return self._get_caps()
        
    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='i',
                         out_signature='')
    def VolumeSet(self, volume):
        self._player_prop.Set(MPRIS2_IFACE, 'Volume', volume / 100.0)
    
    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='',
                         out_signature='i')
    def VolumeGet(self):
        return self._player_prop.Get(MPRIS2_IFACE, 'Volume') * 100
    
    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='i',
                         out_signature='')
    def PositionSet(self, time_in_mili):
        track_id = self._player_prop.Get(MPRIS2_IFACE, 'Metadata')['mpris:trackid']
        self._player.SetPosition(track_id, time_in_mili * 1000)
    
    @dbus.service.method(dbus_interface=MPRIS1_IFACE,
                         in_signature='',
                         out_signature='i')
    def PositionGet(self):
        return self._player_prop.Get(MPRIS2_IFACE, 'Position') / 1000

    @dbus.service.signal(dbus_interface=MPRIS1_IFACE,
                         signature='a{sv}')
    def TrackChange(self, metadata):
        pass

    @dbus.service.signal(dbus_interface=MPRIS1_IFACE,
                         signature='(iiii)')
    def StatusChange(self, status):
        pass

    @dbus.service.signal(dbus_interface=MPRIS1_IFACE,
                         signature='i')
    def CapsChange(self, caps):
        pass

def run():
    import glib
    def daemon_name_changed(name):
        if len(name) == 0:
            print 'Daemon is not running, exit'
            loop.quit()
    
    def watch_daemon_bus(name):
        if len(name) > 0:
            bus.watch_name_owner(osdlyrics.BUS_NAME, daemon_name_changed)

    def get_options():
        from optparse import OptionParser
        parser = OptionParser()
        parser.add_option('-w', '--watch-daemon',
                          dest='watch_daemon',
                          action='store',
                          default=osdlyrics.BUS_NAME,
                          metavar='BUS_NAME',
                          help='A well-known bus name on DBus. Exit when the ' \
                          'name disappears. If set to empty string, this player ' \
                          'proxy will not exit.')
        (options, args) = parser.parse_args()
        return options

    options = get_options()
    loop = glib.MainLoop()
    dbus_mainloop = DBusGMainLoop()
    bus = dbus.SessionBus(mainloop=dbus_mainloop)
    bus_name = dbus.service.BusName(BUS_NAME, bus)
    mpris_proxy = ProxyObject(bus_name)
    watch_daemon_bus(options.watch_daemon)
    loop.run()

if __name__ == '__main__':
    run()
