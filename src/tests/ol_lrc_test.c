#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ol_lrc.h"
#include "ol_test_util.h"
const char *DEFAULT_URI = "test:///default/uri";

struct LineItem
{
  gint64 timestamp;
  gchar *text;
};
const struct LineItem DEFAULT_CONTENTS[] = {
  {0, "First Line"},
  {3, "Second Line"},
  {10, "Third Line"},
  {20, "Fourth Line"},
  {21, "Fifth Line"},
  {21, "Sixth Line"},
  {22, "Seventh Line"},
};

const int DEFAULT_SEEK[][2] = {
  {-1, 0},
  {0, 0},
  {2, 0},
  {3, 1},
  {5, 1},
  {11, 2},
  {19, 2},
  {20, 3},
  {21, 5},
  {22, 6},
  {100, 6},
};

const char *DEFAULT_ATTRIBUTES[][2] = {
  {"ti", "Title"},
  {"ar", "Artist"},
  {"offset", "123321"},
};

const char *NO_OFFSET_ATTRIBUTES[][2] = {
  {"ti", "Title"},
  {"ar", "Artist"},
  {"al", "Album"},
};

const int DEFAULT_OFFSET = 123321;
const int ANOTHER_OFFSET = 321123;
const char *ANOTHER_OFFSET_STR = "321123";


void
set_content (OlLrc *lrc, const struct LineItem *items, int count)
{
  GVariantBuilder *builder = g_variant_builder_new (G_VARIANT_TYPE ("aa{sv}"));
  int i;
  for (i = 0; i < count; i++)
  {
    GVariantBuilder *dict_builder = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));
    g_variant_builder_add (dict_builder,
                           "{sv}",
                           "id",
                           g_variant_new_uint32 (i));
    g_variant_builder_add (dict_builder,
                           "{sv}",
                           "timestamp",
                           g_variant_new_int64 (items[i].timestamp));
    g_variant_builder_add (dict_builder,
                           "{sv}",
                           "text",
                           g_variant_new_string (items[i].text));
    GVariant *dict = g_variant_builder_end (dict_builder);
    g_variant_builder_add_value (builder, dict);
    g_variant_builder_unref (dict_builder);
  }
  GVariant *content = g_variant_builder_end (builder);
  g_variant_builder_unref (builder);
  ol_lrc_set_content_from_variant (lrc, content);
  g_variant_unref (content);
}

void
set_attribute (OlLrc *lrc, const char *attributes[][2], int count)
{
  GVariantBuilder *builder = g_variant_builder_new (G_VARIANT_TYPE ("a{ss}"));
  int i;
  for (i = 0; i < count; i++)
  {
    g_variant_builder_add (builder, "{ss}", attributes[i][0], attributes[i][1]);
  }
  GVariant *metadata = g_variant_builder_end (builder);
  g_variant_builder_unref (builder);
  ol_lrc_set_attributes_from_variant (lrc, metadata);
  g_variant_unref (metadata);
}

void
test_init (void)
{
  OlLrc *lrc = ol_lrc_new (DEFAULT_URI);
  ol_test_expect (strcmp (ol_lrc_get_uri (lrc), DEFAULT_URI) == 0);
  ol_test_expect (ol_lrc_get_item_count (lrc) == 1);
}

void
test_content (void)
{
  OlLrc *lrc = ol_lrc_new (DEFAULT_URI);
  set_content (lrc, DEFAULT_CONTENTS, G_N_ELEMENTS (DEFAULT_CONTENTS));
  int i;
  ol_test_expect (ol_lrc_get_item_count (lrc) == G_N_ELEMENTS (DEFAULT_CONTENTS));
  OlLrcIter *iter = ol_lrc_iter_from_id (lrc, 0);
  ol_test_expect (iter != NULL);
  for (; ol_lrc_iter_is_valid (iter); i++, ol_lrc_iter_next (iter))
  {
    ol_test_expect (ol_lrc_iter_get_id (iter) == i);
    ol_test_expect (ol_lrc_iter_get_timestamp (iter) == DEFAULT_CONTENTS[i].timestamp);
    ol_test_expect (strcmp (ol_lrc_iter_get_text (iter), DEFAULT_CONTENTS[i].text) == 0);
  }
  ol_lrc_iter_free (iter);
  
  iter = ol_lrc_iter_from_id (lrc, 0);
  i = 0;
  guint id;
  gint64 timestamp;
  const char *text;
  while (ol_lrc_iter_loop (iter, &id, &timestamp, &text))
  {
    ol_test_expect (id == i);
    ol_test_expect (timestamp == DEFAULT_CONTENTS[i].timestamp);
    ol_test_expect (strcmp (text, DEFAULT_CONTENTS[i].text) == 0);
    i++;
  }
  ol_lrc_iter_free (iter);
  
  guint index = 3;
  iter = ol_lrc_iter_from_id (lrc, index);
  ol_test_expect (ol_lrc_iter_get_id (iter) == index);
  ol_lrc_iter_free (iter);
  
  ol_test_expect (ol_lrc_iter_from_id (lrc, 1000) == NULL);
  g_object_unref (lrc);
}

