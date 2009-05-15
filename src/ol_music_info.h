#ifndef _OL_MUSIC_INFO_H_
#define _OL_MUSIC_INFO_H_

/**
 * defines a music's infomation structure
 */
typedef struct
{
  char *title;                 /* The title of the music */
  char *artist;                /* The artist of the music */
  char *album;                 /* The album name of the music */
  int track_number;            /* The track number of the music */
} OlMusicInfo;

#endif /* _OL_MUSIC_INFO_H_ */
