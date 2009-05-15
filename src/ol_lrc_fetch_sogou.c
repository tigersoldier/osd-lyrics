#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include "ol_lrc_fetch.h"

#define PREFIX_PAGE_SOGOU "http://mp3.sogou.com/gecisearch.so?query="
#define PREFIX_LRC_SOGOU "http://mp3.sogou.com/"
#define LRC_CLUE_SOGOU "downlrc"
#define TRY_MATCH_MAX 5

static int
convert_to_gbk(const char *init, char *target, size_t t_size, const char *localcharset)
{		
  int len;
  char *pt;
	
  len = strlen(init);
  if((pt = malloc(len + 1)) == NULL)
    return -1;
  strcpy(pt, init);
  pt[len] = 0;
  convert(localcharset==NULL?"UTF-8":localcharset, "GBK", pt, len, target, t_size);
  free(pt);
  return 0;
}


static struct OlLrcCandidate *
ol_lrc_fetch_sogou_search(const char *title, const char *artist, int *size, const char* charset)
{
  static struct OlLrcCandidate result[TRY_MATCH_MAX];
  char page_url[OL_URL_LEN_MAX];
  char title_buf[BUFSIZE];
  char artist_buf[BUFSIZE];
  char buf[BUFSIZE], buf2[BUFSIZE];
  char tmpfilenam[] = "tmplrc-XXXXXX";
  char *ptitle, *partist;
  char *ptr, *tp, *p;
  FILE *fp;
  int fd, ret, bl1, bl2, count=0;

  memset(result, 0, sizeof(result));
  if(title == NULL && artist == NULL)
    return NULL;

  if(title != NULL) {
    convert_to_gbk(title, buf, BUFSIZE, charset);
    url_encoding(buf, strlen(buf), title_buf, BUFSIZE);
  }
  if(artist != NULL) {
    convert_to_gbk(artist, buf, BUFSIZE, charset);
    url_encoding(buf, strlen(buf), artist_buf, BUFSIZE);
  }

  strcpy(page_url, PREFIX_PAGE_SOGOU);
  if(title != NULL) {
    strcat(page_url, title_buf);
    if(artist != NULL) {
      strcat(page_url, "-");
      strcat(page_url, artist_buf);
    }
  } else 
    strcat(page_url, artist_buf);

  if((fd = mkstemp(tmpfilenam)) < 0)
    return NULL;
  if((fp = fdopen(fd, "w+")) == NULL)
    return NULL;

  if((ret = fetch_into_file(page_url, fp)) < 0)
    return NULL;
  rewind(fp);
  while(fgets(buf, BUFSIZE, fp) != NULL && count<TRY_MATCH_MAX) {
    if((ptr = strstr(buf, LRC_CLUE_SOGOU)) != NULL) {
      tp = strchr(ptr, '"');
      if(tp != NULL)
        *tp = 0;
      tp = strrchr(ptr, '=');
      tp++;
      /* 
       * no matter Chinese or English, 
       * the title and artist are separated by "-"
       */
      p = strchr(tp, '-'); 
      /* artist */
      ++p;
      bl2 = artist==NULL? 1 : ignore_case_strcmp(artist_buf, p, strlen(artist_buf))==0;
      url_decoding(p, strlen(p), buf2, BUFSIZE);
      convert("GBK", charset==NULL?"UTF-8":charset, buf2, strlen(buf2), result[count].artist, OL_TS_LEN_MAX);
      *(--p) = 0;
      /* title */
      bl1 = title==NULL ? 1 : ignore_case_strcmp(title_buf, tp, strlen(title_buf))==0;
      url_decoding(tp, strlen(tp), buf2, BUFSIZE);
      convert("GBK", charset==NULL?"UTF-8":charset, buf2, strlen(buf2), result[count].title, OL_TS_LEN_MAX);
      /* restore the url */
      *p = '-';

      if(bl1 && bl2) {
        strcpy(result[count].url, PREFIX_LRC_SOGOU);
        strcat(result[count].url, ptr);
        count++;
      }
    }
  }
  *size = count;
  fclose(fp);
  remove(tmpfilenam);
  return result;
}

struct OlLrcCandidate *
ol_lrc_fetch_sogou_search_wrapper(const OlMusicInfo *music_info, int *size, const char *charset)
{
  return (ol_lrc_fetch_sogou_search(music_info->title,
                                    music_info->artist,
                                    size,
                                    charset));
}

int 
ol_lrc_fetch_sogou_download(struct OlLrcCandidate *tsu, const char *pathname, const char *charset)
{
  char *lrc_conv, *pathbuf;
  FILE *fp;
  int ret;
  struct memo lrc;
  lrc.mem_base = NULL;
  lrc.mem_len = 0;

  if((ret = fetch_into_memory(tsu->url, &lrc)) < 0)
    return -1;
  lrc_conv = calloc(lrc.mem_len*2, sizeof(char));
  convert("GBK", charset==NULL ? "UTF-8" : charset, lrc.mem_base, lrc.mem_len, lrc_conv, lrc.mem_len*2);
  free(lrc.mem_base);
	
  pathbuf = path_alloc();
  if(pathname == NULL)
    strcpy(pathbuf, "./");
  else
    strcpy(pathbuf, pathname);

  if((fp = fopen(pathbuf, "w")) == NULL)
    return -1;
  fputs(lrc_conv, fp);
  fclose(fp);
  free(pathbuf);
  free(lrc_conv);

  return 0;
}

struct lrc_interface sogou = {
  ol_lrc_fetch_sogou_search_wrapper,
  ol_lrc_fetch_sogou_download,
};
