#include <stdlib.h>
#include <glib.h>
#include "ol_lrc_fetch_module.h"
#include "ol_fork.h"
#include "ol_utils.h"
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
static GPtrArray *search_listeners;
static GPtrArray *download_listeners;

static void internal_search (struct OlLrcFetchResult *search_result);
static gpointer ol_lrc_fetch_download_func (struct DownloadContext *context);

typedef void (*FreeFunc)(gpointer userdata);

struct CallerParam {
  GPtrArray *func_array;
  gpointer userdata;
  FreeFunc data_free_func;
};

static void
internal_run_func (GSourceFunc func,
                       gpointer data)
{
  ol_log_func ();
  func (data);
}

static gboolean
ol_lrc_fetch_run_funcs (struct CallerParam *param)
{
  ol_log_func ();
  g_ptr_array_foreach (param->func_array,
                       (GFunc)internal_run_func,
                       param->userdata);
  if (param->data_free_func != NULL && param->userdata != NULL)
    param->data_free_func (param->userdata);
  g_free (param);
  return FALSE;
}

static void
ol_lrc_fetch_run_on_ui_thread (GPtrArray *func_array,
                               gpointer userdata,
                               FreeFunc data_free_func)
{
  ol_log_func ();
  struct CallerParam *new_params = g_new0 (struct CallerParam, 1);
  new_params->func_array = func_array;
  new_params->userdata = userdata;
  new_params->data_free_func = data_free_func;
  g_timeout_add (1, (GSourceFunc) ol_lrc_fetch_run_funcs, new_params);
  ol_debug ("  done\n");
}

static void
internal_invoke_callback (GPtrArray *func_array,
                 gpointer userdata)
{
  g_ptr_array_foreach (func_array,
                       (GFunc)internal_run_func,
                       userdata);
}

static void
internal_search (struct OlLrcFetchResult *search_result)
{
  ol_assert (search_result != NULL);
  int lrc_count;
  search_result->candidates = NULL;
  search_result->candidates = search_result->engine->search (&search_result->info,
                                                             &search_result->count,
                                                             "UTF-8");
  /* Output search result */
  fprintf (fret, "%d\n", search_result->count);
  int i;
  for (i = 0; i < search_result->count; i++)
  {
    int size = ol_lrc_candidate_serialize (&search_result->candidates[i], 
                                           NULL, 
                                           0);
    int buf_size = size + 10;
    char *buffer = g_new0 (char, buf_size);
    ol_lrc_candidate_serialize (&search_result->candidates[i], buffer, buf_size);
    fprintf (fret, "%s\n", buffer);
    g_free (buffer);
  }
}

static void
internal_search_callback (void *ret_data,
                          size_t ret_size,
                          int status,
                          void *userdata)
{
  ol_assert (ret_data != NULL);
  ol_assert (userdata != NULL);
  struct OlLrcFetchResult *result = (struct OlLrcFetchResult *) userdata;
  char *current = (char *) ret_data;
  char *count_str = ret_data;
  current = ol_split_a_line (current);
  ol_assert (current != NULL);
  sscanf (count_str, "%d", &result->count);
  int i;
  result->candidates = g_new0 (OlLrcCandidate, result->count);
  for (i = 0; i < result->count; i++)
  {
    current = ol_lrc_candidate_deserialize (&result->candidates[i],
                                            current);
    ol_assert (current != NULL);
    current = ol_split_a_line (current);
    ol_assert (current != NULL);
  }
  internal_invoke_callback (search_listeners, result);
  ol_lrc_fetch_result_free (result);
}

static gpointer
ol_lrc_fetch_download_func (struct DownloadContext *context)
{
  ol_log_func ();
  ol_assert_ret (context != NULL, NULL);
  char *file = NULL;
  if (context->candidate != NULL &&
      context->engine != NULL &&
      context->filepath != NULL)
  {
    if (context->engine->download (context->candidate,
                                   context->filepath,
                                   "UTF-8") >= 0)
    {
      ol_debugf ("download %s success\n", context->filepath);
      file = g_strdup (context->filepath);
    }
  }
  ol_debugf ("path: %s\n", file);
  ol_lrc_fetch_run_on_ui_thread (download_listeners, file, g_free);
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
  search_listeners = g_ptr_array_new ();
  download_listeners = g_ptr_array_new ();
}

int
ol_lrc_fetch_begin_search (OlLrcFetchEngine* _engine, OlMusicInfo *_music_info)
{
  ol_log_func ();
  ol_assert_ret (_engine != NULL, -1);
  ol_assert_ret (_music_info != NULL, -1);
  ol_debugf ("  title: %s\n"
             "  artist: %s\n"
             "  album: %s\n",
             ol_music_info_get_title (_music_info),
             ol_music_info_get_artist (_music_info),
             ol_music_info_get_album (_music_info));
  search_id++;
  struct OlLrcFetchResult *search_result = ol_lrc_fetch_result_new ();
  search_result->engine = _engine;
  ol_music_info_copy (&search_result->info, _music_info);
  search_result->id = search_id++;
  if (ol_fork (internal_search_callback,
               search_result) == 0)
  {
    internal_search (search_result);
    exit (0);
  }
  else
  {
  }
  return search_id;
}

void
ol_lrc_fetch_begin_download (OlLrcFetchEngine *engine,
                             OlLrcCandidate *candidate,
                             const char *pathname)
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

struct OlLrcFetchResult*
ol_lrc_fetch_result_new ()
{
  struct OlLrcFetchResult *ret = g_new0 (struct OlLrcFetchResult, 1);
  ol_music_info_init (&ret->info);
  return ret;
}

void
ol_lrc_fetch_result_free (struct OlLrcFetchResult *result)
{
  g_free (result->candidates);
  ol_music_info_clear (&result->info);
  g_free (result);
}
