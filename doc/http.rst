================================================
 OSD Lyrics Player Proxy HTTP API Specification
================================================

Types
=====

Status
------

One of `playing`, `paused`, `stopped`

Capability
----------

Comma seperated string with values of `play`, `pause`, `stop`, `next`, `prev`, `seek`

Connect
=======

- path: `connect`
- parameters:

  - `name`: The name of the player.
  - `caps`: Player capabilities, described in `Capability`_.

- return values:

  - `id`: The id of the player connection.

Track Change
============
Notifies the track has been changed.

- path: `track_changed`
- parameters: the metadata of the track

  - `id`: The id returned from `connect`
  - `status`: Playing status, described in `Status`_
  - `title`: Title of the track
  - `artist`: (optional) Artist of the track
  - `album`: (optional) Album the track in.
  - `arturl`: (optional) URL of the album art
  - `tracknum`: (optional, int)
  - `length`: (int)

Status Change
=============
Notifies the playing status has been changed.

- path: `status_changed`
- parameters:

  - `id`: The id returned from `connect`
  - `status`: Playing status, described in `Status`_

- return values: nothing

Position Change
===============
Sync the position of currently playing track with the server.

- path: `position_changed`
- parameters:

  - `id`: The id returned from `connect`
  - `pos`: The position in milliseconds.

- return values: nothing

Query
=====
Gets the command sends by user.

The client should query the server in every 1 seconds.

There is a timestamp to indicate the last query time. Always use the timestamp
returned by server. The timestamp of the first query should be 0.

- path: `query`
- parameters:

  - `id`: The id returned from `connect`
  - `timestamp`: The timestamp described above

- return values:
  - `cmds`: An array of objects. Each object represents a command. The object is described below.
  - `timestamp`: New timestamp

An command object has two members: `cmd` and `params`. `params` is a object, and differs according to `cmd`. Available `cmd` are:

- `play`: no param
- `pause`: no param
- `stop`: no param
- `prev`: no param
- `next`: no param
- `seek`: `params` has a member `pos`, the position to seek in milliseconds.
