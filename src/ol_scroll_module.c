#include "ol_scroll_module.h"
#include "ol_lrc.h"

static void
ol_scroll_module_init_scroll (OlScrollModule *module)
{
  
  module->scroll = OL_SCROLL_WINDOW (ol_scroll_window_new ());
  g_object_ref_sink(module->scroll);
  if (module->scroll == NULL)
  {
    return;
  }
  gtk_widget_show(module->scroll);
}

OlScrollModule*
ol_scroll_module_new ()
{
  OlScrollModule *module = g_new (OlScrollModule, 1);
  module->scroll = NULL;
  module->lrc = NULL;
  ol_music_info_init (&module->music_info);
  ol_scroll_module_init_scroll (module);
  return module;
}

void
ol_scroll_module_destroy (OlScrollModule *module)
{
  if (module == NULL)
    return;
  if (module->scroll != NULL)
  {
    g_object_unref (module->scroll);
    module->scroll = NULL;
  }
  ol_music_info_clear (&module->music_info);
  g_free (module);
}

void
ol_scroll_module_set_music_info (OlScrollModule *module, OlMusicInfo *music_info)
{
  ol_assert (music_info != NULL); 
  ol_music_info_copy (&module->music_info, music_info);
  /*
  if (module->scroll != NULL)
  {
    ol_scroll_window_set_lyric (module->scroll, NULL);
    }*/
}

void
ol_scroll_module_set_played_time (OlScrollModule *module, int played_time)
{
  ol_log_func();
  ol_assert (module != NULL);
  if (module->lrc != NULL && module->scroll != NULL)
  {
    double percentage;
    int lyric_id;
    ol_lrc_get_lyric_by_time (module->lrc,
                              played_time,
                              module->duration,
                              NULL,
                              &percentage,
                              &lyric_id);
    const struct OlLrcItem *info = ol_lrc_get_item (module->lrc,lyric_id);
    if (lyric_id == -1)
    {
      ol_scroll_window_set_lyric(module, -1);
      return;
    }
    else
    {
      /*change to the next lyric line*/
      if (lyric_id != ol_scroll_window_get_current_lyric_id (module->scroll))
      {
        ol_scroll_window_set_lyric (module->scroll, ol_lrc_item_get_id (info));
      }
      /*set the percentage of the current lyric line*/
      else
        ol_scroll_window_set_current_percentage (module->scroll, percentage);
    }
    
  }
  else
    ol_scroll_window_set_lyric (module->scroll, -1);
}



void
ol_scroll_module_set_lrc (OlScrollModule *module, struct OlLrc *lrc_file)
{
  ol_log_func ();
  ol_assert (module != NULL);
  module->lrc = lrc_file;
  if(module->scroll == NULL)
  {
      module->scroll = OL_SCROLL_WINDOW (ol_scroll_window_new ());
  }
  if (lrc_file == NULL)
    ol_scroll_window_set_whole_lyrics(module->scroll, NULL, 0);
  else {
    /*dump the whole lyrics of a song*/
    int count = ol_lrc_item_count (lrc_file);
    GPtrArray *whole_lyrics = g_ptr_array_new_with_free_func (g_free);
    const struct OlLrcItem *info = NULL;
    int i;
    for (i = 0; i < count; i++) {
      info = ol_lrc_get_item (lrc_file, i);
      g_ptr_array_add (whole_lyrics, g_strdup (ol_lrc_item_get_lyric (info)));
    }
    ol_scroll_window_set_whole_lyrics(module->scroll, whole_lyrics, count);
    g_ptr_array_unref (whole_lyrics);
  }
}

void
ol_scroll_module_set_duration (OlScrollModule *module, int duration)
{
  if (module == NULL)
    return;
  module->duration = duration;
}
