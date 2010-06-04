#include "ol_sdm_module.h"
#include "ol_lrc.h"

static void
ol_classic_module_init_classic (OlClassicModule *module)
{
  
  module->classic = OL_CLASSIC_WINDOW (ol_classic_window_new ());
  g_object_ref_sink(module->classic);
  if (module->classic == NULL)
  {
    return;
  }
  gtk_widget_show(module->classic);
  printf ("classic module: init\n");
}

OlClassicModule*
ol_classic_module_new ()
{
  OlClassicModule *module = g_new (OlClassicModule, 1);
  module->classic = NULL;
  module->lrc = NULL;
  ol_music_info_init (&module->music_info);
  ol_classic_module_init_classic (module);
  return module;
}

void
ol_classic_module_destroy (OlClassicModule *module)
{
  if (module == NULL)
    return;
  if (module->classic != NULL)
  {
    g_object_unref (module->classic);
    module->classic = NULL;
  }
  ol_music_info_clear (&module->music_info);
  g_free (module);
}

void
ol_classic_module_set_music_info (OlClassicModule *module, OlMusicInfo *music_info)
{
  ol_assert (music_info != NULL); 
  ol_music_info_copy (&module->music_info, music_info);
  /*
  if (module->classic != NULL)
  {
    ol_classic_window_set_lyric (module->classic, NULL);
    }*/
}

void
ol_classic_module_set_played_time (OlClassicModule *module, int played_time)
{
  printf ("played Time:%d", played_time);
  ol_log_func();
  ol_assert (module != NULL);
  if (module->lrc != NULL && module->classic != NULL)
  {
    printf ("%s\n", g_ptr_array_index (module->classic->whole_lyrics, 0));
    double percentage;
    int lyric_id;
    ol_lrc_get_lyric_by_time (module->lrc,
                              played_time,
                              module->duration,
                              NULL,
                              &percentage,
                              &lyric_id);
    const struct OlLrcItem *info = ol_lrc_get_item (module->lrc,lyric_id);
    
    printf ("lyric_id:%d current_id:%d\n", lyric_id, ol_classic_window_get_current_lyric_id (module->classic));

    if (lyric_id == -1)
    {
      ol_classic_window_set_lyric(module, -1);
      return;
    }
    else
    {
      /*change to the next lyric line*/
      if (lyric_id != ol_classic_window_get_current_lyric_id (module->classic))
      {
        ol_classic_window_set_lyric (module->classic, ol_lrc_item_get_id (info));
      }
      /*set the percentage of the current lyric line*/
      else
        ol_classic_window_set_current_percentage (module->classic, percentage);
    }
    
  }
  else
    ol_classic_window_set_lyric (module->classic, -1);
}



void
ol_classic_module_set_lrc (OlClassicModule *module, struct OlLrc *lrc_file)
{
  ol_log_func ();
  ol_assert (module != NULL);
  module->lrc = lrc_file;
  if(module->classic == NULL)
  {
      module->classic = OL_CLASSIC_WINDOW (ol_classic_window_new ());
  }
  if (lrc_file == NULL)
    ol_classic_window_set_whole_lyrics(module->classic, NULL, 0);
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
    ol_classic_window_set_whole_lyrics(module->classic, whole_lyrics, count);
    g_ptr_array_unref (whole_lyrics);
  }
}

void
ol_classic_module_set_duration (OlClassicModule *module, int duration)
{
  if (module == NULL)
    return;
  module->duration = duration;
}
