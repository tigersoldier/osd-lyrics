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
import os.path
import urllib
__all__ = (
    'get_config_path',
    'path2uri',
    )

def get_config_path(filename='', expanduser=True):
    """
    Gets the path to save config files

    Arguments:
    - `filename`: (optional string) The filename of config file.
    - `expanduser`: (optional bool) If the leading "~" should be expanded as user's
      home directory

    >>> get_config_path(expanduser=False)
    '~/.config/osdlyrics/'
    >>> get_config_path('osdlyrics.conf', False)
    '~/.config/osdlyrics/osdlyrics.conf'
    """
    path = os.path.join('~/.config/osdlyrics/', filename)
    if expanduser:
        path = os.path.expanduser(path)
    return path

def path2uri(path):
    r"""
    Converts a path to URI with file sheme.
    
    If a path does not start with a slash (/), it is considered to be an invalid
    path and returned directly.

    >>> path2uri('/path/to/file')
    'file:///path/to/file'
    >>> path2uri('file:///path/to/file')
    'file:///path/to/file'
    >>> path2uri(u'/path/to/file')
    'file:///path/to/file'
    >>> path2uri('invalid/path')
    'invalid/path'
    >>> path2uri('/\xe8\xb7\xaf\xe5\xbe\x84/\xe6\x96\x87\xe4\xbb\xb6')
    'file:///%E8%B7%AF%E5%BE%84/%E6%96%87%E4%BB%B6'
    """
    if path.startswith('~'):
        path = os.path.expanduser(path)
    if not path.startswith('/'):
        return path
    if isinstance(path, unicode):
        path = path.encode('utf8')
    return 'file://' + urllib.pathname2url(path)

if __name__ == '__main__':
    import doctest
    doctest.testmod()
