#
# Regular cron jobs for the osdlyrics package
#
0 4	* * *	root	[ -x /usr/bin/osdlyrics_maintenance ] && /usr/bin/osdlyrics_maintenance
