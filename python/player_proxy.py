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
import dbusext
import app
import logging
from consts import \
    PLAYER_PROXY_OBJECT_PATH_PREFIX, \
    PLAYER_PROXY_INTERFACE, \
    MPRIS1_INTERFACE

class BasePlayerProxy(dbus.service.Object):
    """ Base class to create an application to provide player proxy support
    """
    
    def __init__(self, name):
        """
        
        Arguments:
        - `name`: The suffix of the bus name. The full bus name is
          `org.osdlyrics.PlayerProxy.` + name
        """
        self._app = app.App('PlayerProxy.' + name)
        super(BasePlayerProxy, self).__init__(conn=self._app.connection,
                                             object_path=PLAYER_PROXY_OBJECT_PATH_PREFIX + name)
        self._name = name
        self._connected_players = {}

    @property
    def name(self):
        return self._name

    def run(self):
        self._app.run()

    @dbus.service.method(dbus_interface=PLAYER_PROXY_INTERFACE,
                         in_signature='',
                         out_signature='aa{sv}')
    def ListActivePlayers(self):
        return [player.to_dict() for player in self.do_list_active_players()]
    
    @dbus.service.method(dbus_interface=PLAYER_PROXY_INTERFACE,
                         in_signature='',
                         out_signature='aa{sv}')
    def ListSupportedPlayers(self):
        return [player.to_dict() for player in self.do_list_supported_players()] 

    @dbus.service.method(dbus_interface=PLAYER_PROXY_INTERFACE,
                         in_signature='',
                         out_signature='aa{sv}')
    def ListActivatablePlayers(self):
        return [player.to_dict() for player in self.do_list_activatable_players()]

    @dbus.service.method(dbus_interface=PLAYER_PROXY_INTERFACE,
                         in_signature='s',
                         out_signature='o')
    def ConnectPlayer(self, player_name):
        if self._connected_players.setdefault(player_name, None):
            return self._connected_players[player_name].object_path
        player = self.do_connect_player(player_name)
        if player and player.connected:
            player.set_disconnect_cb(self._player_lost_cb)
            self._connected_players[player_name] = player
            logging.info('Connected to %s' % player.object_path)
            return player.object_path
        else:
            raise Exception('%s cannot be connected' % player_name)

    @dbus.service.signal(dbus_interface=PLAYER_PROXY_INTERFACE,
                         signature='s')
    def PlayerLost(self, player_name):
        pass

    def _player_lost_cb(self, player):
        if player.name in self._connected_players:
            del self._connected_players[player.name]
            self.PlayerLost(player.name)

    def do_list_active_players(self):
        """
        Lists supported players that are aready running

        Returns an list of `PlayerInfo` objects.

        Derived classes must reimplement this method.
        """
        raise NotImplementedError()

    def do_list_supported_players(self):
        """
        Lists supported players.

        Returns an list of `PlayerInfo` objects.

        Derived classes must reimplement this method.
        """
        raise NotImplementedError()

    def do_list_activatable_players(self):
        """
        Lists supported players installed on the system.

        Returns an list of `PlayerInfo` objects.

        Derived classes must reimplement this method.
        """
        raise NotImplementedError()

    def do_connect_player(self, playername):
        """
        Creates an Player object according to playername.

        Returns the created `BasePlayer` object, or None if cannot connect to the
        player according to playername.
        """
        raise NotImplementedError()

class PlayerInfo(object):
    """Information about a supported player
    """
    
    def __init__(self, name, appname='', binname='', cmd='', icon=''):
        """
        """
        self._name = name
        self._appname = appname
        self._binname = binname
        self._cmd = cmd
        self._icon = icon
        
    @property
    def name(self):
        return self._name

    @property
    def appname(self):
        return self._appname

    @property
    def binname(self):
        return self._binname

    @property
    def cmd(self):
        return self._cmd

    @property
    def icon(self):
        return self._icon

    def to_dict(self):
        """
        Converts the PlayerInfo object to an dict that fits the specification
        """
        keys = ['name', 'appname', 'binname', 'cmd', 'icon']
        ret = {}
        for k in keys:
            ret[k] = getattr(self, '_' + k)
        return ret

CAPS_NEXT = 1 << 0
CAPS_PREV = 1 << 1
CAPS_PAUSE = 1 << 2
CAPS_PLAY = 1 << 3
CAPS_SEEK = 1 << 4
CAPS_PROVIDE_METADATA = 1 << 5

