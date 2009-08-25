/** 
 * @file util_lrc_fetch.h
 * @brief utility functions for fetching the lrc from Internet using libcurl
 * @author simplyzhao
 * @version 0.0.0
 * @date 2009-05-02
 */

#ifndef _UTIL_LRC_FETCH
#define _UTIL_LRC_FETCH

/* libiconv */
/* http://www.gnu.org/software/libiconv */
#include<iconv.h>

/* libcurl */
/* http://curl.haxx.se/libcurl/c */
#include<curl/curl.h>
#include<curl/types.h>
#include<curl/easy.h>

#include<limits.h>

/** 
 * @brief converts text from one encoding to another using libiconv
 * 
 * @param icv a conversion descriptor
 * @param src text to be converted
 * @param srclen at most #srclen bytes will be read
 * @param dest buffer to store the convert result
 * @param destlen at most #destlen bytes will be written
 * 
 * @return The iconv function returns the number of characters converted in a non-reversible way during this call; 
 *         reversible conversions are not counted. In case of error, it sets errno and returns (size_t)(−1). 
 */
size_t convert_icv(iconv_t *icv, char *src, size_t srclen, char *dest, size_t destlen);

/* the same to function: convert_icv */
size_t convert(const char *from_charset, const char *to_charset, char *src, size_t srclen, char *dest, size_t destlen);


typedef size_t (*WriteCallback)(void *ptr, size_t size, size_t nmemb, void *data);

struct memo {
	char *mem_base;
	size_t mem_len;
};
/** 
 * @brief fetch the content of url into memery
 *
 * @param url target Url
 * @param dest a bunch of memory identified by the struct memo
 *
 * @return 0 if success, or negative number
 */
int fetch_into_memory(const char *url, const char *refer, struct memo *dest);

/** 
 * @brief fetch the content of url into file
 * 
 * @param url target Url
 * @param fp file descriptor
 * 
 * @return 0 if success, or negative number
 */
int fetch_into_file(const char *url, const char *refer, FILE *fp);

/** 
 * @brief URL encoding;
 *        especially, 
 *        ' ' will be turned to '+';
 *        "."、"-"、"*"、"_" will keep no change 
 * 
 * @return 0 if success, or negative number
 */
int url_encoding(const char *src, const int srclen, char *dest, int destlen, int space_cat); 

/** 
 * @brief URL decoding
 */
int url_decoding(const char *src, const int srclen, char *dest, int destlen);


/** 
 * @brief URL encoding using libcurl which all input characters that are not a-z, A-Z or 0-9 
 *        are converted to their "URL escaped" version (%NN where NN is a two-digit hexadecimal number)
 * 
 * @return 0 if success, or negative number
 */
int curl_url_encoding(CURL *curl, char *input, char *output, size_t size);

/** 
 * @brief URL decoding using libcurl
 */
int curl_url_decoding(CURL *curl, char *input, char *output, size_t size);

/** 
 * @brief comparing str1 with str2 case insensitive
 * 
 * @param str1
 * @param str2
 * @param count
 * 
 * @return the same with the function: strcmp in <string.h>
 */
int ignore_case_strcmp(const char *str1, const char *str2, const size_t count);

#endif /* _UTIL_LRC_FETCH */ 
