#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include "ol_lrc_fetch.h"
#include "ol_lrc_fetch_qianqian.h"
#include "ol_utils.h"

#define PREFIX_PAGE_QIANQIAN "http://www.qianqian.com/lrcresult.php?qfield=1&pageflag=1&qword=%s"
#define PREFIX_FRAME_QIANQIAN "http://www.qianqian.com/lrcresult_frame.php?qword=%s&qfield=1"
#define QIANQIAN_REFER "www.qianqian.com"
#define QIANQIAN_PREFIX_FRAME "lrcresult_frame"
#define QIANQIAN_PREFIX_DOWN "downfromlrc"
#define PREFIX_LRC_QIANQIAN "http://www.qianqian.com/%s"
#define LRC_CLUE_QIANQIAN "downfromlrc"
#define TRY_MATCH_MAX 5

static const char* get_url_by_prefix (char *buf, size_t t_size, const char *html, const char *prefix);

static char* get_url_field (char *buf, size_t buflen, const char *url, const char *field);
/** 
 * @brief get candidates from lyric list page
 * 
 * @param frame_url URL of lrcresult_frame
 * @param candidates candidates to put in
 * @param count count of already fetched candidates
 * @param deep get candidates from other frame pages linked from frame_url
 * 
 * @return count of candidates after those from frame_url appended
 */
static int ol_lrc_fetch_qianqian_get_candidates (const char *title,
                                                 const char *artist,
                                                 const char *frame_url,
                                                 OlLrcCandidate *candidates,
                                                 int count,
                                                 int deep);
static int ol_lrc_fetch_qianqian_get_frame (const char *title,
                                            const char *artist,
                                            OlLrcCandidate *candidates,
                                            int count,
                                            FILE *fp,
                                            int deep);


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


static const char*
get_url_by_prefix (char *buf, size_t t_size, const char *html, const char *prefix)
{
  const char *ptr;
  const char *tp;
  if (buf == NULL)
    return NULL;
  size_t len = -1;
  if((ptr = strstr (html, prefix)) != NULL)
  {
    tp = strchr(ptr, '"');
    /* while (*tp+1 */
    if(tp != NULL)
    {
      len = tp - ptr;
    }
    if (len >= 0 && len < t_size)
      t_size = len;
    strncpy (buf, ptr, t_size);
    buf[t_size] = 0;
    return ptr + strlen (prefix);
  }
  else
  {
    return NULL;
  }
}

static char*
get_url_field (char *buf, size_t buflen, const char *url, const char *field)
{
  const char *start, *end;
  if (buf == NULL)
    return NULL;
  start = url;
  while ((start = strstr (start, field)) != NULL)
  {
    if (start[-1] == '&' || start[-1] == '?')
      break;
    start++;
  }
  /* fprintf (stderr, "field=%s ,start=%s, url=%s\n", field, start, url); */
  if (start == NULL)
    return NULL;
  start = strchr (start, '=');
  if (start == NULL)
    return NULL;
  start++;
  end = strchr (start, '&');
  if (end == NULL)
    end = start + strlen (start);
  if (buflen > end - start)
    buflen = end - start;
  strncpy (buf, start, buflen);
  buf[buflen] = '\0';
  return buf;
}

