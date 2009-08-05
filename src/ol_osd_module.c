#include "ol_osd_module.h"
/** 
 * @brief Gets the real lyric of the given lyric
 * A REAL lyric is the nearest lyric to the given lyric, whose text is not empty
 * If the given lyric text is not empty, the given lyric is a real lyric
 * If not real lyric available, returns NULL
 * @param lrc An LrcInfo
 * 
 * @return The real lyric of the lrc. returns NULL if not available
 */
static LrcInfo* ol_osd_module_get_real_lyric (LrcInfo *lrc);
static void ol_osd_module_update_next_lyric (OlOsdModule *module,
                                             LrcInfo *current_lrc);
static void ol_osd_module_init_osd (OlOsdModule *module);
static void config_change_handler (OlConfig *config, gchar *name, gpointer userdata);

static void
ol_osd_module_update_next_lyric (OlOsdModule *module, LrcInfo *current_lrc)
{
  LrcInfo *info = ol_lrc_parser_get_next_of_lyric (current_lrc);
  info = ol_osd_module_get_real_lyric (info);
  if (info == NULL)
  {
    if (module->lrc_next_id == -1)
    {
      return;
    }
    else
    {
      module->lrc_next_id = -1;
      ol_osd_window_set_lyric (module->osd, 1 - module->current_line, "");
    }
  }
  else
  {
    if (module->lrc_next_id == ol_lrc_parser_get_lyric_id (info))
      return;
    if (info != NULL)
    {
      module->lrc_next_id = ol_lrc_parser_get_lyric_id (info);
      ol_osd_window_set_lyric (module->osd, 1 - module->current_line,
                               ol_lrc_parser_get_lyric_text (info));
    }
  }
  ol_osd_window_set_percentage (module->osd, 1 - module->current_line, 0.0);
}

static void
config_change_handler (OlConfig *config, gchar *name, gpointer userdata)
{
  fprintf (stderr, "%s:%s\n", __FUNCTION__, name);
  OlOsdWindow *osd = OL_OSD_WINDOW (userdata);
  if (strcmp (name, "locked") == 0)
  {
    fprintf (stderr, "  locked: %d\n", ol_config_get_bool (config, "locked"));
    ol_osd_window_set_locked (osd,
                              ol_config_get_bool (config, "locked"));
  }
  else if (strcmp (name, "xalign") == 0 || strcmp (name, "yalign") == 0)
  {
    double xalign = ol_config_get_double (config, "xalign");
    double yalign = ol_config_get_double (config, "yalign");
    ol_osd_window_set_alignment (osd, xalign, yalign);
  }
  else if (strcmp (name, "font-family") == 0)
  {
    gchar *font = ol_config_get_string (config, "font-family");
    g_return_if_fail (font != NULL);
    ol_osd_window_set_font_family (osd, font);
    g_free (font);
  }
  else if (strcmp (name, "font-size") == 0)
  {
    ol_osd_window_set_font_size (osd,
                                 ol_config_get_double (config, "font-size"));
  }
  else if (strcmp (name, "width") == 0)
  {
    ol_osd_window_set_width (osd,
                             ol_config_get_int (config, "width"));
  }
  else if (strcmp (name, "lrc-align-0") == 0)
  {
    ol_osd_window_set_line_alignment (osd, 0,
                                      ol_config_get_double (config, name));
  }
  else if (strcmp (name, "lrc-align-1") == 0)
  {
    ol_osd_window_set_line_alignment (osd, 1,
                                      ol_config_get_double (config, name));
  }
  else if (strcmp (name, "active-lrc-color") == 0)
  {
    int len;
    char **color_str = ol_config_get_str_list (config, name, &len);
    printf ("len = %d\n", len);
    if (len != OL_LINEAR_COLOR_COUNT) return;
    if (color_str != NULL)
    {
      OlColor *colors = ol_color_from_str_list ((const char**)color_str, NULL);
      ol_osd_window_set_active_colors (osd, colors[0], colors[1], colors[2]);
      g_free (colors);
      g_strfreev (color_str);
    }
  }
  else if (strcmp (name, "inactive-lrc-color") == 0)
  {
    int len;
    char **color_str = ol_config_get_str_list (config, name, &len);
    printf ("len = %d\n", len);
    if (len != OL_LINEAR_COLOR_COUNT) return;
    if (color_str != NULL)
    {
      OlColor *colors = ol_color_from_str_list ((const char**)color_str, NULL);
      ol_osd_window_set_inactive_colors (osd, colors[0], colors[1], colors[2]);
      g_free (colors);
      g_strfreev (color_str);
    }
  }
}