void
test_attribute (void)
{
  OlLrc *lrc = ol_lrc_new (DEFAULT_URI);
  set_attribute (lrc, DEFAULT_ATTRIBUTES, G_N_ELEMENTS (DEFAULT_ATTRIBUTES));
  ol_test_expect (strcmp (ol_lrc_get_attribute (lrc, "ti"), "Title") == 0);
  ol_test_expect (strcmp (ol_lrc_get_attribute (lrc, "ar"), "Artist") == 0);
  ol_test_expect (strcmp (ol_lrc_get_attribute (lrc, "offset"), "123321") == 0);
  ol_test_expect (ol_lrc_get_attribute (lrc, "al") == NULL);

  set_attribute (lrc, NO_OFFSET_ATTRIBUTES, G_N_ELEMENTS (DEFAULT_ATTRIBUTES));
  ol_test_expect (strcmp (ol_lrc_get_attribute (lrc, "ti"), "Title") == 0);
  ol_test_expect (strcmp (ol_lrc_get_attribute (lrc, "ar"), "Artist") == 0);
  ol_test_expect (strcmp (ol_lrc_get_attribute (lrc, "al"), "Album") == 0);
  ol_test_expect (ol_lrc_get_attribute (lrc, "offset") == NULL);
  g_object_unref (lrc);
}

void
test_offset (void)
{
  OlLrc *lrc = ol_lrc_new (DEFAULT_URI);
  ol_test_expect (ol_lrc_get_offset (lrc) == 0);
  ol_lrc_set_offset (lrc, ANOTHER_OFFSET);
  ol_test_expect (ol_lrc_get_offset (lrc) == ANOTHER_OFFSET);
  ol_test_expect (strcmp (ol_lrc_get_attribute (lrc, "offset"), ANOTHER_OFFSET_STR) == 0);
  set_attribute (lrc, DEFAULT_ATTRIBUTES, G_N_ELEMENTS (DEFAULT_ATTRIBUTES));
  ol_test_expect (ol_lrc_get_offset (lrc) == DEFAULT_OFFSET);
  set_attribute (lrc, NO_OFFSET_ATTRIBUTES, G_N_ELEMENTS (NO_OFFSET_ATTRIBUTES));
  ol_test_expect (ol_lrc_get_offset (lrc) == 0);
  
  ol_lrc_set_offset (lrc, DEFAULT_OFFSET);
  set_content (lrc, DEFAULT_CONTENTS, G_N_ELEMENTS (DEFAULT_CONTENTS));

  OlLrcIter *iter = ol_lrc_iter_from_id (lrc, 3);
  ol_test_expect (ol_lrc_iter_get_timestamp (iter) == DEFAULT_CONTENTS[3].timestamp - DEFAULT_OFFSET);
  ol_lrc_iter_free (iter);
  g_object_unref (lrc);
}

void
test_duration (void)
{
  OlLrc *lrc = ol_lrc_new (DEFAULT_URI);
  set_content (lrc, DEFAULT_CONTENTS, G_N_ELEMENTS (DEFAULT_CONTENTS));
  OlLrcIter *iter = ol_lrc_iter_from_id (lrc, 3);
  ol_test_expect (ol_lrc_iter_get_duration (iter) ==
                  DEFAULT_CONTENTS[4].timestamp - DEFAULT_CONTENTS[3].timestamp);
  guint last = G_N_ELEMENTS (DEFAULT_CONTENTS) - 1;
  ol_test_expect (ol_lrc_iter_move_to (iter, last) == TRUE);
  ol_test_expect (ol_lrc_iter_get_duration (iter) == 5000);
  ol_lrc_set_duration (lrc, DEFAULT_CONTENTS[last].timestamp + 123321);
  ol_test_expect (ol_lrc_iter_get_duration (iter) == 123321);
  ol_lrc_set_offset (lrc, -321);
  ol_test_expect (ol_lrc_iter_get_duration (iter) == 123000);
  ol_lrc_set_offset (lrc, -123456);
  ol_test_expect (ol_lrc_iter_get_duration (iter) == 5000);
  ol_lrc_iter_free (iter);
  g_object_unref (lrc);
}

void
test_seek (void)
{
  OlLrc *lrc = ol_lrc_new (DEFAULT_URI);
  set_content (lrc, DEFAULT_CONTENTS, G_N_ELEMENTS (DEFAULT_CONTENTS));
  int i;
  for (i = 0; i < G_N_ELEMENTS (DEFAULT_SEEK); i++)
  {
    OlLrcIter *iter = ol_lrc_iter_from_timestamp (lrc, DEFAULT_SEEK[i][0]);
    ol_test_expect (ol_lrc_iter_get_id (iter) == DEFAULT_SEEK[i][1]);
    ol_lrc_iter_free (iter);
  }
  g_object_unref (lrc);
}

int main ()
{
  g_type_init ();
  test_init ();
  test_content ();
  test_attribute ();
  test_offset ();
  test_duration ();
  test_seek ();
  return 0;
}