static int
ol_lrc_fetch_qianqian_get_frame (const char *title,
                                 const char *artist,
                                 OlLrcCandidate *candidates,
                                 int count,
                                 FILE *fp,
                                 int deep)
{
  char line_buf[BUFSIZE];
  char field_buf[BUFSIZE];
  char url_buf[BUFSIZE];
  char title_buf[BUFSIZE];
  char frame_url[OL_URL_LEN_MAX];
  while(fgets(line_buf, BUFSIZE, fp) != NULL /* && count<TRY_MATCH_MAX */)
  {
    /* fprintf (stderr, "%s\n", buf); */
    const char *line_pt = line_buf;
    while ((line_pt = get_url_by_prefix (url_buf,
                                         BUFSIZE - 1,
                                         line_pt,
                                         QIANQIAN_PREFIX_FRAME)) != NULL)
    {
      /* fprintf (stderr, "frame: %s\n", url_buf); */
      if (get_url_field (field_buf, BUFSIZE - 1, url_buf, "qword") != NULL)
      {
        /* fprintf (stderr, "qword %s\n", buf); */
        /* fprintf (stderr, "html: %s\n", buf); */
        /* snprintf (buf, BUFSIZE - 1, PREFIX_FRAME_QIANQIAN, buf2); */
        url_encoding (field_buf, strlen(field_buf), title_buf, BUFSIZE - 1, TRUE);
        snprintf (frame_url, OL_URL_LEN_MAX - 1, PREFIX_FRAME_QIANQIAN, title_buf);
        if (get_url_field (field_buf, BUFSIZE - 1, url_buf, "page") != NULL)
        {
          strncat (frame_url, "&page=", OL_URL_LEN_MAX - 1);
          strncat (frame_url, field_buf, OL_URL_LEN_MAX - 1);
        }
        count = ol_lrc_fetch_qianqian_get_candidates (title,
                                                      artist,
                                                      frame_url,
                                                      candidates,
                                                      count,
                                                      deep);
      }
    }
  }
  return count;
}


static int
ol_lrc_fetch_qianqian_get_candidates (const char *title,
                                      const char *artist,
                                      const char *frame_url,
                                      OlLrcCandidate *candidates,
                                      int count,
                                      int deep)
{
  if (frame_url == NULL || candidates == NULL)
    return 0;
  /* fprintf (stderr, "%s:%s\n", __FUNCTION__, frame_url); */
  FILE *fp2 = NULL;
  char frame_tmp_file[] = "/tmp/tmplrc-XXXXXX";
  char buf[BUFSIZE], buf2[BUFSIZE];
  int fd2 = mkstemp(frame_tmp_file);
  int ret;
  if (fd2 < 0)
  {
    return 0;
  }
  unlink (fd2);
  if ((fp2 = fdopen (fd2, "w+")) == NULL)
  {
    return 0;
  }
  /* fprintf (stderr, "frame url:%s->%s\n", frame_url, frame_tmp_file); */
  if ((ret = fetch_into_file (frame_url, QIANQIAN_REFER, fp2)) < 0)
  {
    fclose (fp2);
    return 0;
  }
  rewind (fp2);
  while(fgets(buf, BUFSIZE, fp2) != NULL && count<TRY_MATCH_MAX)
  {
    if (get_url_by_prefix (buf2, BUFSIZE - 1, buf, QIANQIAN_PREFIX_DOWN) != NULL)
    {
      get_url_field (buf, BUFSIZE, buf2, "title");
      url_decoding (buf, strlen (buf), candidates[count].title, OL_TS_LEN_MAX);
      /* fprintf (stderr, "title:%s\n", result[count].title); */
      get_url_field (buf, BUFSIZE, buf2, "artist");
      url_decoding (buf, strlen (buf), candidates[count].artist, OL_TS_LEN_MAX);
      /* fprintf (stderr, "artist:%s\n", result[count].artist); */
      snprintf (candidates[count].url, OL_TS_LEN_MAX, PREFIX_LRC_QIANQIAN, buf2);
      fprintf (stderr, "found: \'%s\' \'%s\'\n", candidates[count].title, candidates[count].artist);
      if ((title == NULL ||
           ignore_case_strcmp(candidates[count].title, title, strlen(title))==0) &&
          (artist == NULL ||
           ignore_case_strcmp(candidates[count].artist, artist, strlen(artist))==0))
        count++;
    }
  }
  if (deep)
  {
    rewind (fp2);
    count = ol_lrc_fetch_qianqian_get_frame (title, artist, candidates, count, fp2, 0);
  }
  fclose (fp2);
  return count;
}

