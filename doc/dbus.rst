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
``aa{sv}``

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

Player Info
--------
``a{sv}``

An dict of information of supported player application. Avaliable fields are:

 - name: string. The name used to identify the player in OSD Lyrics. It SHOULD be composed with alphabet, underscore, or digits only.
 - appname: string. The display name of the player application.
 - binname: string. The name of executable file of the player applicatoin.
 - cmd: string. The command line to launch the player.
 - icon: string. *(optional)* The name of icon of the player application. May be empty if it is a cli player.

Metadata
--------
``a{sv}``

Use the format described in `MPRIS1 http://xmms2.org/wiki/MPRIS`_, with the following fields:

 - title:
 - artist:
 - album:
 - location:
 - tracknumber:

Interfaces
==========

Player Controlling
------------------

OSD Lyrics follows `MPRIS1 http://xmms2.org/wiki/MPRIS`_ and `MPRIS2 specification <http://www.mpris.org/2.1/spec/>`_ for controlling players. 

OSD Lyrics uses the bus name ``org.mpris.MediaPlayer2.osdlyrics`` as an alias name according to the specification of MPRIS2.

Player Support
--------------

These are features about players not covered by MPRIS2.

The object path is ``/org/osdlyrics/Player``.

The interface is ``org.osdlyrics.Player``

Methods
~~~~~~~

ListSupportedPlayers() -> aa{sv}
  Gets an array of infomation of supported players.

  The returned value is an array of dicts. The fields of the dict is described in `Player Info`_.

ListActivatablePlayers() -> aa{sv}
  Similar to ``GetSupportedPlayers``, but the array contains the supported players which are installed in the computer only.

GetCurrentPlayer() -> b, a{sv}
  Gets the infomation of player currently connected to.

  If no supported player is running, the first returned value is False. Otherwise the first returned value is True, and the second value is the infomation of the player in the format described in `Player Info`_.

Signals
~~~~~~~
PlayerLost()
  Emit when the currently connected player quits.

PlayerConnected(a{sv})
  Emit when a support player is launched and connected as current player.

  player_info(a{sv}): The info of connected player. The format is described in `Player Info`_

Lyrics
------

The object path for lyrics manipulation is ``/org/osdlyrics/Lyrics``.

The interface is ``org.osdlyrics.Lyrics``.

Methods
~~~~~~~

GetLyrics(a{sv}:metadata) -> b, aa{sv}
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
  
  Returns the path of assigned lyrics. Path is a URI and follows the format described in `Lyric Path`_. If the given metadata cannot be expended to a valid path, or errors raised when saving the content to the file, an empty string is returned and the lyrics to the metadata is not changed.

AssignLyricFile(a{sv}:metadata, s:filepath) -> nothing
  Assigns an LRC file to given metadata.

Signals
~~~~~~~

CurrentLyricsChanged()
  The current lyrics is changed by ``SetLyricContent`` or ``AssignLyricFile``, or lyrics downloaded. This signal will be emitted only when the lyrics of the SAME track is changed. If the track is changed, the signal will not be emitted.

Configure
---------
The well-known bus name of configure module is ``org.osdlyrics.config``

The object path of configuration is ``/org/osdlyrics/Config``.

The interface is ``org.osdlyrics.Config``.

The name of configure options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
All the name used in configure options MUST be in the format of "group_name/options_name". For example, the visibility of OSD Window should be configured in "OSD/visible".

Methods
~~~~~~~

GetInt(s:name) -> int32
    Gets an int32 in config. If the value of name does not exists, default_value will be returned.

SetInt(s:name, int32:value)
  Sets an int32 value.

GetString(s:name) -> s
  Gets a string. If the value of name does not exists, default_value will be returned.

SetString(s:name, s:value)
  Sets a string value.

GetBool(s:name) -> b
  Gets a boolean value. If the value of name does not exists, default_value will be returned.

SetBool(s:name, b:value)
  Sets a boolean value.

GetDouble(s:name) -> d
  Gets a double value. If the value of name does not exists, default_value will be returned.

SetDouble(s:name, d:value)
  Sets a double value.

GetStringList(s:name) -> as
  Gets an array of strings. If the value of name does not exists, default_value will be returned.

SetStringList(s:name, as:value)
  Sets an array of string.

Signals
~~~~~~~

ValueChanged(as:name_list)
  Emit when one or more config value has been changed. ``name_list`` is a list of names of changed values.

Lyrics searching
----------------

TODO:


Player Proxy
============

A player proxy is a client to support one or more players.

A player proxy MUST have a unique name, like ``Mpris`` or ``Exaile03``. The well-known bus name and object path MUST be of the form ``org.osdlyrics.PlayerProxy.proxyname`` and ``/org/osdlyrics/PlayerProxy/proxyname``, where ``proxyname`` is the unique name.

For instance, a player proxy of MPRIS2 may have a unique name ``Mpris2``, and provides the bus name ``org.osdlyrics.PlayerProxy.Mpris2`` with object path ``/org/osdlyrics/PlayerProxy/Mpris2``.

The interface of player proxy is ``org.osdlyrics.PlayerProxy``

Methods
-------

ListActivePlayers() -> aa{sv}
  Lists supported players that are already running.

  Returns an array of dict. The dict represents the information of a player described in `Player Info`_.

ListActivatablePlayers() -> aa{sv}
  Lists supported players installed on the system.

  Returns an array of dict. The dict represents the information of a player described in `Player Info`_.

ListSupportedPlayers() -> aa{sv}
  Lists all supported players which can be launched on system.

ConnectPlayer(s:player_name) -> o
  Connect to an active player. The player proxy SHOULD create an dbus object with the path of ``/org/osdlyrics/PlayerProxy/proxyname/player_name``. The ``player_name`` is the ``name`` field described in `Player Info`_.

  The path of created object is returned. The created player object MUST implement interfaces described in `Player Object`_.

Signals
-------

PlayerLost(s)
  The player of name s is lost
