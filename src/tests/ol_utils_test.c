#include <glib.h>
#include <glib-object.h>
#include "ol_utils.h"
#include "ol_test_util.h"
#include "ol_debug.h"

void
g_value_free (GValue *value)
{
  g_value_unset (value);
  g_free (value);
}

void
test_hashtable (void)
{
  static const int INT_VALUE = 23840;
  static const char *STR_VALUE = "String Value";
  static char *STRV_VALUE[] = {
    "Str1",
    "Str2",
    NULL,
  };
  GHashTable *ht = g_hash_table_new_full (g_str_hash, g_str_equal,
                                          g_free, (GDestroyNotify)g_value_free);
  GValue *value = g_new0 (GValue, 1);
  g_value_init (value, G_TYPE_INT);
  g_value_set_int (value, INT_VALUE);
  g_hash_table_insert (ht, g_strdup ("int"), value);

  value = g_new0 (GValue, 1);
  g_value_init (value, G_TYPE_STRING);
  g_value_set_string (value, STR_VALUE);
  g_hash_table_insert (ht, g_strdup ("str"), value);

  value = g_new0 (GValue, 1);
  g_value_init (value, G_TYPE_STRV);
  g_value_set_boxed (value, STRV_VALUE);
  g_hash_table_insert (ht, g_strdup ("strv"), value);

  ol_test_expect (ol_get_int_from_hash_table (ht, "int") == INT_VALUE);

  ol_test_expect (strcmp (ol_get_string_from_hash_table (ht, "str"), STR_VALUE) == 0);

  ol_test_expect (g_strv_length (ol_get_str_list_from_hash_table (ht, "strv")) ==
                  g_strv_length (STRV_VALUE));

  ol_test_expect (ol_get_int_from_hash_table (ht, "str") == -1);
  ol_test_expect (ol_get_int_from_hash_table (ht, "not_exists") == -1);
  
  ol_test_expect (ol_get_string_from_hash_table (ht, "int") == NULL);
  ol_test_expect (ol_get_string_from_hash_table (ht, "not_exists") == NULL);

  ol_test_expect (ol_get_str_list_from_hash_table (ht, "str") == NULL);
  ol_test_expect (ol_get_str_list_from_hash_table (ht, "not_exists") == NULL);

  g_hash_table_destroy (ht);
}

static gboolean
traverse_func (const char *path,
               const char *filename,
               gpointer data)
{
  ol_test_expect (GPOINTER_TO_INT (data) == 123);
  gchar *fullpath = g_build_path (G_DIR_SEPARATOR_S, path, filename, NULL);
  ol_test_expect (g_file_test (fullpath, G_FILE_TEST_EXISTS));
  g_free (fullpath);
  return TRUE;
}

static void
test_traverse (void)
{
  ol_traverse_dir ("/usr/bin",
                   FALSE,
                   traverse_func,
                   GINT_TO_POINTER (123));
}

int
main (int argc, char **argv)
{
  g_type_init ();
  ol_log_set_file ("-");
  test_hashtable ();
  test_traverse ();
  return 0;
}