static OlLrcCandidate *
ol_lrc_fetch_qianqian_search(const char *title, const char *artist, int *size, const char* charset)
{
  static OlLrcCandidate candidates[TRY_MATCH_MAX];
  int count=0;
  char page_url[OL_URL_LEN_MAX];
  char title_buf[BUFSIZE];
  char artist_buf[BUFSIZE];
  char buf[BUFSIZE], buf2[BUFSIZE];
  char tmpfilenam[] = "/tmp/tmplrc-XXXXXX";
  char *ptitle, *partist;
  char *ptr, *tp, *p;
  FILE *fp;
  int fd, ret, bl1, bl2;

  memset(candidates, 0, sizeof(candidates));
  if(title == NULL && artist == NULL)
    return NULL;

  if(title != NULL) {
    convert_to_gbk(title, buf, BUFSIZE, charset);
    url_encoding(buf, strlen(buf), title_buf, BUFSIZE, TRUE);
    /* strncpy (title_buf, title, BUFSIZE); */
  }
  /* if(artist != NULL) { */
  /*   url_encoding(artist, strlen(title), artist_buf, BUFSIZE, TRUE); */
  /* } */

  snprintf(page_url, BUFSIZE - 1, PREFIX_PAGE_QIANQIAN, title_buf);
  /* fprintf (stderr, "title:%s->%s\n", title, title_buf); */
  /* fprintf (stderr, "page url:%s\n", page_url); */
  if((fd = mkstemp(tmpfilenam)) < 0)
    return NULL;
  /* Unlink an opened fd is OK. The fd is available and the file will be  */
  /* delete after the fd is closed. See apue for detail */
  unlink (fd);
  if((fp = fdopen(fd, "w+")) == NULL)
    return NULL;
  if((ret = fetch_into_file(page_url, QIANQIAN_REFER, fp)) < 0)
  {
    fclose (fp);
    return NULL;
  }
  rewind(fp);
  count = ol_lrc_fetch_qianqian_get_frame (title, artist, candidates, count, fp, 1);
  *size = count;
  fclose(fp);
  return candidates;
}

OlLrcCandidate *
ol_lrc_fetch_qianqian_search_wrapper(const OlMusicInfo *music_info, int *size, const char *charset)
{
  return (ol_lrc_fetch_qianqian_search(music_info->title,
                                    music_info->artist,
                                    size,
                                    charset));
}

int 
ol_lrc_fetch_qianqian_download(OlLrcCandidate *tsu, const char *pathname, const char *charset)
{
  if (tsu == NULL) return;
  fprintf (stderr, "%s:%s->%s\n", __FUNCTION__, tsu->url, pathname);
  char *lrc_conv, *pathbuf;
  FILE *fp;
  int ret;
  struct memo lrc;
  lrc.mem_base = NULL;
  lrc.mem_len = 0;

  if((ret = fetch_into_memory(tsu->url, QIANQIAN_REFER, &lrc)) < 0)
    return -1;
  /* lrc_conv = calloc(lrc.mem_len*2, sizeof(char)); */
  /* convert("GBK", charset==NULL ? "UTF-8" : charset, lrc.mem_base, lrc.mem_len, lrc_conv, lrc.mem_len*2); */
	
  pathbuf = ol_path_alloc();
  if(pathname == NULL)
    strcpy(pathbuf, "./");
  else
    strcpy(pathbuf, pathname);

  if((fp = fopen(pathbuf, "w")) == NULL)
    return -1;
  fputs(lrc.mem_base, fp);
  fclose(fp);
  free(pathbuf);
  free(lrc.mem_base);
  /* free(lrc_conv); */

  return 0;
}

static OlLrcFetchEngine qianqian = {
  "Qianqian",
  ol_lrc_fetch_qianqian_search_wrapper,
  ol_lrc_fetch_qianqian_download,
};

OlLrcFetchEngine *ol_lrc_fetch_qianqian_engine ()
{
  return &qianqian;
}
