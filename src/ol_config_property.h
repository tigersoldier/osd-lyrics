/**
 * @file   ol_config_property.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Sun Jun 21 09:34:30 2009
 * 
 * @brief  Defines properties that can be configured
 * 
 * 
 */

#ifndef _OL_CONFIG_PROPERTY_H_
#define _OL_CONFIG_PROPERTY_H_
#include "config.h"

enum {
  PROP_0,

  PROP_XALIGN,
  PROP_YALIGN,
  PROP_FONT_SIZE,
  PROP_WIDTH,
  PROP_LOCKED,
  PROP_VISIBLE,
  PROP_FONT_FAMILY,
  PROP_LRC_ALIGN_0,
  PROP_LRC_ALIGN_1,
  PROP_ACTIVE_LRC_COLOR,
  PROP_INACTIVE_LRC_COLOR,
  PROP_LINE_COUNT,
  PROP_TOTAL_COUNT,
  PROP_OSD_TRANSLUCENT_ON_MOUSE_OVER,
  PROP_DOWNLOAD_ENGINE,
  PROP_LRC_PATH,
  PROP_LRC_FILENAME,
#ifdef ENABLE_MPD
  PROP_MPD_HOSTNAME,
  PROP_MPD_PORT,
#endif
};

static const char *OL_CONFIG_ACTIVE_LRC_COLOR[] = {
  "#662600", "#FFFF00", "#FF8000", NULL,
};
static const char *OL_CONFIG_INACTIVE_LRC_COLOR[] = {
  "#99FFFF", "#0000FF", "#99FFFF", NULL,
};

static const char *OL_CONFIG_DEFAULT_LRC_PATH[] = {
  "~/.lyrics",
  "%",
  NULL,
};

static const char *OL_CONFIG_DEFAULT_LRC_FILENAME[] = {
  "%p-%t",
  "%t-%p",
  "%f",
  "%t",
  NULL,
};

typedef struct _OlConfigStrListValue OlConfigStrListValue;
struct _OlConfigStrListValue
{
  int key;
  char *name;
  char *group;
  char *nick;
  char *description;
  int len;
  const char **default_value;
};

typedef struct _OlConfigBoolValue OlConfigBoolValue;
struct _OlConfigBoolValue
{
  int key;
  char *name;
  char *group;
  char *nick;
  char *description;
  gboolean default_value;
};

typedef struct _OlConfigStringValue OlConfigStringValue;
struct _OlConfigStringValue
{
  int key;
  char *name;
  char *group;
  char *nick;
  char *description;
  char *default_value;
};

typedef struct _OlConfigIntValue OlConfigIntValue;
struct _OlConfigIntValue
{
  int key;
  char *name;
  char *group;
  char *nick;
  char *description;
  int min;
  int max;
  int default_value;
};

typedef struct _OlConfigDoubleValue OlConfigDoubleValue;
struct _OlConfigDoubleValue
{
  int key;
  char *name;
  char *group;
  char *nick;
  char *description;
  double min;
  double max;
  double default_value;
};

typedef struct _OlConfigInfo OlConfigInfo;
struct _OlConfigInfo
{
  int key;
  char *name;
  char *group;
};
static const OlConfigInfo config_info[] = {
  {PROP_0, NULL, NULL},                    /* PROP_0, */

  {PROP_XALIGN, "xalign", "OSD"},               /* PROP_XALIGN, */
  {PROP_YALIGN, "yalign", "OSD"},               /* PROP_YALIGN, */
  {PROP_FONT_SIZE, "font-size", "OSD"},            /* PROP_FONT_SIZE, */
  {PROP_WIDTH, "width", "OSD"},                /* PROP_WIDTH, */
  {PROP_LOCKED, "locked", "OSD"},               /* PROP_LOCKED, */
  {PROP_VISIBLE, "visible", "General"},          /* PROP_VISIBLE, */
  {PROP_FONT_FAMILY, "font-family", "OSD"},          /* PROP_FONT_FAMILY, */
  {PROP_LRC_ALIGN_0, "lrc-align-0", "OSD"},          /* PROP_LRC_ALIGN_0, */
  {PROP_LRC_ALIGN_1, "lrc-align-1", "OSD"},          /* PROP_LRC_ALIGN_1, */
  {PROP_ACTIVE_LRC_COLOR, "active-lrc-color", "OSD"},     /* PROP_ACTIVE_LRC_COLOR, */
  {PROP_INACTIVE_LRC_COLOR, "inactive-lrc-color", "OSD"},
  {PROP_LINE_COUNT, "line-count", "OSD"},
};

