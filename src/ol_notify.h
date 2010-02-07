#ifndef _OL_NOTIFY_H_
#define _OL_NOTIFY_H_

#include "ol_music_info.h"

void ol_notify_init (void);
void ol_notify_music_change (OlMusicInfo *info);
void ol_notify_unload (void);

#endif /* _OL_NOTIFY_H_ */
