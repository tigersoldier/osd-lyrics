===============================
 OSD Lyrics DBus specification
===============================

Bus Names
=========

OSD Lyrics uses ``org.osdlyrics.osdlyrics`` as well-known bus name.

Data Structures
===============

Lyrics Data
-----------
``a{sv}``

Lyrics are repesented as array of dicts. Each dict in array represent a line of lyric. dicts are in ascending order by timestamp.

Fields of lyrics dict are:

 - id: uint32. Starts from 0 and increases according to timestamp.
 - timestamp: int64. The start time of the lyric text in microseconds.
 - text: string. The lyric text itself.

Lyric Path
----------
``s``

The path of an LRC file. It is in url format. Currently available schemas are:
 - file:

Interfaces
==========

Player Control
--------------

OSD Lyrics follows `MPRIS2 specification <http://www.mpris.org/2.1/spec/>`_ for controlling players. 

OSD Lyrics uses the bus name ``org.mpris.MediaPlayer2.osdlyrics`` as an alias name according to the specification of MPRIS2.

Lyrics
------

The object path for lyrics manipulation is ``/org/osdlyrics/lyrics``.

The interface is ``org.osdlyrics.lyrics``.

Methods
~~~~~~~

GetLyrics(a{sv}:metadata) -> b, a{sv}
  Gets the lyircs of specified metadata.
  
  If lyrics found for given metadata, the first returned value is True, and the second returned value is the array of lyrics described in `Lyrics Data`_. If no lyrics found, the first value will be False and the second is an empty array.

GetCurrentLyrics() -> b, a{sv}
  Similar to GetLyrics. Returns the lyrics of current playing track.

GetRawLyrics(a{sv}:metadata) -> b, s
  Gets the content of LRC file of specified metadata. 
  
  If lyrics found, the first value will be True and the second one is the content encoded in UTF-8. If no lyrics found, the first value will be False and the second one will be an empty string.

GetCurrentRawLyrics() -> b, s
  Similar to GetRawLyrics. 
  
  Returns the content of LRC file of current playing track.

SetLyricContent(a{sv}:metadata, s:content) -> s
  Sets the lyrics of specified metadata by content of LRC file.
  
  Returns the path of assigned lyrics. Path is a URI and follows the format described in `Lyric Path`_.

AssignLyricFile(a{sv}:metadata, s:filepath) -> nothing
  Assigns an LRC file to given metadata.

Signals
~~~~~~~

CurrentLyricsChanged()
  The current lyrics is changed by ``SetLyricContent`` or ``AssignLyricFile``, or lyrics downloaded. This signal will be emitted only when the lyrics of the SAME track is changed. If the track is changed, the signal will not be emitted.

Configure
---------

The object path of configuration is ``/org/osdlyrics/config``.

The interface is ``org.osdlyrics.config``.

The name of configure options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
All the name used in configure options MUST be in the format of "group_name/options_name". For example, the visibility of OSD Window should be configured in "OSD/visible".

Methods
~~~~~~~

GetInt(s:name, int32:default_value) -> int32
    Gets an int32 in config. If the value of name does not exists, default_value will be returned.

SetInt(s:name, int32:value)
  Sets an int32 value.

GetString(s:name, s:default_value) -> s
  Gets a string. If the value of name does not exists, default_value will be returned.

SetString(s:name, s:value)
  Sets a string value.

GetBool(s:name, b:default_value) -> b
  Gets a boolean value. If the value of name does not exists, default_value will be returned.

SetBool(s:name, b:value)
  Sets a boolean value.

GetDouble(s:name, d:default_value) -> d
  Gets a double value. If the value of name does not exists, default_value will be returned.

SetDouble(s:name, d:value)
  Sets a double value.

GetStringList(s:name, as:default_value) -> as
  Gets an array of strings. If the value of name does not exists, default_value will be returned.

SetStringList(s:name, as:value)
  Sets an array of string.

Signals
~~~~~~~

ConfigChanged(s:name)
  Emit when one or more config value has been changed.

Lyrics searching
----------------

TODO:
