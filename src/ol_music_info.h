#ifndef _OL_MUSIC_INFO_H_
#define _OL_MUSIC_INFO_H_

/**
 * defines a music's infomation structure
 */
typedef struct _OlMusicInfo OlMusicInfo;
struct _OlMusicInfo
{
  char *title;                 /* The title of the music */
  char *artist;                /* The artist of the music */
  char *album;                 /* The album name of the music */
  int track_number;            /* The track number of the music */
  char *uri;                   /* URI of the music */
};

void ol_music_info_init (OlMusicInfo *music_info);
void ol_music_info_clear (OlMusicInfo *music_info);
void ol_music_info_copy (OlMusicInfo *dest, const OlMusicInfo *src);
/** 
 * @ Free members of music_info. music_info itself will no be freed.
 * 
 * @param music_info 
 */
void ol_music_info_finalize (OlMusicInfo *music_info);
/** 
 * @ Free music_info, including its members.
 * 
 * @param music_info 
 */
void ol_music_info_destroy (OlMusicInfo *music_info);
#endif /* _OL_MUSIC_INFO_H_ */