STATUS_PLAYING = 0
STATUS_PAUSED = 1
STATUS_STOPPED = 2

REPEAT_NONE = 0
REPEAT_TRACK = 1
REPEAT_ALL = 2

class BasePlayer(dbusext.Object):
    """ Base class of a player

    Derived classes MUST reimplement following methods:

    - `get_metadata`
    - `get_position`
    - `get_caps`

    Derived classes SHOULD reimplement following methods if supported.
    - `get_status`
    - `get_repeat`
    - `set_repeat`
    - `get_suffle`
    - `play`
    - `pause`
    - `prev`
    - `next`
    - `set_position`
    - `set_volume`
    - `get_volume`
    - `set_repeat`
    """
    
    def __init__(self, proxy, name):
        """

        Arguments:
        - `proxy`: The BasePlayerProxy object that creates the player
        - `name`: The name of the player object
        """
        super(BasePlayer, self).__init__(conn=proxy.connection,
                                         object_path=PLAYER_PROXY_OBJECT_PATH_PREFIX + \
                                         proxy.name + '/' + name)
        self._name = name
        self._proxy = proxy
        self._disconnect_cb = None
        self._connected = True

    def set_disconnect_cb(self, disconnect_cb):
        self._disconnect_cb = disconnect_cb

    @property
    def name(self):
        return self._name

    @property
    def proxy(self):
        return self._proxy

    def disconnect(self):
        if self._connected:
            self._connected = False
            self.remove_from_connection()
            if callable(self._disconnect_cb):
                self._disconnect_cb(self)

    def get_status(self):
        """
        Return the playing status.

        The return value should be one of STATUS_PLAYING, STATUS_PAUSED, and
        STATUS_STOPPED

        Derived classes that supports playing status should reimplement this.
        """
        raise NotImplementedError()

    def get_metadata(self):
        """
        Return metadata of current track

        Derived classes must reimplement this method.
        """
        raise NotImplementedError()

    def get_position(self):
        """
        Gets the ellapsed time in current track.

        Returns the time in milliseconds.
        """
        raise NotImplementedError()
        
    def get_caps(self):
        """
        Return capablities of the players.

        The return value should be a set of CAPS_PLAY, CAPS_PAUSE, CAPS_NEXT,
        CAPS_PREV, CAPS_SEEK
        """
        raise NotImplementedError()

    def get_repeat(self):
        """
        Gets the repeat mode of the player

        Returns one of REPEAT_NONE, REPEAT_TRACK, or REPEAT_ALL

        The default implementation returns REPEAT_NONE
        """
        return REPEAT_NONE

    def set_repeat(self, mode):
        """
        Sets the repeat mode of the player
        
        Arguments:
        - `mode`: REPEAT_NONE, REPEAT_TRACK, or REPEAT_ALL
        """
        raise NotImplementedError()

    def get_shuffle(self):
        """
        Gets the shuffle mode of the player

        Returns True if the playlist is shuffled, or False otherwise.

        The default implementation returns False
        """
        return False

    def set_shuffle(self, shuffled):
        """
        Set whether the tracks in track list should be played randomly.

        Arguments:
        - `shuffled`: boolean, True if shuffled
        """
        raise NotImplementedError()

    def play(self):
        """
        Start/continue playing the current track
        """
        raise NotImplementedError()

    def pause(self):
        """
        Pause the current track
        """
        raise NotImplementedError()

    def stop(self):
        """
        Stop playing.
        """
        raise NotImplementedError()

    def prev(self):
        """
        Play the previous track.
        """
        raise NotImplementedError()

    def next(self):
        """
        Play the next track.
        """
        raise NotImplementedError()

    def set_position(self, pos):
        """
        Seek to the given position.
        
        Arguments:
        - `pos`: Seek time in millisecond
        """
        raise NotImplementedError()

    def get_volume(self, pos):
        """
        Gets the volume of the player.

        Return the volume in the range of [0.0, 1.0]
        """
        raise NotImplementedError()

    def set_volume(self, volume):
        """
        Sets the volume of the player.
        
        Arguments:
        - `volume`: volume in the range of [0.0, 1.0]
        """
        raise NotImplementedError()
        
    @property
    def connected(self):
        return self._connected

    @property
    def object_path(self):
        return self._object_path

    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Next(self):
        self.next()

    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Prev(self):
        self.prev()

    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Pause(self):
        self.pause()

    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Stop(self):
        self.stop()
    
    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='')
    def Play(self):
        self.play()
    
    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='b',
                         out_signature='')
    def Repeat(self, repeat):
        if repeat:
            mode = REPEAT_ALL
        else:
            mode = REPEAT_NONE
        self.set_repeat(mode)

    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='(iiii)')
    def GetStatus(self):
        playback_status = self.get_status()
        shuffle_status = 1 if self.get_shuffle() else 0
        repeat_current = 1 if self.get_repeat() == REPEAT_TRACK else 0
        rewind = 1 if self.get_repeat() == REPEAT_TRACK else 0
        return (playback_status, shuffle_status, repeat_current, rewind)
        
    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='a{sv}')
    def GetMetadata(self):
        return self.get_metadata().to_mpris1()

    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='i')
    def GetCaps(self):
        caps = CAPS_PROVIDE_METADATA
        cap_set = self.get_caps()
        for cap in cap_set:
            caps = caps | cap
        return caps

    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='i',
                         out_signature='')
    def VolumeSet(self, volume):
        self.set_volume(volume / 100.0)

    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='i')
    def VolumeGet(self):
        return self.get_volume() * 100

    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='i',
                         out_signature='')
    def PositionSet(self, time_in_mili):
        self.set_position(time_in_mili)

    @dbus.service.method(dbus_interface=MPRIS1_INTERFACE,
                         in_signature='',
                         out_signature='i')
    def PositionGet(self):
        return int(self.get_position())

    def track_changed(self):
        self.TrackChange(self.GetMetadata())
    
    def status_changed(self):
        """
        Notify that the playing status has been changed.
        """
        self.StatusChange(self.GetStatus())

    def repeat_changed(self):
        """
        Notify the repeat mode has been changed.
        """
        self.StatusChange(self.GetStatus())

    def shuffle_changed(self):
        """
        Notify the shuffle mode has been changed.
        """
        self.StatusChange(self.GetStatus())

    def caps_changed(self):
        """
        Notify the capability of the player has been changed.
        """
        self.CapsChange(self.GetCaps())

    def position_changed(self):
        """
        Notify that the position has been changed
        """
        #DOTO: notify in MPRIS2
        pass

    @dbus.service.signal(dbus_interface=MPRIS1_INTERFACE,
                         signature='a{sv}')
    def TrackChange(self, metadata):
        pass

    @dbus.service.signal(dbus_interface=MPRIS1_INTERFACE,
                         signature='(iiii)')
    def StatusChange(self, status):
        pass

    @dbus.service.signal(dbus_interface=MPRIS1_INTERFACE,
                         signature='i')
    def CapsChange(self, caps):
        pass

