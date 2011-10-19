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
import osdlyrics
import player
import lyrics
import dbus

logging.basicConfig(level=logging.INFO)

class MainApp(osdlyrics.App):
    def __init__(self, ):
        osdlyrics.App.__init__(self, 'Daemon', False)
        self._player = player.PlayerSupport(self.connection)
        self._lyrics = lyrics.LyricsService(self.connection)
        self._connect_metadata_signal()
        self._activate_config()
        self.request_bus_name(osdlyrics.APP_MPRIS1_NAME)

    def _connect_metadata_signal(self, ):
        self._mpris_proxy = self.connection.get_object(osdlyrics.BUS_NAME,
                                                       '/Player')
        self._metadata_signal = self._mpris_proxy.connect_to_signal('TrackChange',
                                                                    self._metadata_changed)

    def _activate_config(self, ):
        try:
            self.connection.activate_name_owner(osdlyrics.CONFIG_BUS_NAME)
        except:
            print "Cannot activate config service"
        
    def _metadata_changed(self, metadata):
        """
        
        Arguments:
        - `metadata`:
        """
        self._lyrics.set_current_metadata(metadata)

def main():
    try:
        app = MainApp()
        app.run()
    except osdlyrics.AlreadyRunningException:
        print 'OSD Lyrics is running'

if __name__ == '__main__':
    main()
