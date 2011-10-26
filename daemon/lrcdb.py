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

import sqlite3
import os.path
import config
import urllib
import osdlyrics.utils
from osdlyrics.utils import ensure_unicode, ensure_utf8
from osdlyrics.consts import METADATA_URI, METADATA_TITLE, METADATA_ALBUM, \
    METADATA_ARTIST, METADATA_TRACKNUM

__all__ = (
    'LrcDb',
    )

def unicode_metadata(metadata):
    ret = {}
    for k, v in metadata.items():
        ret[k] = ensure_unicode(v)
    return ret

def normalize_location(location):
    """
    Normalize location of metadata to URI form

    >>> normalize_location('/path/to/file')
    u'file:///path/to/file'
    >>> normalize_location(u'/path/to/file')
    u'file:///path/to/file'
    >>> normalize_location('file:///path/to/file')
    u'file:///path/to/file'
    >>> normalize_location('/\xe6\x96\x87\xe4\xbb\xb6/\xe8\xb7\xaf\xe5\xbe\x84')
    u'file:///%E6%96%87%E4%BB%B6/%E8%B7%AF%E5%BE%84'
    >>> normalize_location(u'/\u6587\u4ef6/\u8def\u5f84')
    u'file:///%E6%96%87%E4%BB%B6/%E8%B7%AF%E5%BE%84'
    """
    if location and location[0] == '/':
        location = 'file://' + urllib.pathname2url(ensure_utf8(location))
    return location.decode('utf8')

class LrcDb(object):
    """ Database to store location of LRC files
    """
    
    TABLE_NAME = 'lyrics'

    CREATE_TABLE = """
CREATE TABLE IF NOT EXISTS %s (
  id INTEGER PRIMARY KEY AUTOINCREMENT, 
  title TEXT, artist TEXT, album TEXT, tracknum INTEGER,
  uri TEXT UNIQUE ON CONFLICT REPLACE, lrcpath TEXT
)
""" % TABLE_NAME

    ASSIGN_LYRIC = """
INSERT OR REPLACE INTO %s 
  (title, artist, album, tracknum, uri, lrcpath)
  VALUES (?, ?, ?, ?, ?, ?)
""" % TABLE_NAME

    UPDATE_LYRIC = """
UPDATE %s
  SET lrcpath=?
  WHERE uri=?
""" % TABLE_NAME
    
    FIND_LYRIC = 'SELECT lrcpath FROM %s WHERE ' % TABLE_NAME;

    QUERY_LOCATION = 'uri = ?'

    QUERY_INFO = {'title': 'title', 'artist': 'artist', 'album': 'album'}

    def __init__(self, dbfile=None):
        """
        
        Arguments:
        - `dbfile`: The sqlite db to open
        """
        if dbfile is None:
            dbfile = osdlyrics.utils.get_config_path('lrc.db')
        self._dbfile = dbfile
        self._conn = sqlite3.connect(os.path.expanduser(dbfile))
        self._create_table()

    def _create_table(self):
        """ Ensures the table structure of new open dbs
        """
        c = self._conn.cursor()
        c.execute(LrcDb.CREATE_TABLE)
        self._conn.commit()
        c.close()

    def assign(self, metadata, uri):
        """ Assigns a uri of lyrics to tracks represented by metadata
        """
        uri = ensure_unicode(uri)
        metadata = unicode_metadata(metadata)
        c = self._conn.cursor()
        if METADATA_URI in metadata:
            location = normalize_location(metadata[METADATA_URI])
        else:
            location = ''
        if self._find_by_location(metadata):
            c.execute(LrcDb.UPDATE_LYRIC, (uri, location,))
        else:
            title = metadata[METADATA_TITLE] if METADATA_TITLE in metadata else ''
            artist = metadata[METADATA_ARTIST] if METADATA_ARTIST in metadata else ''
            album = metadata[METADATA_ALBUM] if METADATA_ALBUM in metadata else ''
            try:
                tracknum = int(metadata[METADATA_TRACKNUM])
            except:
                tracknum = 0
            c.execute(LrcDb.ASSIGN_LYRIC, (title, artist, album, tracknum, location, uri))
        self._conn.commit()
        c.close()

    def find(self, metadata):
        """ Finds the location of LRC files for given metadata

        To find the location of lyrics, firstly find whether there is a record matched
        with the ``location`` attribute in metadata. If not found or ``location`` is
        not specified, try to find with respect to ``title``, ``artist``, ``album``
        and ``tracknumber``
        
        If found, return the uri of the LRC file. Otherwise return None. Note that
        this method may return an empty string, so use ``is None`` to figure out
        whether an uri is found
        """
        metadata = unicode_metadata(metadata)
        ret = self._find_by_location(metadata)
        if ret is not None:
            return ret
        ret = self._find_by_info(metadata)
        if ret is not None:
            return ret
        return None

    def _find_by_condition(self, where_clause, parameters=None):
        query = LrcDb.FIND_LYRIC + where_clause
        c = self._conn.cursor()
        c.execute(query, parameters)
        r = c.fetchone()
        if r:
            return r[0]
        return None

    def _find_by_location(self, metadata):
        location = metadata.setdefault('location', None)
        if not location:
            return None
        location = normalize_location(location)
        return self._find_by_condition(LrcDb.QUERY_LOCATION, (location,))

    def _find_by_info(self, metadata):
        query = []
        for mkey, qkey in LrcDb.QUERY_INFO.items():
            if mkey in metadata:
                query.append('%s=:%s' % (qkey, mkey))
        if len(query) > 0:
            return self._find_by_condition(' AND '.join(query),
                                           metadata)
        return None

def test():
    """
    >>> import dbus
    >>> db = LrcDb('/tmp/asdf')
    >>> db.assign({'title': 'Tiger', 'artist': 'Soldier', 'location': '/tmp/asdf'}, \
    'file:///tmp/a.lrc')
    >>> db.find({'location': '/tmp/asdf'})
    u'file:///tmp/a.lrc'
    >>> db.find({'location': '/tmp/asdfg'})
    >>> db.find({'title': 'Tiger', 'location': '/tmp/asdfg'})
    u'file:///tmp/a.lrc'
    >>> db.find({'title': 'Tiger', })
    u'file:///tmp/a.lrc'
    >>> db.assign({'title': 'ttTiger', 'artist': 'ssSoldier', 'location': '/tmp/asdf'}, 'file:///tmp/b.lrc')
    >>> db.find({'artist': 'Soldier', })
    u'file:///tmp/b.lrc'
    >>> db.find({'title': 'Tiger', 'artist': 'Soldier', })
    u'file:///tmp/b.lrc'
    >>> db.find({dbus.String(u'title'): dbus.String(u'Tiger'), dbus.String(u'artist'): dbus.String(u'Soldier'), })
    u'file:///tmp/b.lrc'
    >>> metadata_uni = {u'title': u'\u6807\u9898', u'artist': u'\u6b4c\u624b', }
    >>> db.assign(metadata_uni, u'\u8def\u5f84')
    >>> metadata_utf8 = {'title': '\xe6\xa0\x87\xe9\xa2\x98', 'artist': '\xe6\xad\x8c\xe6\x89\x8b', }
    >>> db.find(metadata_utf8)
    u'\u8def\u5f84'
    >>> db.assign(metadata_utf8, '\xe8\xb7\xaf\xe5\xbe\x84')
    >>> db.find(metadata_uni)
    u'\u8def\u5f84'
    >>> db.find({'title': 'Tiger', 'artist': 'Soldiers', })
    >>> db.find({})
    """
    import doctest
    doctest.testmod()

if __name__ == '__main__':
    test()
