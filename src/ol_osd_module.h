#ifndef _OL_OSD_MODULE_H_
#define _OL_OSD_MODULE_H_

#include "ol_lrc_parser.h"
#include "ol_lrc_utility.h"
#include "ol_music_info.h"
#include "ol_config.h"
#include "ol_osd_window.h"

typedef struct _OlOsdModule OlOsdModule;

struct _OlOsdModule
{
  OlMusicInfo music_info;
  gint lrc_id;
  gint lrc_next_id;
  gint current_line;
  gint duration;
  gint line_count;
  LrcQueue *lrc_file;
  OlOsdWindow *osd;
};

OlOsdModule* ol_osd_module_new ();
void ol_osd_module_destroy (OlOsdModule *module);
void ol_osd_module_set_music_info (OlOsdModule *module, OlMusicInfo *music_info);
void ol_osd_module_set_played_time (OlOsdModule *module, int played_time);
void ol_osd_module_set_lrc (OlOsdModule *module, LrcQueue *lrc_file);
void ol_osd_module_set_duration (OlOsdModule *module, int duration);

#endif /* _OL_OSD_MODULE_H_ */
