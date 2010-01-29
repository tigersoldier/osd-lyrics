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

/** 
 * @brief Initialize an OlMusicInfo
 * All the fields of music_info will set to empty without memory free
 * 
 * @param music_info 
 */
void ol_music_info_init (OlMusicInfo *music_info);

/** 
 * @brief Clears an OlMusicInfo
 * All fields will be freed if they are not empty
 * 
 * @param music_info 
 */
void ol_music_info_clear (OlMusicInfo *music_info);
void ol_music_info_copy (OlMusicInfo *dest, const OlMusicInfo *src);

/** 
 * @brief Sets the value of music title
 * 
 * @param music_info An OlMusicInfo
 * @param title The value of title. If not NULL, it will be copied
 *              inside the music_info
 */
void ol_music_info_set_title (OlMusicInfo *music_info,
                              const char *title);
char *ol_music_info_get_title (OlMusicInfo *music_info);

/** 
 * @brief Sets the artist of music 
 * 
 * @param music_info An OlMusicInfo
 * @param artist The value of artist. If not NULL, it will be copied
 *               inside the music_info
 */
void ol_music_info_set_artist (OlMusicInfo *music_info,
                               const char *artist);
char *ol_music_info_get_artist (OlMusicInfo *music_info);

/** 
 * @brief Sets the name of music album
 * 
 * @param music_info An OlMusicInfo
 * @param album The name of album. If not NULL, it will be copied
 *              inside the music_info
 */
void ol_music_info_set_album (OlMusicInfo *music_info,
                              const char *album);
char *ol_music_info_get_album (OlMusicInfo *music_info);

void ol_music_info_set_track_number (OlMusicInfo *music_info,
                                     int track_number);
int ol_music_info_get_track_number (OlMusicInfo *music_info);

/** 
 * @brief Sets the location of music file
 * 
 * @param music_info An OlMusicInfo
 * @param uri The value of uri. If not NULL, it will be copied
 *            inside the music_info
 */
void ol_music_info_set_uri (OlMusicInfo *music_info,
                            const char *uri);
char *ol_music_info_get_uri (OlMusicInfo *music_info);

/** 
 * @brief Check whether two MusicInfos are equal
 * Two MusicInfos are equal if and only if all their fields are equal
 *
 * @param lhs An OlMusicInfo, or NULL
 * @param rhs An OlMusicInfo, or NULL
 * 
 * @return If lhs is equal to rhs, return 1. Otherwise return 0
 */
int ol_music_info_equal (const OlMusicInfo *lhs,
                         const OlMusicInfo *rhs);

/** 
 * @ Free music_info, including its members.
 * 
 * @param music_info 
 */
void ol_music_info_destroy (OlMusicInfo *music_info);

/** 
 * @brief Converts a music info to a string
 * The returned buffer is NUL-terminated
 * @param music_info A MusicInfo
 * @param buffer Buffer of serialzed string, or NULL.
 *               If not NULL, the serialzed string is terminated with NUL.
 * @param count The size of the buffer. 
 * 
 * @return The length of the serialized string, regardless of the size of buffer.
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
                               const char *data);
#endif /* _OL_MUSIC_INFO_H_ */
