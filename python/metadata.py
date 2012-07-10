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
import re

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
            kargs['length'] = int(mpris2_dict['mpris:length']) / 1000
        return Metadata(**kargs)

    @staticmethod
    def from_dict(dbusdict):
        """
        Create a Metadata object from a D-Bus dict object.

        The D-Bus dict object can be MPRIS1 metadata or MPRIS2 metadata format. If
        the dict both compatable with MPRIS1 and MPRIS2, MPRIS1 will be used.

        >>> title = 'Title'
        >>> artist = 'Artist'
        >>> arturl = 'file:///art/url'
        >>> location = 'file///location'
        >>> tracknumber = 42
        >>> md1 = Metadata.from_dict({'title': title,
        ...                           'artist': artist,
        ...                           'arturl': arturl,
        ...                           'location': location,
        ...                           'tracknumber': str(tracknumber) + '/2'})
        >>> md1.title == title
        True
        >>> md1.artist == artist
        True
        >>> md1.arturl == arturl
        True
        >>> md1.location == location
        True
        >>> md1.tracknum == tracknumber
        True
        >>> md2 = Metadata.from_dict({'xesam:title': title,
        ...                           'xesam:artist': [artist],
        ...                           'mpris:artUrl': arturl,
        ...                           'xesam:url': location,
        ...                           'xesam:trackNumber': tracknumber})
        >>> md2.title == title
        True
        >>> md2.artist == artist
        True
        >>> md2.arturl == arturl
        True
        >>> md2.location == location
        True
        >>> md2.tracknum == tracknumber
        True
        >>> md3 = Metadata.from_dict({'title': title,
        ...                           'artist': artist,
        ...                           'arturl': arturl,
        ...                           'location': location,
        ...                           'tracknumber': str(tracknumber) + '/2',
        ...                           'xesam:title': title + '1',
        ...                           'xesam:artist': [artist + '1', '1'],
        ...                           'mpris:artUrl': arturl + '1',
        ...                           'xesam:url': location + '1',
        ...                           'xesam:trackNumber': tracknumber + 1})
        >>> md3.title == title
        True
        >>> md3.artist == artist
        True
        >>> md3.arturl == arturl
        True
        >>> md3.location == location
        True
        >>> md3.tracknum == tracknumber
        True
        >>> timedict = {'time': 10, 'mtime': 20, 'mpris:length': 3000}
        >>> Metadata.from_dict(timedict).length
        20
        >>> del timedict['mtime']
        >>> Metadata.from_dict(timedict).length
        3
        >>> del timedict['mpris:length']
        >>> Metadata.from_dict(timedict).length
        10000
        """
        string_dict = {'title': ['title', 'xesam:title'],
                       'album': ['album', 'xesam:album'],
                       'arturl': ['arturl', 'mpris:artUrl'],
                       'artist': ['artist'],
                       'location': ['location', 'xesam:url'],
                       }
        string_list_dict = {'artist': 'xesam:artist',
                            }
        kargs = {}
        for k, v in string_dict.items():
            for dict_key in v:
                if dict_key in dbusdict:
                    kargs[k] = dbusdict[dict_key]
                    break;
        # artist
        for k, v in string_list_dict.items():
            if k not in kargs and v in dbusdict:
                kargs[k] = ', '.join(dbusdict[v])
        # tracknumber
        if 'tracknumber' in dbusdict:
            tracknumber = dbusdict['tracknumber']
            if not re.match(r'\d+(/\d+)?', tracknumber):
                logging.warning('Malfromed tracknumber: %s' % tracknumber)
            else:
                kargs['tracknum'] = int(dbusdict['tracknumber'].split('/')[0])
        if 'tracknum' not in kargs and 'xesam:trackNumber' in dbusdict:
            kargs['tracknum'] = int(dbusdict['xesam:trackNumber'])

        # length
        if 'mtime' in dbusdict:
            kargs['length'] = dbusdict['mtime']
        elif 'mpris:length' in dbusdict:
            kargs['length'] = int(dbusdict['mpris:length']) / 1000
        elif 'time' in dbusdict:
            kargs['length'] = dbusdict['time'] * 1000
        return Metadata(**kargs)

if __name__ == '__main__':
    import doctest
    doctest.testmod()
