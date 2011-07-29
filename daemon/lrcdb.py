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

__all__ = (
    'LrcDb',
    )

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
            dbfile = os.path.expanduser('~/.config/%s/%s/lrc.db')
        self._dbfile = dbfile
        self._conn = sqlite3.connect(dbfile)
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
        c = self._conn.cursor()
        location = self._normalize_location(metadata.setdefault('location', ''))
        if self._find_by_location(metadata):
            c.execute(LrcDb.UPDATE_LYRIC, (uri, location,))
        else:
            title = metadata.setdefault('title', '')
            artist = metadata.setdefault('artist', '')
            album = metadata.setdefault('album', '')
            try:
                tracknum = int(metadata['tracknumber'])
            except:
                tracknum = 0
            c.execute(LrcDb.ASSIGN_LYRIC, (title, artist, album, tracknum, location, uri))
        self._conn.commit()
        c.close()

    def _normalize_location(self, location):
        """
        Normalize location of metadata to URI form
        """
        if location and location[0] == '/':
            location = 'file://' + urllib.pathname2url(location)
        return location

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
        location = self._normalize_location(location)
        return self._find_by_condition(LrcDb.QUERY_LOCATION, (location,))

    def _find_by_info(self, metadata):
        query = []
        for mkey, qkey in LrcDb.QUERY_INFO.items():
            value = metadata.setdefault(mkey, None)
            if value:
                query.append('%s=:%s' % (qkey, mkey))
        if len(query) > 0:
            return self._find_by_condition(' AND '.join(query),
                                           metadata)
        return None

def test():
    """
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
    >>> db.find({'title': 'Tiger', 'artist': 'Soldiers', })
    >>> db.find({})
    """
    import doctest
    doctest.testmod()

if __name__ == '__main__':
    test()
