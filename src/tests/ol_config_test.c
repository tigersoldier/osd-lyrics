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
  ol_config_get_int (config, "width");
  ol_config_get_double (config, "xalign");
  ol_config_get_double (config, "yalign");
  ol_config_get_double (config, "font-size");
  ol_config_get_bool (config, "locked");
  g_free (ol_config_get_string (config, "font-family"));
}

void
test_set_value ()
{
  const int INT_VAL = 10;
  const double DOUBLE_VAL = 0.75;
  const char* STR_VAL = "string test";
  printf ("%s\n", __FUNCTION__);
  OlConfig *config = ol_config_get_instance ();
  assert (config != NULL);
  g_signal_connect (config, "changed", G_CALLBACK (changed_handler), NULL);
  ol_config_set_int (config, "width", INT_VAL);
  assert (ol_config_get_int (config, "width") == INT_VAL);
  ol_config_set_double (config, "xalign", DOUBLE_VAL);
  assert (ol_config_get_double (config, "xalign") == DOUBLE_VAL);
  ol_config_set_string (config, "font-family", STR_VAL);
  assert (strcmp (ol_config_get_string (config, "font-family"), STR_VAL) == 0);
  assert (strcmp (ol_config_get_string (config, "font-family"), STR_VAL) == 0);
  ol_config_set_bool (config, "locked", FALSE);
  assert (ol_config_get_bool (config, "locked") == FALSE);
  ol_config_set_bool (config, "locked", TRUE);
  assert (ol_config_get_bool (config, "locked") == TRUE);
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  test_singleton ();
  test_default_value ();
  test_set_value ();
}
