#ifndef _OL_SCROLL_MODULE_H_
#define _OL_SCROLL_MODULE_H_

#include "ol_music_info.h"
#include "ol_scroll_window.h"

typedef struct _OlScrollModule OlScrollModule;

struct OlLrc;

struct _OlScrollModule
{
  OlMusicInfo music_info;
  gint duration;
  struct OlLrc *lrc;
  OlScrollWindow *scroll;
};

OlScrollModule* ol_scroll_module_new ();
void ol_scroll_module_destroy (OlScrollModule *module);
void ol_scroll_module_set_music_info (OlScrollModule *module, OlMusicInfo *music_info);
void ol_scroll_module_set_played_time (OlScrollModule *module, int played_time);
void ol_scroll_module_set_lrc (OlScrollModule *module, struct OlLrc *lrc_file);
void ol_scroll_module_set_duration (OlScrollModule *module, int duration);
/*new*/
//void ol_scroll_module_set_music_info (OlScrollModule *module, OlMusicInfo *music_info);
//void ol_scroll_module_set_player (OlScrollModule *module, struct OlPlayer *player);
/*
void ol_osd_module_search_message (OlScrollModule *module, const char *message);
void ol_osd_module_search_fail_message (OlScrollModule *module, const char *message);
void ol_osd_module_download_fail_message (OlScrollModule *module, const char *message);
void ol_osd_module_clear_message (OlScrollModule *module);*/

#endif
