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

enum {
  PROP_0,

  PROP_XALIGN,
  PROP_YALIGN,
  PROP_FONT_SIZE,
  PROP_WIDTH,
  PROP_LOCKED,
  PROP_FONT_FAMILY,
  PROP_LRC_ALIGN_0,
  PROP_LRC_ALIGN_1,
  PROP_ACTIVE_LRC_COLOR,
  PROP_INACTIVE_LRC_COLOR,
};

static const char *OL_CONFIG_ACTIVE_LRC_COLOR[] = {
  "#662600", "#FFFF00", "#FF8000", NULL,
};
static const char *OL_CONFIG_INACTIVE_LRC_COLOR[] = {
  "#99FFFF", "#0000FF", "#99FFFF", NULL,
};

typedef struct _OlConfigStrListValue OlConfigStrListValue;
struct _OlConfigStrListValue
{
  int key;
  char *name;
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
  char *nick;
  char *description;
  gboolean default_value;
};

typedef struct _OlConfigStringValue OlConfigStringValue;
struct _OlConfigStringValue
{
  int key;
  char *name;
  char *nick;
  char *description;
  char *default_value;
};

typedef struct _OlConfigIntValue OlConfigIntValue;
struct _OlConfigIntValue
{
  int key;
  char *name;
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
  char *nick;
  char *description;
  double min;
  double max;
  double default_value;
};

static const OlConfigStrListValue config_str_list[] = {
  {PROP_ACTIVE_LRC_COLOR, "active-lrc-color", "Active lyric color",
   "Colors of active lyrics", 3, OL_CONFIG_ACTIVE_LRC_COLOR},
  {PROP_INACTIVE_LRC_COLOR, "inactive-lrc-color", "Inactive lyric color",
   "Colors of inactive lyrics", 3, OL_CONFIG_INACTIVE_LRC_COLOR},
};
static const OlConfigBoolValue config_bool[] = {
  {PROP_LOCKED, "locked", "Lock", "Whether the OSD is locked", TRUE},
};

static const OlConfigIntValue config_int[] = {
  {PROP_WIDTH, "width", "OSD Width", "The width of the OSD", 1, 10000, 1024},
};

static const OlConfigDoubleValue config_double[] = {
  {PROP_XALIGN, "xalign", "Horizontal position",
   "Horizontal position of window in desktop. "
   "0.0 is left aligned, 1.0 is right aligned",
   0.0, 1.0, 0.5},
  {PROP_YALIGN, "yalign", "Vertical position",
   "Vertical position of window in desktop. "
   "0.0 is left top, 1.0 is bottom aligned",
   0.0, 1.0, 1.0},
  {PROP_FONT_SIZE, "font-size", "OSD Font Size",
   "The font size of OSD lyrics",
   0.0, 10000.0, 30.0},
  {PROP_LRC_ALIGN_0, "lrc-align-0", "Lyric alignment 0",
   "Alignment of the first lyric line",
   0.0, 1.0, 0.0},
  {PROP_LRC_ALIGN_1, "lrc-align-1", "Lyric alignment 1",
   "Alignment of the second lyric line",
   0.0, 1.0, 1.0},
};

static const OlConfigStringValue config_str[] = {
  {PROP_FONT_FAMILY, "font-family", "OSD Font family",
   "Font family of OSD lyrics",
   "serif"},
};

#endif /* _OL_CONFIG_PROPERTY_H_ */
