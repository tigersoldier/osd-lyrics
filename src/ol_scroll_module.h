#ifndef _OL_CLASSIC_MODULE_H_
#define _OL_CLASSIC_MODULE_H_

#include "ol_music_info.h"
#include "ol_scroll_window.h"

typedef struct _OlClassicModule OlClassicModule;

struct OlLrc;

struct _OlClassicModule
{
  OlMusicInfo music_info;
  gint duration;
  struct OlLrc *lrc;
  OlClassicWindow *classic;
};

OlClassicModule* ol_classic_module_new ();
void ol_classic_module_destroy (OlClassicModule *module);
void ol_classic_module_set_music_info (OlClassicModule *module, OlMusicInfo *music_info);
void ol_classic_module_set_played_time (OlClassicModule *module, int played_time);
void ol_classic_module_set_lrc (OlClassicModule *module, struct OlLrc *lrc_file);
void ol_classic_module_set_duration (OlClassicModule *module, int duration);
/*new*/
//void ol_classic_module_set_music_info (OlClassicModule *module, OlMusicInfo *music_info);
//void ol_classic_module_set_player (OlClassicModule *module, struct OlPlayer *player);
/*
void ol_osd_module_search_message (OlClassicModule *module, const char *message);
void ol_osd_module_search_fail_message (OlClassicModule *module, const char *message);
void ol_osd_module_download_fail_message (OlClassicModule *module, const char *message);
void ol_osd_module_clear_message (OlClassicModule *module);*/

#endif