static void
ol_osd_module_init_osd (OlOsdModule *module)
{
  module->osd = OL_OSD_WINDOW (ol_osd_window_new ());
  if (module->osd == NULL)
    return;
  /* ol_osd_window_resize (osd, 1024, 100); */
  ol_osd_window_set_alignment (module->osd, 0.5, 1);
  gtk_widget_show (GTK_WIDGET (module->osd));
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  config_change_handler (config, "xalign", module->osd);
  config_change_handler (config, "font-family", module->osd);
  config_change_handler (config, "font-size", module->osd);
  config_change_handler (config, "width", module->osd);
  config_change_handler (config, "lrc-align-0", module->osd);
  config_change_handler (config, "lrc-align-1", module->osd);
  config_change_handler (config, "active-lrc-color", module->osd);
  config_change_handler (config, "inactive-lrc-color", module->osd);
  g_signal_connect (config, "changed",
                    G_CALLBACK (config_change_handler),
                    module->osd);
}

OlOsdModule*
ol_osd_module_new ()
{
  OlOsdModule *module = g_new (OlOsdModule, 1);
  module->osd = NULL;
  module->lrc_file = NULL;
  module->lrc_id = -1;
  module->lrc_next_id = -1;
  module->current_line = 0;
  ol_music_info_init (&module->music_info);
  ol_osd_module_init_osd (module);
  return module;
}

void
ol_osd_module_destroy (OlOsdModule *module)
{
  if (module->osd != NULL)
    g_object_unref (module->osd);
  ol_music_info_finalize (&module->music_info);
  g_free (module);
}

void
ol_osd_module_set_music_info (OlOsdModule *module, OlMusicInfo *music_info)
{
  g_return_if_fail (music_info != NULL);
  ol_music_info_copy (&module->music_info, music_info);
  if (module->osd != NULL)
  {
    gtk_widget_hide (GTK_WIDGET (module->osd));
    ol_osd_window_set_lyric (module->osd, 0, NULL);
    ol_osd_window_set_lyric (module->osd, 1, NULL);
  }
  module->current_line = 0;
  module->lrc_id = -1;
  module->lrc_next_id = -1;
}

void
ol_osd_module_set_played_time (OlOsdModule *module, int played_time)
{
  /* updates the time and lyrics */
  if (module->lrc_file != NULL && module->osd != NULL)
  {
    char current_lrc[1024];
    double percentage;
    int id, lyric_id;
    ol_lrc_utility_get_lyric_by_time (module->lrc_file,
                                      played_time,
                                      module->duration,
                                      current_lrc,
                                      &percentage,
                                      &lyric_id);
    LrcInfo *info = ol_lrc_parser_get_lyric_by_id (module->lrc_file,
                                                   lyric_id);
    info = ol_osd_module_get_real_lyric (info);
    if (info == NULL)
      id = -1;
    else
      id = ol_lrc_parser_get_lyric_id (info);
    if (module->lrc_id != id)
    {
      if (id == -1)
        return;
      if (id != module->lrc_next_id)
      {
        module->current_line = 0;
        if (ol_lrc_parser_get_lyric_text (info) != NULL)
          ol_osd_window_set_lyric (module->osd, module->current_line, ol_lrc_parser_get_lyric_text (info));
        if (id != lyric_id)
          ol_osd_window_set_current_percentage (module->osd, 0.0);
        ol_osd_module_update_next_lyric (module, info);
      }
      else
      {
        ol_osd_window_set_percentage (module->osd, module->current_line, 1.0);
        module->current_line = 1 - module->current_line;
      }
      module->lrc_id = id;
      ol_osd_window_set_current_line (module->osd, module->current_line);
    }
    if (id == lyric_id && percentage > 0.5)
      ol_osd_module_update_next_lyric (module, info);
    if (id == lyric_id)
    {
      ol_osd_window_set_current_percentage (module->osd, percentage);
    }
    if (!GTK_WIDGET_MAPPED (GTK_WIDGET (module->osd)))
      gtk_widget_show (GTK_WIDGET (module->osd));
  }
  else
  {
    if (module->osd != NULL && GTK_WIDGET_MAPPED (GTK_WIDGET (module->osd)))
      gtk_widget_hide (GTK_WIDGET (module->osd));
  }
}

static LrcInfo*
ol_osd_module_get_real_lyric (LrcInfo *lrc)
{
  while (lrc != NULL)
  {
    if (!ol_is_string_empty (ol_lrc_parser_get_lyric_text (lrc)))
      break;
    lrc = ol_lrc_parser_get_next_of_lyric (lrc);
  }
  return lrc;
}

void
ol_osd_module_set_lrc (OlOsdModule *module, LrcQueue *lrc_file)
{
  module->lrc_file = lrc_file;
}

void
ol_osd_module_set_duration (OlOsdModule *module, int duration)
{
  module->duration = duration;
}
