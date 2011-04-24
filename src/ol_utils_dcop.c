#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "ol_utils_dcop.h"
#include "ol_debug.h"

enum {
  BUFFER_SIZE = 512,
};

gboolean
ol_dcop_get_string (const gchar *cmd, gchar **returnval)
{
  ol_assert_ret (cmd != NULL, FALSE);
  ol_assert_ret (returnval != NULL, FALSE);
  FILE *pPipe = popen (cmd, "r");
  if (!pPipe)
    return FALSE;
  gchar buffer[BUFFER_SIZE] = "";
  if (!fgets (buffer, BUFFER_SIZE, pPipe)) {
    pclose(pPipe);
    return FALSE;
  }
  pclose (pPipe);
  strtok (buffer,"\n");
  if (*returnval != NULL)
  {
    g_free (*returnval);
  }
  *returnval = g_strdup (buffer);
  return TRUE;
}

gboolean
ol_dcop_get_uint (const gchar *cmd, guint *returnval)
{
  ol_assert_ret (cmd != NULL, FALSE);
  ol_assert_ret (returnval != NULL, FALSE);
  gchar *ret = NULL;
  if (!ol_dcop_get_string (cmd, &ret))
    return FALSE;
  *returnval = atoi (ret);
  g_free (ret);
  return TRUE;
}

gboolean
ol_dcop_get_boolean (const gchar *cmd, gboolean *returnval)
{
  ol_log_func ();
  ol_assert_ret (cmd != NULL, FALSE);
  ol_assert_ret (returnval != NULL, FALSE);
  gchar *ret = NULL;
  if (!ol_dcop_get_string (cmd, &ret))
    return FALSE;
  *returnval = (strcmp (ret, "true") == 0);
  ol_debugf ("returns %s\n", ret);
  g_free (ret);
  return TRUE;
}
