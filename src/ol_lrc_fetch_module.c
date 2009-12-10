#include "ol_lrc_fetch_module.h"
#include "ol_debug.h"

struct DownloadContext
{
  char *filepath;
  OlLrcCandidate *candidate;
  OlLrcFetchEngine *engine;
};

static GThread *search_thread = NULL;
static GCond *search_cond = NULL;
static GMutex *search_mutex = NULL;
static GMutex *search_thread_mutex = NULL;
static int search_id = 0;
static OlLrcFetchEngine *engine = NULL;
static OlMusicInfo music_info;
static struct OlLrcFetchResult search_result;
static GPtrArray *search_listeners;
static GPtrArray *download_listeners;
static gboolean first_run = TRUE;

static gpointer ol_lrc_fetch_search_func (gpointer data);
static gpointer ol_lrc_fetch_download_func (struct DownloadContext *context);
static void ol_lrc_fetch_add_timeout (gpointer ptr,
                                      gpointer userdata);

static void
ol_lrc_fetch_add_timeout (gpointer ptr,
                          gpointer userdata)
{
  GSourceFunc callbackFunc = (GSourceFunc) ptr;
  g_timeout_add (1, callbackFunc, userdata);
}

static gpointer
ol_lrc_fetch_search_func (gpointer data)
{
  ol_log_func ();
  for (;;)
  {
    g_mutex_lock (search_mutex);
    if (first_run)
    {
      g_mutex_unlock (search_thread_mutex);
      first_run = FALSE;
    }
    g_cond_wait (search_cond, search_mutex);
    fprintf (stderr, "search request\n");
    search_result.engine = engine;
    ol_music_info_copy (&search_result.info, &music_info);
    search_result.id = search_id;
    g_mutex_unlock (search_mutex);
    
    int lrc_count;
    search_result.candidates = search_result.engine->search (&search_result.info, &search_result.count, "UTF-8");
    ol_debug ("  search done");
    g_mutex_lock (search_mutex);
    if (search_id == search_result.id)
    {
      g_ptr_array_foreach (search_listeners,
                           ol_lrc_fetch_add_timeout,
                           &search_result);
    }
    g_mutex_unlock (search_mutex);
  }
}

static gpointer
ol_lrc_fetch_download_func (struct DownloadContext *context)
{
  ol_log_func ();
  if (context == NULL)
    return;
  char *file = NULL;
  if (context->candidate != NULL &&
      context->engine != NULL &&
      context->filepath != NULL)
  {
    fprintf (stderr, "  gogogo\n");
    if (context->engine->download (context->candidate, context->filepath, "UTF-8") >= 0)
    {
      fprintf (stderr, "download %s success\n", context->filepath);
      file = context->filepath;
    }
  }
  fprintf (stderr, "path: %s\n", file);
  g_ptr_array_foreach (download_listeners,
                       ol_lrc_fetch_add_timeout,
                       file);
  if (context->candidate != NULL)
    ol_lrc_candidate_free (context->candidate);
  if (context->filepath != NULL)
    g_free (context->filepath);
  g_free (context);
  return NULL;
}

void
ol_lrc_fetch_add_async_search_callback (GSourceFunc callbackFunc)
{
  g_ptr_array_add (search_listeners, callbackFunc);
}

void
ol_lrc_fetch_add_async_download_callback (GSourceFunc callbackFunc)
{
  g_ptr_array_add (download_listeners, callbackFunc);
}

void
ol_lrc_fetch_module_init ()
{
  ol_lrc_fetch_init ();
  search_cond = g_cond_new ();
  search_mutex = g_mutex_new ();
  search_thread_mutex = g_mutex_new ();
  ol_music_info_init (&music_info);
  g_mutex_lock (search_thread_mutex);
  search_thread = g_thread_create (ol_lrc_fetch_search_func,
                                   NULL,
                                   FALSE,
                                   NULL);
  g_mutex_lock (search_thread_mutex);
  search_listeners = g_ptr_array_new ();
  download_listeners = g_ptr_array_new ();
}

int
ol_lrc_fetch_begin_search (OlLrcFetchEngine* _engine, OlMusicInfo *_music_info)
{
  g_mutex_lock (search_mutex);
  ol_log_func ();
  engine = _engine;
  ol_music_info_copy (&music_info,_music_info);
  fprintf (stderr,
           "  title: %s\n"
           "  artist: %s\n"
           "  album: %s\n",
           music_info.title,
           music_info.artist,
           music_info.album);
  search_id++;
  g_mutex_unlock (search_mutex);
  fprintf (stderr, "  send cond\n");
  g_cond_signal (search_cond);
  return search_id;
}

void
ol_lrc_fetch_begin_download (OlLrcFetchEngine *engine, OlLrcCandidate *candidate, const char *pathname)
{
  ol_log_func ();
  ol_assert (engine != NULL);
  ol_assert (candidate != NULL);
  ol_assert (pathname != NULL);
  ol_debugf ("  pathname: %s\n", pathname);
  struct DownloadContext *context = g_new (struct DownloadContext, 1);
  context->engine = engine;
  context->candidate = ol_lrc_candidate_new ();
  ol_lrc_candidate_copy (context->candidate, candidate);
  context->filepath = g_strdup (pathname);
  g_thread_create ((GThreadFunc) ol_lrc_fetch_download_func,
                   context,
                   FALSE,
                   NULL);
}

