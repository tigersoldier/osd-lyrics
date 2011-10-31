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

import re
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
from osdlyrics.exceptions import Error
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

SUPPORTED_SCHEMES = [
    'file',
#    'tag',
    'none',
    ]

class InvalidUriException(Error):
    """ Exception of invalid uri.
    """
    
    def __init__(self, uri):
        Error.__init__(self, "Invalid URI: %s" % uri)

class CannotLoadLrcException(Error):
    def __init__(self, uri):
        Error.__init__(self, "Cannot load lrc file from %s" % uri)

class CannotSaveLrcException(Error):
    def __init__(self, uri):
        Error.__init__(self, "Cannot save lrc file to %s" % uri)

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

def metadata_equal(lhs, rhs):
    """
    Tell if two metadatas are equal.

    Two metadatas are equal if:
    - The locations are not empty and are equal, or
    - The titles, artists and albums are equal.
    """
    try:
        if lhs['location'] == rhs['location'] and lhs['location'] != '':
            return True
    except:
        pass
    keys = ['title', 'artist', 'album']
    for key in keys:
        if (key in lhs) != (key in rhs):
            return False
        if key in lhs and lhs[key] != rhs[key]:
            return False
    return True

def is_valid_uri(uri):
    """
    Tell if a URI is valid.

    A valid URI must begin with the schemes defined in SUPPORTED_SCHEMES
    """
    for scheme in SUPPORTED_SCHEMES:
        if uri.startswith(scheme + ':'):
            return True
    return False

def ensure_uri_scheme(uri):
    """
    Converts a file path to an URI with scheme of "file:", leaving other URI not
    changed

    If the uri doesn't have any scheme, it is considered to be a file path.
    """
    if uri:
        url_parts = urlparse.urlparse(uri)
        if not url_parts.scheme:
            uri = osdlyrics.utils.path2uri(uri)
    return uri

def load_from_file(urlparts):
    """
    Load the content of file from urlparse.ParseResult

    Return the content of the file, or None if error raised.
    """
    if urlparts.scheme == 'file':
        path = urllib.url2pathname(urlparts.path)
    else:
        path = urlparts.path
    try:
        file = open(path)
    except IOError, e:
        logging.warning("Cannot read from file %s: %s" % (path, e))
        return None
    content = decode_non_utf8(file.read())
    return content

def load_from_uri(uri):
    """
    Load the content of LRC file from given URI
    
    If loaded, return the content. If failed, return None.
    """
    URI_LOAD_HANDLERS = {
        'file': load_from_file,
        'none': lambda uri: '',
        }

    url_parts = urlparse.urlparse(osdlyrics.utils.ensure_utf8(uri))
    return URI_LOAD_HANDLERS[url_parts.scheme](url_parts)

def save_to_file(urlparts, content):
    """
    Save the content of file to urlparse.ParseResult

    Return True if succeeded
    """
    if urlparts.scheme == 'file':
        path = urllib.url2pathname(urlparts.path)
    else:
        path = urlparts.path
    try:
        file = open(path, 'w')
    except IOError, e:
        logging.warning("Cannot write to file %s: %s" % (path, e))
        return False
    file.write(osdlyrics.utils.ensure_utf8(content))
    return True

def save_to_uri(uri, content):
    """
    Save the content of LRC file to given URI.

    Return True if succeeded, or False if failed.
    """
    URI_SAVE_HANDLERS = {
        'file': save_to_file,
        }

    url_parts = urlparse.urlparse(osdlyrics.utils.ensure_utf8(uri))
    return URI_SAVE_HANDLERS[url_parts.scheme](url_parts, content)

def update_lrc_offset(content, offset):
    r"""
    Replace the offset attributes in the content of LRC file.
    >>> update_lrc_offset('no tag', 100)
    '[offset:100]\nno tag'
    >>> update_lrc_offset('[ti:title]\n[offset:200]\nSome lrc', 100)
    '[ti:title]\n[offset:100]\nSome lrc'
    >>> update_lrc_offset('[ti:title][offset:200]Some lrc\nanother', 100)
    '[ti:title][offset:100]Some lrc\nanother'
    >>> update_lrc_offset('Some [offset:200] lrc', 100)
    '[offset:100]\nSome [offset:200] lrc'
    >>> update_lrc_offset('[[offset:200]] lrc', 100)
    '[offset:100]\n[[offset:200]] lrc'
    """
    search_result = re.search(r'^(\[[^\]]*\])*?\[offset:(.*?)\]', content, re.MULTILINE)
    if search_result is None:
        return '[offset:%s]\n%s' % (offset, content)
    return '%s%s%s' % (content[:search_result.start(2)],
                       offset,
                       content[search_result.end(2):])

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

    def find_lrc_from_db(self, metadata):
        uri = self._db.find(metadata)
        if uri == '':
            return 'none:'
        return ensure_uri_scheme(uri)

    def find_lrc_by_pattern(self, metadata):
        return ensure_uri_scheme(self._expand_patterns(metadata))
        
    def assign_lrc_uri(self, metadata, uri):
        self._db.assign(metadata, uri)
        if metadata_equal(metadata, self._metadata):
            self.CurrentLyricsChanged()

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
        uri = self.find_lrc_from_db(metadata)
        lrc = None
        if uri:
            if uri == 'none:':
                return True, uri, ''
            lrc = load_from_uri(uri)
            if lrc is not None:
                return True, uri, lrc
        uri = self.find_lrc_by_pattern(metadata)
        if uri:
            lrc = load_from_uri(uri)
            if lrc is not None:
                self.assign_lrc_uri(metadata, uri)
        if lrc is None:
            return False, '', ''
        else:
            return True, uri, lrc

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
        uri = osdlyrics.utils.path2uri(path)
        self.assign_lrc_uri(metadata, uri)
        return uri

    @dbus.service.method(dbus_interface=osdlyrics.LYRICS_INTERFACE,
                         in_signature='a{sv}s',
                         out_signature='')
    def AssignLyricFile(self, metadata, filepath):
        self.assign_lrc_uri(metadata, osdlyrics.utils.path2uri(filepath))

    @dbus.service.signal(dbus_interface=osdlyrics.LYRICS_INTERFACE,
                         signature='')
    def CurrentLyricsChanged(self):
        pass

    @dbus.service.method(dbus_interface=osdlyrics.LYRICS_INTERFACE,
                         in_signature='si',
                         out_signature='')
    def SetOffset(self, uri, offset_ms):
        if not is_valid_uri(uri):
            raise InvalidUriException(uri)
        content = load_from_uri(uri)
        if content is None:
            raise CannotLoadLrcException(uri)
        content = update_lrc_offset(content, offset_ms)
        if not save_to_uri(uri, content):
            raise CannotSaveLrcException(uri)

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
