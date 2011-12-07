===============================
 OSD Lyrics DBus specification
===============================

Bus Names
=========

The daemon of OSD Lyrics uses ``org.osdlyrics.Daemon`` as well-known bus name.

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

Lyric URI
----------
``s``

The path of an LRC file. It is in url format. Currently available schemas are:

 - `file:` The lyrics are stored in local file system. The path of the lyrics is the path of the URI. Example: tag:///home/osdlyrics/track1.lrc
 - `tag:` The lyrics are stored in ID3 tag of the track. The path of the music file to store the ID3 tag is specified in the path of the URI. Example: tag:///home/osdlyrics/track1.ogg
 - `none:` The track is assigned not to show any lyrics. Example: none:

For compatability reasons, an empty string is considered to identical to `none:`.

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

Use the format described in `MPRIS Metadata <http://xmms2.org/wiki/MPRIS_Metadata>`_, with the following fields:

 - title: (string)
 - artist: (string)
 - album: (string)
 - location: (string)
 - tracknumber: (string)
 - arturl: (string)
 - time: (uint32) The duration in seconds 
 - mtime: (uint32) The duration in milliseconds 

Interfaces
==========

The Daemon
----------

The object path is ``/org/osdlyrics/Daemon``. The interface is ``org.osdlyrics.Daemon``.

Methods
~~~~~~~

Hello(s:client_bus_name) -> None
  Notifies that a client has connected to the daemon. The daemon watches all the bus names of clients that says hello to it. If all clients disappeared, which means the bus names of clients vanished, the daemon quits automatically.

  A valid bus name of the client should be in the format of ``org.osdlyrics.Client.ClientName``.

  It is possible to not notify the daemon. However, the daemon may quit at any time, even if the client is still running.

GetVersion() -> s
  Returns the current version of the daemon.

Quit() -> None
  Quits the daemon

Player Controlling
------------------

OSD Lyrics follows `MPRIS1 <http://xmms2.org/wiki/MPRIS>`_ and `MPRIS2 specification <http://www.mpris.org/2.1/spec/>`_ for controlling players. 

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

GetLyrics(a{sv}:metadata) -> b, s, a{ss}, aa{sv}
  Gets the lyircs of specified metadata.

  Return values:

  - ``found(b)``: Whether the lyrics file is found.
  - ``uri(s)``: The URI of the lyrics file. See `Lyrics URI`_ for more details. If no lyrics found, the uri is an empty string.
  - ``attributes(a{ss})``: The key-value attributes in the LRC file, such like [title:The title].
  - ``content(aa{sv})``: The content of the lyrics. See `Lyrics Data`_ for more details. If no lyrics found, an empty array will be returned.
  
GetCurrentLyrics() -> b, s, a{ss}, aa{sv}
  Similar to GetLyrics. Returns the lyrics of the current playing track.

GetRawLyrics(a{sv}:metadata) -> b, s, s
  Gets the content of LRC file of specified metadata. 

  Return values:

  - ``found(b)``: Whether the lyrics file is found.
  - ``uri(s)``: The URI of the lyrics file. See `Lyrics URI`_ for more details. If no lyrics found, the uri is an empty string.
  - ``content(s)``: The content of the LRC file. If no lyrics found, an empty string will be returned.
  
GetCurrentRawLyrics() -> b, s, s
  Similar to GetRawLyrics. 
  
  Returns the content of LRC file of current playing track.

SetLyricContent(a{sv}:metadata, ay:content) -> s
  Sets the lyrics of specified metadata by content of LRC file.

  The content is a byte string rather than utf-8 string so that the content can be encoded in other charsets.
  
  Returns the URI of assigned lyrics. The URI follows the format described in `Lyric URI`_. If the given metadata cannot be expended to a valid path, or errors raised when saving the content to the file, an empty string is returned and the lyrics to the metadata is not changed.

AssignLyricFile(a{sv}:metadata, s:uri) -> nothing
  Assigns an LRC file to given metadata. The ``uri`` should follow the format described in `Lyric URI`_.

SetOffset(s:uri, i:offset_ms)
  Sets the offset of an LRC file. The ``uri`` should be a valid lyrics URI described in `Lyric URI`_. The ``offset`` is in milliseconds. Errors will be raise as exceptions.

Signals
~~~~~~~

CurrentLyricsChanged()
  The current lyrics is changed by ``SetLyricContent`` or ``AssignLyricFile``, or lyrics downloaded. This signal will be emitted only when the lyrics of the SAME track is changed. If the track is changed, the signal will not be emitted.

Configure
---------
The well-known bus name of configure module is ``org.osdlyrics.Config``

The object path of configuration is ``/org/osdlyrics/Config``.

The interface is ``org.osdlyrics.config``.

The name of configure options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
All the name used in configure options MUST be in the format of "group_name/options_name". For example, the visibility of OSD Window should be configured in "OSD/visible".

Methods
~~~~~~~

GetInt(s:name) -> int32
  Gets an int32 in config. If the value of name does not exists, ``org.osdlyrics.Error.ValueNotExist`` exception will be raised. 

SetInt(s:name, int32:value)
  Sets an int32 value.

GetString(s:name) -> s
  Gets a string. If the value of name does not exists, ``org.osdlyrics.Error.ValueNotExist`` exception will be raised.

SetString(s:name, s:value)
  Sets a string value.

GetBool(s:name) -> b
  Gets a boolean value. If the value of name does not exists, ``org.osdlyrics.Error.ValueNotExist`` exception will be raised.

SetBool(s:name, b:value)
  Sets a boolean value.

GetDouble(s:name) -> d
  Gets a double value. If the value of name does not exists, ``org.osdlyrics.Error.ValueNotExist`` exception will be raised.

SetDouble(s:name, d:value)
  Sets a double value.

GetStringList(s:name) -> as
  Gets an array of strings. If the value of name does not exists, ``org.osdlyrics.Error.ValueNotExist`` exception will be raised.

SetStringList(s:name, as:value)
  Sets an array of string.

Signals
~~~~~~~

ValueChanged(as:name_list)
  Emit when one or more config value has been changed. ``name_list`` is a list of names of changed values.

Exceptions
~~~~~~~~~~

org.osdlyrics.Error.ValueNotExistError
  If the name does not exist when calling Get series methods, a ``org.osdlyrics.Error.ValueNotExist`` will be raised.

org.osdlyrics.Error.MalformedKeyError
  If the key does not follow the "group/name" format, a ``org.osdlyrics.Error.MalformedKey``  will be raised.

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
