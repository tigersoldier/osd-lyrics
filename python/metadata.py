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
import utils

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
                ret[k] = dbus.String(utils.ensure_unicode(getattr(self, k)))
        if self.tracknum >= 0:
            ret['tracknumber'] = dbus.String(self.tracknum)
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
