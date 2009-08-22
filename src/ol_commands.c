#include "ol_commands.h"
#include "ol_config.h"

void
ol_osd_lock_unlock ()
{
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  ol_config_set_bool (config, "OSD",  "locked", !ol_config_get_bool (config, "OSD", "locked"));
}

void
ol_show_hide ()
{
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  ol_config_set_bool (config, "General", "visible", !ol_config_get_bool (config, "General", "visible"));
}
