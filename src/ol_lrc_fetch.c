#include <stdlib.h>
#include <gtk/gtk.h>
#include "ol_lrc_fetch.h"
#include "string.h"
#include "ol_lrc_fetch_sogou.h"
#include "ol_lrc_fetch_qianqian.h"

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
