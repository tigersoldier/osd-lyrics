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

import logging
import urlparse
import urllib
import os.path
import dbus
import dbus.service
import chardet
import osdlyrics
import osdlyrics.config
import osdlyrics.lrc
from osdlyrics.pattern import expand_file, expand_path
from osdlyrics import LYRICS_OBJECT_PATH, LYRICS_INTERFACE as INTERFACE
import lrcdb

DEFAULT_FILE_PATTERNS = [
    '%p-%t',
    '%t-%p',
    '%f',
    '%t',
    ]

DEFAULT_PATH_PATTERNS = [
    '~/.lyrics',
    '%',
    ]

def decode_non_utf8(content):
    r"""
    Detect the charset encoding of a string. If it is encoded in UTF-8, return
    the string itself. Otherwise decode the string to an unicode according to its
    encoding.

    >>> decode_non_utf8(u'\u4e2d\u6587'.encode('utf8'))
    '\xe4\xb8\xad\xe6\x96\x87'
    >>> decode_non_utf8(u'\u4e2d\u6587'.encode('gbk'))
    u'\u4e2d\u6587'
    """
    encoding = chardet.detect(content)['encoding']
    if encoding != 'utf-8':
        return content.decode(encoding)
    else:
        return content

class LyricsService(dbus.service.Object):

    scheme_handlers = {
        'file': '_lrc_from_file',
        }

    def __init__(self, conn):
        dbus.service.Object.__init__(self,
                                     conn=conn,
                                     object_path=osdlyrics.LYRICS_OBJECT_PATH)
        self._db = lrcdb.LrcDb()
        self._config = osdlyrics.config.Config(conn)
        self._metadata = {}

    def _lrc_from_file(self, urlparts):
        if urlparts.scheme == 'file':
            path = urllib.url2pathname(urlparts.path)
        else:
            path = urlparts.path
        try:
            file = open(path)
        except IOError, e:
            return False, ''
        content = decode_non_utf8(file.read())
        return True, content

    @dbus.service.method(dbus_interface=osdlyrics.LYRICS_INTERFACE,
                         in_signature='a{sv}',
                         out_signature='bsa{ss}aa{sv}')
    def GetLyrics(self, metadata):
        ret, uri, content = self.GetRawLyrics(metadata)
        if ret:
            attr, lines = osdlyrics.lrc.parse_lrc(content)
            return ret, uri, attr, lines
        else:
            return ret, uri, {}, []
    
    @dbus.service.method(dbus_interface=osdlyrics.LYRICS_INTERFACE,
                         in_signature='a{sv}',
                         out_signature='bss')
    def GetRawLyrics(self, metadata):
        path = self._db.find(metadata)
        if path is None:
            return False, '', ''
        if path == '':
            return True, 'none:', ''
        url = urlparse.urlparse(path)
        ret, lrc = False, ''
        if not url.scheme: # no scheme, consider to be a plain file path
            ret, lrc = self._lrc_from_file(url)
            path = osdlyrics.utils.path2uri(path)
        elif url.scheme in LyricsService.scheme_handlers:
            handler = getattr(self, LyricsService.scheme_handlers[url.scheme])
            ret, lrc =  handler(url)
        if not ret:  # lyrics not assigned, try to find lrc file according to patterns
            path = self._expand_patterns(metadata)
            if path is not None:
                url = urlparse.ParseResult(scheme='',
                                           netloc='',
                                           path=path,
                                           params='',
                                           query='',
                                           fragment='')
                ret, lrc = self._lrc_from_file(url)
            if ret:
                # LRC file not found in database but found according to
                # matching rules, assign the new LRC file to the lyrics
                path = osdlyrics.utils.path2uri(path)
                self._db.assign(metadata, path)
            else:
                path = ''
        return ret, path, lrc

    @dbus.service.method(dbus_interface=osdlyrics.LYRICS_INTERFACE,
                         in_signature='',
                         out_signature='bsa{ss}aa{sv}')
    def GetCurrentLyrics(self):
        return self.GetLyrics(self._metadata)

    @dbus.service.method(dbus_interface=osdlyrics.LYRICS_INTERFACE,
                         in_signature='',
                         out_signature='bss')
    def GetCurrentRawLyrics(self):
        return self.GetRawLyrics(self._metadata)

    @dbus.service.method(dbus_interface=osdlyrics.LYRICS_INTERFACE,
                         in_signature='a{sv}s',
                         out_signature='s')
    def SetLyricContent(self, metadata, content):
        path = self._expand_patterns(metadata)
        if path is None:
            return ''
        content = decode_non_utf8(content)
        if isinstance(content, unicode):
            content = content.encode('utf8')
        try:
            f = open(path, 'w')
            f.write(content)
            f.close()
        except:
            return ''
        self._db.assign(metadata, osdlyrics.utils.path2uri(path))
        return path

    @dbus.service.method(dbus_interface=osdlyrics.LYRICS_INTERFACE,
                         in_signature='a{sv}s',
                         out_signature='')
    def AssignLyricFile(self, metadata, filepath):
        self._db.assign(metadata, osdlyrics.utils.path2uri(filepath))

    def _expand_patterns(self, metadata):
        file_patterns = self._config.get_string_list('General/lrc-filename',
                                                     DEFAULT_FILE_PATTERNS)
        path_patterns = self._config.get_string_list('General/lrc-path',
                                                     DEFAULT_PATH_PATTERNS)
        for path_pat in path_patterns:
            try:
                path = expand_path(path_pat, metadata)
            except:
                continue
            for file_pat in file_patterns:
                try:
                    filename = expand_file(file_pat, metadata)
                    fullpath = os.path.join(path, filename + '.lrc')
                    if os.path.isfile(fullpath):
                        return fullpath
                except osdlyrics.pattern.PatternException, e:
                    pass
        return None

    def set_current_metadata(self, metadata):
        logging.debug('Setting current metadata: %s' % metadata)
        self._metadata = metadata

def doc_test():
    import doctest
    doctest.testmod()

def test():
    app = osdlyrics.App('Lyrics', False)
    lyrics_service = LyricsService(app.connection)
    app.run()

if __name__ == '__main__':
    doc_test()
    test()
