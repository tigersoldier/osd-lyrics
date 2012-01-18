/**
 * @file   ol_config_test.c
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Sun Jun 21 08:26:35 2009
 * 
 * @brief  Unit test for OlConfig
 * 
 * 
 */
#include <assert.h>
#include <string.h>
#include <gtk/gtk.h>
#include "ol_config.h"

void
changed_handler (OlConfig *config, char *key, gpointer data)
{
  printf ("%s has been changed\n", key);
}

void
test_singleton ()
{
  printf ("%s\n", __FUNCTION__);
  OlConfig *config = ol_config_get_instance ();
  assert (config != NULL);
  OlConfig *config2 = ol_config_get_instance ();
  assert (config == config2);
}

void
test_default_value ()
{
  printf ("%s\n", __FUNCTION__);
  OlConfig *config = ol_config_get_instance ();
  assert (config != NULL);
  ol_config_get_int (config, "OSD", "width");
  ol_config_get_double (config, "OSD", "xalign");
  ol_config_get_double (config, "OSD", "yalign");
  ol_config_get_double (config, "OSD", "font-size");
  ol_config_get_bool (config, "OSD", "locked");
  g_free (ol_config_get_string (config, "OSD", "font-family"));
  g_strfreev (ol_config_get_str_list (config, "OSD", "active-lrc-color", NULL));
}

void
test_set_value ()
{
  const int INT_VAL = 10;
  const double DOUBLE_VAL = 0.75;
  const char* STR_VAL = "string test";
  char *colors[] = {"#123456", "#7890ab", "#cdef01", NULL};
  /* printf ("%s\n", __FUNCTION__); */
  OlConfig *config = ol_config_get_instance ();
  assert (config != NULL);
  g_signal_connect (config, "changed", G_CALLBACK (changed_handler), NULL);
  ol_config_set_int (config, "OSD", "width", INT_VAL);
  assert (ol_config_get_int (config, "OSD", "width") == INT_VAL);
  ol_config_set_double (config, "OSD", "xalign", DOUBLE_VAL);
  assert (ol_config_get_double (config, "OSD", "xalign") == DOUBLE_VAL);
  ol_config_set_string (config, "OSD", "font-family", STR_VAL);
  assert (strcmp (ol_config_get_string (config, "OSD", "font-family"), STR_VAL) == 0);
  assert (strcmp (ol_config_get_string (config, "OSD", "font-family"), STR_VAL) == 0);
  ol_config_set_bool (config, "OSD", "locked", FALSE);
  assert (ol_config_get_bool (config, "OSD", "locked") == FALSE);
  ol_config_set_bool (config, "OSD", "locked", TRUE);
  assert (ol_config_get_bool (config, "OSD", "locked") == TRUE);
  ol_config_set_str_list (config, "OSD", "active-lrc-color", (const char **)colors, 3);
  g_strfreev (ol_config_get_str_list (config, "OSD", "active-lrc-color", NULL));
  g_strfreev (ol_config_get_str_list (config, "OSD", "active-lrc-color", NULL));
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  test_singleton ();
  test_default_value ();
  test_set_value ();
  return 0;
}
