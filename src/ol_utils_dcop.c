#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "ol_utils_dcop.h"

enum {
  BUFFER_SIZE = 512,
};

gboolean
ol_dcop_get_string (const gchar *cmd, gchar **returnval)
{
  g_return_val_if_fail (cmd != NULL, FALSE);
  g_return_val_if_fail (returnval != NULL, FALSE);
/*   fprintf (stderr, "DCOP: %s\n", cmd); */
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
  if (*returnval == NULL)
  {
    *returnval = g_strdup (buffer);
  }
  else
  {
    strcpy (*returnval, buffer);
  }
/*   fprintf (stderr, "DCOP returns: %s\n", buffer); */
  return TRUE;
}

gboolean
ol_dcop_get_uint (const gchar *cmd, guint *returnval)
{
  g_return_val_if_fail (cmd != NULL, FALSE);
  g_return_val_if_fail (returnval != NULL, FALSE);
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
  fprintf (stderr, "%s\n", __FUNCTION__);
  g_return_val_if_fail (cmd != NULL, FALSE);
  g_return_val_if_fail (returnval != NULL, FALSE);
  gchar *ret = NULL;
  if (!ol_dcop_get_string (cmd, &ret))
    return FALSE;
  *returnval = (strcmp (ret, "true") == 0);
  printf ("returns %s\n", ret);
  g_free (ret);
  if (*returnval)
    printf ("TRUE");
  return TRUE;
}
