#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "ol_lrc_fetch.h"
#include "string.h"
#include "ol_lrc_fetch_sogou.h"
#include "ol_lrc_fetch_qianqian.h"
#include "ol_utils.h"
#include "ol_debug.h"

#define OL_LRC_FETCH_ENGINE_MAX 10

static OlLrcFetchEngine *engines[OL_LRC_FETCH_ENGINE_MAX] = {0};
static int engine_count = 0;
static int ol_lrc_fetch_add_engine (OlLrcFetchEngine *engine);
static char *engine_list[OL_LRC_FETCH_ENGINE_MAX];

OlLrcCandidate*
ol_lrc_candidate_new ()
{
  OlLrcCandidate *candidate = g_new (OlLrcCandidate, 1);
  if (candidate == NULL)
    return NULL;
  candidate->title[0] = '\0';
  candidate->artist[0] = '\0';
  candidate->url[0] = '\0';
  candidate->rank = 0;
  return candidate;
}

void
ol_lrc_candidate_copy (OlLrcCandidate *dest, OlLrcCandidate *src)
{
  if (dest == NULL || src == NULL || dest == src)
    return;
  strcpy (dest->title, src->title);
  strcpy (dest->artist, src->artist);
  strcpy (dest->url, src->url);
  dest->rank = src->rank;
}

void
ol_lrc_candidate_free (OlLrcCandidate *ptr)
{
  if (ptr == NULL)
    return;
  free (ptr);
}


OlLrcFetchEngine *
ol_lrc_fetch_get_engine (const char *name)
{
  fprintf (stderr, "%s:%s\n", __FUNCTION__, name);
  if (engine_count == 0)
    return NULL;
  if (name == NULL)
    return engines[0];
  int i = 0;
  size_t len = strlen (name);
  for (i = 0; i < engine_count; i++)
  {
    fprintf (stderr, "[%d]:%s\n", i, engines[i]->name);
    if (ol_stricmp (name, engines[i]->name, len) == 0)
      return engines[i];
  }
  return engines[0];
}

void
ol_lrc_fetch_init ()
{
  ol_lrc_fetch_add_engine (ol_lrc_fetch_sogou_engine ());
  ol_lrc_fetch_add_engine (ol_lrc_fetch_qianqian_engine ());
}

static int
ol_lrc_fetch_add_engine (OlLrcFetchEngine *engine)
{
  fprintf (stderr, "%s:%s\n", __FUNCTION__, engine->name);
  if (engine_count >= OL_LRC_FETCH_ENGINE_MAX)
    return 0;
  int i;
  for (i = 0; i < engine_count; i++)
  {
    if (engines[i] == engine)
      return 0;
  }
  engine_list[engine_count] = engine->name;
  engines[engine_count++] = engine;
  return engine_count;
}

const char**
ol_lrc_fetch_get_engine_list (int *count)
{
  if (count != NULL)
    *count = engine_count;
  return (const char**) engine_list;
}

static char*
internal_strncpy (char *dest, const char *src, size_t count)
{
  ol_assert_ret (dest != NULL, NULL);
  if (src == NULL)
    dest[0] = '\0';
  else
    strncpy (dest, src, count);
  return dest;
}

void
ol_lrc_candidate_set_title (OlLrcCandidate *candidate,
                            const char *title)
{
  ol_assert (candidate != NULL);
  internal_strncpy (candidate->title, title, OL_TS_LEN_MAX);
}

char*
ol_lrc_candidate_get_title (OlLrcCandidate *candidate)
{
  ol_assert_ret (candidate != NULL, NULL);
  return candidate->title;
}

void
ol_lrc_candidate_set_artist (OlLrcCandidate *candidate,
                                  const char *artist)
{
  ol_assert (candidate != NULL);
  internal_strncpy (candidate->artist, artist, OL_TS_LEN_MAX);
}

char*
ol_lrc_candidate_get_artist (OlLrcCandidate *candidate)
{
  ol_assert_ret (candidate != NULL, NULL);
  return candidate->artist;
}

void
ol_lrc_candidate_set_url (OlLrcCandidate *candidate,
                          const char *url)
{
  ol_assert (candidate != NULL);
  internal_strncpy (candidate->url, url, OL_URL_LEN_MAX);
}

char*
ol_lrc_candidate_get_url (OlLrcCandidate *candidate)
{
  ol_assert_ret (candidate != NULL, NULL);
  return candidate->url;
}

void
ol_lrc_candidate_set_rank (OlLrcCandidate *candidate,
                           int rank)
{
  ol_assert (candidate != NULL);
  candidate->rank = rank;
}

int
ol_lrc_candidate_get_rank (OlLrcCandidate *candidate)
{
  ol_assert_ret (candidate != NULL, 0);
  return candidate->rank;
}

int 
ol_lrc_candidate_serialize (OlLrcCandidate *candidate,
                            char *buffer,
                            size_t count)
{
  ol_assert_ret (candidate != NULL, 0);
  int cnt = 0;
  if (buffer)
  {
    cnt += snprintf (buffer + cnt, count - cnt, "%s\n", candidate->title);
    cnt += snprintf (buffer + cnt, count - cnt, "%s\n", candidate->artist);
    cnt += snprintf (buffer + cnt, count - cnt, "%s\n", candidate->url);
    cnt += snprintf (buffer + cnt, count - cnt, "%d\n", candidate->rank);
    if (cnt >= count)
    {
      if (count > 0)
        buffer[count - 1] = '\0';
    }
    else
    {
      buffer[cnt] = '\0';
    }
  }
  else
  {
    cnt += strlen (candidate->title);
    cnt++;
    cnt += strlen (candidate->artist);
    cnt++;
    cnt += strlen (candidate->url);
    cnt++;
    static char buffer[20];
    cnt += snprintf (buffer, 20, "%d\n", candidate->rank);
  }
  return cnt;
}

char * 
ol_lrc_candidate_deserialize (OlLrcCandidate *candidate,
                              char *data)
{
  ol_assert_ret (candidate != NULL, 0);
  ol_assert_ret (data != NULL, 0);
  char *title, *artist, *url, *rank, *ret;
  title = artist = url = rank = ret = NULL;
  title = data;
  artist = ol_split_a_line (title);
  if (artist != NULL)
    url = ol_split_a_line (artist);
  if (url != NULL)
    rank = ol_split_a_line (url);
  if (rank != NULL)
    ret = ol_split_a_line (rank);
  if (ret != NULL)
  {
    ol_lrc_candidate_set_title (candidate, title);
    ol_lrc_candidate_set_artist (candidate, artist);
    ol_lrc_candidate_set_url (candidate, url);
    int rank_int = 0;
    sscanf (rank, "%d", &rank_int);
    ol_lrc_candidate_set_rank (candidate, rank_int);
  }
  return ret;
}

const char *
ol_lrc_fetch_engine_get_name (OlLrcFetchEngine *engine)
{
  ol_assert_ret (engine != NULL, NULL);
  return engine->name;
}
