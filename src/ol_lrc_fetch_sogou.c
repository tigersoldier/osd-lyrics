#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include "ol_lrc_fetch.h"
#include "ol_lrc_fetch_sogou.h"
#include "ol_utils.h"
#include "ol_intl.h"
#include "ol_debug.h"

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


static OlLrcCandidate *
ol_lrc_fetch_sogou_search(const OlMusicInfo *info, int *size, const char* charset)
{
  if(info == NULL)
    return NULL;
  ol_debugf ("  title: %s\n"
             "  artist: %s\n"
             "  album: %s\n",
             info->title,
             info->artist,
             info->album);
  if (info->title == NULL && info->artist == NULL)
    return NULL;
  static OlLrcCandidate result[TRY_MATCH_MAX];
  OlLrcCandidate candidate;
  char page_url[OL_URL_LEN_MAX];
  char title_buf[BUFSIZE];
  char artist_buf[BUFSIZE];
  char buf[BUFSIZE], buf2[BUFSIZE];
  char tmpfilenam[] = "/tmp/tmplrc-XXXXXX";
  char *ptitle, *partist;
  char *ptr, *tp, *p;
  FILE *fp;
  int fd, ret, bl1, bl2, count=0;

  memset(result, 0, sizeof(result));

  if(info->title != NULL) {
    convert_to_gbk(info->title, buf, BUFSIZE, charset);
    url_encoding(buf, strlen(buf), title_buf, BUFSIZE, TRUE);
  }
  if(info->artist != NULL) {
    convert_to_gbk(info->artist, buf, BUFSIZE, charset);
    url_encoding(buf, strlen(buf), artist_buf, BUFSIZE, TRUE);
  }

  strcpy(page_url, PREFIX_PAGE_SOGOU);
  if(info->title != NULL) {
    strcat(page_url, title_buf);
    if(info->artist != NULL) {
      strcat(page_url, "-");
      strcat(page_url, artist_buf);
    }
  } else 
    strcat(page_url, artist_buf);
  if((fd = mkstemp(tmpfilenam)) < 0)
    return NULL;
  if((fp = fdopen(fd, "w+")) == NULL)
    return NULL;
  if((ret = fetch_into_file(page_url, NULL, fp)) < 0)
  {
    fclose (fp);
    remove (tmpfilenam);
    return NULL;
  }
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
      /* bl2 = info->artist==NULL? 1 : ignore_case_strcmp(artist_buf, p, strlen(artist_buf))==0; */
      url_decoding(p, strlen(p), buf2, BUFSIZE);
      convert("GBK", charset==NULL?"UTF-8":charset, buf2, strlen(buf2), candidate.artist, OL_TS_LEN_MAX);
      *(--p) = 0;
      /* title */
      /* bl1 = info->title==NULL ? 1 : ignore_case_strcmp(title_buf, tp, strlen(title_buf))==0; */
      url_decoding(tp, strlen(tp), buf2, BUFSIZE);
      convert("GBK", charset==NULL?"UTF-8":charset, buf2, strlen(buf2), candidate.title, OL_TS_LEN_MAX);
      /* restore the url */
      *p = '-';
      strcpy(candidate.url, PREFIX_LRC_SOGOU);
      strcat(candidate.url, ptr);
      count = ol_lrc_fetch_add_candidate (info, result, count, TRY_MATCH_MAX, &candidate);
    }
  }
  *size = count;
  fclose(fp);
  remove(tmpfilenam);
  return result;
}

int 
ol_lrc_fetch_sogou_download(OlLrcCandidate *tsu, const char *pathname, const char *charset)
{
  ol_log_func ();
  char *lrc_conv, *pathbuf;
  FILE *fp;
  int ret;
  struct memo lrc;
  lrc.mem_base = NULL;
  lrc.mem_len = 0;

  if((ret = fetch_into_memory(tsu->url,
                              NULL, /* refer */
                              NULL, /* user-agent */
                              NULL, /* postdata */
                              0,    /* postdata len */
                              &lrc)) < 0)
    return -1;
  lrc_conv = calloc(lrc.mem_len*2, sizeof(char));
  convert("GBK", charset==NULL ? "UTF-8" : charset, lrc.mem_base, lrc.mem_len, lrc_conv, lrc.mem_len*2);
  free(lrc.mem_base);
	
  pathbuf = ol_path_alloc();
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

static OlLrcFetchEngine sogou = {
  N_("Sogou"),
  ol_lrc_fetch_sogou_search,
  ol_lrc_fetch_sogou_download,
};

OlLrcFetchEngine *ol_lrc_fetch_sogou_engine ()
{
  return &sogou;
}