static const OlConfigStrListValue config_str_list[] = {
  {PROP_ACTIVE_LRC_COLOR, "active-lrc-color", "OSD", "Active lyric color",
   "Colors of active lyrics", 3, OL_CONFIG_ACTIVE_LRC_COLOR},
  {PROP_INACTIVE_LRC_COLOR, "inactive-lrc-color", "OSD", "Inactive lyric color",
   "Colors of inactive lyrics", 3, OL_CONFIG_INACTIVE_LRC_COLOR},
  {PROP_LRC_PATH, "lrc-path", "General", "Path of lrc files",
   "Path lists to search LRC files. Following patterns are allowed:\n"
   "  ~/<path> : Subdirectories in Home directory;\n"
   "  % : The same directory to the music;\n"
   "  Others : Treated as normal path.",
   -1, OL_CONFIG_DEFAULT_LRC_PATH},
  {PROP_LRC_FILENAME, "lrc-filename", "General", "LRC filename patterns",
   "Filename patterns to find LRC files. \n"
   "Patterns can contains following place holders:\n"
   "  %t: Title\n"
   "  %p: Artist\n"
   "  %a: Album\n"
   "  %n: Track number\n"
   "  %f: File name without extension",
   -1, OL_CONFIG_DEFAULT_LRC_FILENAME},
};
static const OlConfigBoolValue config_bool[] = {
  {PROP_LOCKED, "locked", "OSD", "Lock", "Whether the OSD is locked", TRUE},
  {PROP_VISIBLE, "visible", "General", "Visible", "Show/hide", TRUE},
  {PROP_OSD_TRANSLUCENT_ON_MOUSE_OVER, "translucent-on-mouse-over", "OSD",
   "Translucent on mouse over",
   "When the pointer is on the lyrics, make them semi-transparent",
   FALSE},
};

static const OlConfigIntValue config_int[] = {
  {PROP_WIDTH, "width", "OSD", "OSD Width", "The width of the OSD", 1, 10000, 1024},
  {PROP_LINE_COUNT, "line-count", "OSD", "OSD line count", "The number of lyric lines in OSD", 1, 2, 1},
#ifdef ENABLE_MPD
  {PROP_MPD_PORT, "mpd-port", "Player", "MPD Port", "The port of MPD service to connect", 1, 10000000, 6600},
#endif
};

static const OlConfigDoubleValue config_double[] = {
  {PROP_XALIGN, "xalign", "OSD", "Horizontal position",
   "Horizontal position of window in desktop. "
   "0.0 is left aligned, 1.0 is right aligned",
   0.0, 1.0, 0.5},
  {PROP_YALIGN, "yalign", "OSD", "Vertical position",
   "Vertical position of window in desktop. "
   "0.0 is left top, 1.0 is bottom aligned",
   0.0, 1.0, 0.85},
  {PROP_FONT_SIZE, "font-size", "OSD", "OSD Font Size",
   "The font size of OSD lyrics",
   0.0, 10000.0, 30.0},
  {PROP_LRC_ALIGN_0, "lrc-align-0", "OSD", "Lyric alignment 0",
   "Alignment of the first lyric line",
   0.0, 1.0, 0.0},
  {PROP_LRC_ALIGN_1, "lrc-align-1", "OSD", "Lyric alignment 1",
   "Alignment of the second lyric line",
   0.0, 1.0, 1.0},
};

static const OlConfigStringValue config_str[] = {
  {PROP_FONT_FAMILY, "font-family", "OSD", "OSD Font family",
   "Font family of OSD lyrics",
   "serif"},
  {PROP_DOWNLOAD_ENGINE, "download-engine", "Download",
   "Download engine", "Select the source where LRC files are downloaded from",
   "sogou"},
#ifdef ENABLE_MPD
  {PROP_MPD_HOSTNAME, "mpd_hostname", "Player",
   "Hostname of MPD", "The server to connect for MPD service",
   "localhost"},
#endif
};

#endif /* _OL_CONFIG_PROPERTY_H_ */