class Metadata(object):
    """
    Metadata of a track
    """
    
    def __init__(self,
                 title=None,
                 artist=None,
                 album=None,
                 arturl=None,
                 tracknum=-1,
                 location=None,
                 length=-1,
                 *args,
                 **kargs):
        self.title = title
        self.artist = artist
        self.album = album
        self.arturl = arturl
        self.tracknum = tracknum
        self.location = location
        self.length = length

    def to_mpris1(self):
        """
        Converts the metadata to mpris1 dict
        """
        ret = {}
        for k in ['title', 'artist', 'album', 'arturl', 'location']:
            if getattr(self, k) is not None:
                ret[k] = dbus.String(getattr(self, k).decode('utf8'))
        if self.tracknum >= 0:
            ret['tracknumber'] = dbus.UInt32(self.tracknum)
        if self.length >= 0:
            ret['time'] = dbus.UInt32(self.length / 1000)
            ret['mtime'] = dbus.UInt32(self.length)
        return ret

    @staticmethod
    def from_mpris2(mpris2_dict):
        """
        Create a Metadata object from mpris2 metadata dict
        """
        string_dict = {'title': 'xesam:title',
                       'album': 'xesam:album',
                       'arturl': 'mpris:artUrl',
                       'location': 'xesam:url',
                       }
        string_list_dict = {'artist': 'xesam:artist',
                            }
        kargs = {}
        for k, v in string_dict.items():
            if v in mpris2_dict:
                kargs[k] = mpris2_dict[v]
        for k, v in string_list_dict.items():
            if v in mpris2_dict:
                kargs[k] = ', '.join(mpris2_dict[v])
        if 'xesam:trackNumber' in mpris2_dict:
            kargs['tracknum'] = int(mpris2_dict['xesam:trackNumber'])
        if 'mpris:length' in mpris2_dict:
            kargs['length'] = mpris2_dict['mpris:length'] / 1000
        return Metadata(**kargs)
