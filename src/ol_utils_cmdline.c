#include <sys/wait.h>           /* For WEXITSTATUS */
#include "ol_utils_cmdline.h"
#include "ol_debug.h"

gboolean
ol_cmd_get_string (const char *cmd, char **retval)
{
  /* ol_log_func (); */
  /* ol_debugf ("  cmd: %s\n", cmd); */
  ol_assert_ret (cmd != NULL, FALSE);
  int flags = 0;
  if (retval == NULL)
    flags |= G_SPAWN_STDOUT_TO_DEV_NULL;
  flags |=  G_SPAWN_STDERR_TO_DEV_NULL;
  int exit_status;
  if (!g_spawn_command_line_sync (cmd, retval, NULL, &exit_status, NULL))
    return FALSE;
  if (WEXITSTATUS(exit_status) != 0)
    return FALSE;
  return TRUE;
}

gboolean
ol_cmd_get_int (const char *cmd, int *retval)
{
  char *output;
  gboolean ret = ol_cmd_get_string (cmd, &output);
  if ((output != NULL) && (retval != NULL))
  {
    sscanf (output, "%d", retval);
  }
  if (output != NULL)
    g_free (output);
  return ret;
}

gboolean
ol_cmd_exec (const char *cmd)
{
  char *output = NULL;
  gboolean ret = ol_cmd_get_string (cmd, &output);
  if (output != NULL)
    g_free (output);
  return ret;
}
