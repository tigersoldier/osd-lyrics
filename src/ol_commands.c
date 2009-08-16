#include "ol_commands.h"
#include "ol_config.h"

void
ol_osd_lock_unlock ()
{
  OlConfig *config = ol_config_get_instance ();
  g_return_if_fail (config != NULL);
  ol_config_set_bool (config, "locked", !ol_config_get_bool (config, "locked"));
}

void
ol_show_hide ()
{
}
