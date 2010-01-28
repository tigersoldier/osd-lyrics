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
 * @ Free music_info, including its members.
 * 
 * @param music_info 
 */
void ol_music_info_destroy (OlMusicInfo *music_info);

/** 
 * @brief Converts a music info to a string
 *
 * @param music_info A MusicInfo
 * @param buffer The buffer to store conveted string, or NULL
 * @param count The length of the buffer
 * 
 * @return If buffer is not NULL, returns the number of bytes
 *         store in it. Otherwise returns the length of the
 *         serialized string.
 */
int ol_music_info_serialize (OlMusicInfo *music_info,
                             char *buffer,
                             size_t count);

/** 
 * @brief Converts a string to music_info
 * 
 * @param music_info A MusicInfo
 * @param data The serialized string from a MusicInfo
 * 
 * @return 1 if succeeded, or 0 if failed
 */
int ol_music_info_deserialize (OlMusicInfo *music_info,
                               char *data);
#endif /* _OL_MUSIC_INFO_H_ */
