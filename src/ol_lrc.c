#include <stdio.h>
#include <glib.h>
#include "ol_lrc.h"
#include "ol_lrc_parser.h"
#include "ol_debug.h"

struct OlLrcItem
{
  int id;
  int timestamp;
  int lyric_id;                 /**< The index in lyrics array */
  struct OlLrc *lrc;
};

struct OlLrc
{
  GPtrArray *items;
  int nitems;
  GPtrArray *lyrics;
  GHashTable *attrs;
  int nlyrics;
  int offset;
  char *filename;
};

static void _init_lrc (struct OlLrc *lrc);
static void _add_lyric (struct OlLrc *lrc, const char *lyric);
static void _add_time (struct OlLrc *lrc, struct OlLrcTimeToken *token);
static void _add_attr (struct OlLrc *lrc, struct OlLrcAttrToken *token);
static int _cmp_item (const struct OlLrcItem **lhs,
                      const struct OlLrcItem **rhs);
static void _set_item_id (struct OlLrcItem *item,
                          int *id);
/** 
 * @brief Save to an LRC file
 * 
 * @param lrc
 * @param filename
 * 
 * @return 0 if success. Negative if failed.
 */
static int _save (struct OlLrc *lrc, const char *filename);
static void _save_attr (char *key, char *value, FILE *file);
static void _save_lyric (struct OlLrcItem *item, FILE *file);

static void
_set_item_id (struct OlLrcItem *item,
              int *id)
{
  item->id = *id;
  (*id)++;
}

static int
_cmp_item (const struct OlLrcItem **lhs,
           const struct OlLrcItem **rhs)
{
  return (*lhs)->timestamp - (*rhs)->timestamp;
}

static void
_init_lrc (struct OlLrc *lrc)
{
  ol_assert (lrc != NULL);
  lrc->items = g_ptr_array_new_with_free_func (g_free);
  lrc->nitems = 0;
  lrc->lyrics = g_ptr_array_new_with_free_func (g_free);
  lrc->nlyrics = 0;
  lrc->attrs = g_hash_table_new_full (g_str_hash, g_str_equal,
                                      g_free, g_free);
  lrc->offset = 0;
  lrc->filename = NULL;
}

static void
_add_lyric (struct OlLrc *lrc, const char *lyric)
{
  ol_log_func ();
  ol_debugf ("lyric: %s\n", lyric);
  ol_assert (lrc != NULL);
  ol_assert (lyric != NULL);
  g_ptr_array_add (lrc->lyrics, g_strdup (lyric));
  lrc->nlyrics++;
}

static void
_add_time (struct OlLrc *lrc, struct OlLrcTimeToken *token)
{
  ol_log_func ();
  ol_assert (lrc != NULL);
  ol_assert (token != NULL);
  struct OlLrcItem *item = g_new (struct OlLrcItem, 1);
  item->lyric_id = lrc->nlyrics;
  item->timestamp = token->time;
  ol_debugf ("time: %d\n", token->time);
  ol_debugf ("cnt: %d\n", lrc->nitems);
  item->lrc = lrc;
  g_ptr_array_add (lrc->items, item);
  lrc->nitems++;
  ol_assert (token->time - lrc->offset == ol_lrc_item_get_time (ol_lrc_get_item (lrc, lrc->nitems - 1)));
}

static void
_add_attr (struct OlLrc *lrc, struct OlLrcAttrToken *token)
{
  ol_assert (lrc != NULL);
  ol_assert (token != NULL);
  if (strcmp (token->attr, "offset") == 0 && token->value != NULL)
  {
    sscanf (token->value, "%d", &lrc->offset);
  }
  g_hash_table_insert (lrc->attrs, g_strdup (token->attr), g_strdup (token->value));
}

struct OlLrc *
ol_lrc_new (const char *filename)
{
  ol_log_func ();
  struct OlLrcParser *parser = ol_lrc_parser_new_from_file (filename);
  if (parser == NULL)
    return NULL;
  struct OlLrc *lrc = g_new (struct OlLrc, 1);
  _init_lrc (lrc);
  union OlLrcToken *token = NULL;
  gboolean hastime = FALSE;
  while ((token = ol_lrc_parser_next_token (parser)) != NULL)
  {
    //ol_debugf ("token type: %d\n", ol_lrc_token_get_type (token));
    switch (ol_lrc_token_get_type (token))
    {
    case OL_LRC_TOKEN_TEXT:
      if (hastime)
      {
        _add_lyric (lrc, token->text.text);
        hastime = FALSE;
      }
      break;
    case OL_LRC_TOKEN_ATTR:
      _add_attr (lrc, &token->attr);
      break;
    case OL_LRC_TOKEN_TIME:
      hastime = TRUE;
      _add_time (lrc, &token->time);
      break;
    default:
      ol_error ("Invalid token type");
      break;
    }
    ol_lrc_token_free (token);
  }
  ol_lrc_parser_free (parser);
  g_ptr_array_sort (lrc->items, (GCompareFunc)_cmp_item);
  int id = 0;
  g_ptr_array_foreach (lrc->items, (GFunc)_set_item_id, &id);
  lrc->filename = g_strdup (filename);
  return lrc;
}

void
ol_lrc_free (struct OlLrc *lrc)
{
  ol_assert (lrc != NULL);
  if (lrc->items != NULL)
    g_ptr_array_free (lrc->items, TRUE);
  else
    ol_error ("Items are NULL");
  if (lrc->lyrics != NULL)
    g_ptr_array_free (lrc->lyrics, TRUE);
  else
    ol_error ("Lyrics are NULL");
  if (lrc->attrs != NULL)
    g_hash_table_destroy (lrc->attrs);
  else
    ol_error ("Attributes are NULL");
  if (lrc->filename != NULL)
    g_free (lrc->filename);
  g_free (lrc);
}

const struct OlLrcItem *
ol_lrc_get_item (struct OlLrc *lrc, int id)
{
  ol_assert_ret (lrc != NULL, NULL);
  if (id < 0 || id >= lrc->nitems)
    return NULL;
  return (const struct OlLrcItem*) g_ptr_array_index (lrc->items, id);
}

int
ol_lrc_item_get_id (const struct OlLrcItem *item)
{
  ol_assert_ret (item != NULL, -1);
  return item->id;
}

const struct
OlLrcItem *ol_lrc_item_prev (const struct OlLrcItem *item)
{
  ol_assert_ret (item != NULL, NULL);
  return ol_lrc_get_item (item->lrc,
                                 item->id - 1);
}

const struct
OlLrcItem *ol_lrc_item_next (const struct OlLrcItem *item)
{
  ol_assert_ret (item != NULL, NULL);
  return ol_lrc_get_item (item->lrc,
                                 item->id + 1);
}

int
ol_lrc_item_get_time (const struct OlLrcItem *item)
{
  ol_assert_ret (item != NULL, -1);
  return item->timestamp - item->lrc->offset;
}

const char
*ol_lrc_item_get_lyric(const struct OlLrcItem *item)
{
  ol_assert_ret (item != NULL, NULL);
  return g_ptr_array_index (item->lrc->lyrics, item->lyric_id);
}

int
ol_lrc_item_count (struct OlLrc *lrc)
{
  ol_assert_ret (lrc != NULL, 0);
  return lrc->nitems;
}

void
ol_lrc_get_lyric_by_time (struct OlLrc *lrc,
                          int time,
                          int music_duration,
                          char **text,
                          double *percentage,
                          int *id)
{
  /* ol_log_func (); */
  if (id != NULL)
    *id = -1;
  if (text != NULL)
    *text = NULL;
  if (percentage != NULL)
    *percentage = 0.0;
  ol_assert (lrc != NULL);
  int l = 0;
  int r = ol_lrc_item_count (lrc) - 1;
  /* ol_debugf ("r: %d\n", r); */
  /* Binary search */
  while (l < r)
  {
    /* ol_debugf ("l: %d->%d, r: %d->%d\n", */
    /*            l, ol_lrc_item_get_time (ol_lrc_get_item (lrc, l)), */
    /*            r, ol_lrc_item_get_time (ol_lrc_get_item (lrc, r))); */
    int mid = (l + r) / 2 + 1;
    const struct OlLrcItem *item = ol_lrc_get_item (lrc, mid);
    if (ol_lrc_item_get_time (item) < time)
      l = mid;
    else
      r = mid - 1;
  }
  if (l == r)                    /* found */
  {
    const struct OlLrcItem *item = ol_lrc_get_item (lrc, l);
    if (id != NULL)
      *id = l;
    if (text != NULL)
      *text = g_strdup (ol_lrc_item_get_lyric (item));
    if (percentage != NULL)
    {
      const struct OlLrcItem *next = ol_lrc_item_next (item);
      int timestamp, nextstamp;
      timestamp = ol_lrc_item_get_time (item);
      if (next != NULL)
        nextstamp = ol_lrc_item_get_time (next);
      else
        nextstamp = music_duration;
      *percentage = (double)(time - timestamp) / (nextstamp - timestamp);
      /* ol_debugf ("timestamp: %d, id: %d, per: %lf\n", timestamp, *id, *percentage); */
    }
  }
}

void
ol_lrc_set_offset(struct OlLrc *lrc, int offset)
{
  ol_log_func ();
  ol_assert (lrc != NULL);
  lrc->offset = offset;
  char *key, *value;
  key = g_strdup ("offset");
  value = g_strdup_printf ("%d", offset);
  g_hash_table_insert (lrc->attrs, key, value);
  if (lrc->filename != NULL)
    _save (lrc, lrc->filename);
}

int
ol_lrc_get_offset(struct OlLrc *lrc)
{
  ol_assert_ret (lrc != NULL, 0);
  return lrc->offset;
}

const char *
ol_lrc_get_filename (const struct OlLrc *lrc)
{
  ol_assert_ret (lrc != NULL, NULL);
  return lrc->filename;
}

static int
_save (struct OlLrc *lrc, const char *filename)
{
  ol_assert_ret (lrc != NULL, -1);
  ol_assert_ret (filename != NULL, -1);
  FILE *file = fopen (filename, "w");
  if (file == NULL)
    return -1;
  g_hash_table_foreach (lrc->attrs, (GHFunc)_save_attr, file);
  g_ptr_array_foreach (lrc->items, (GFunc)_save_lyric, file);
  fclose (file);
  return 0;
}

static void
_save_attr (char *key, char *value, FILE *file)
{
  if (value == NULL)
    fprintf (file, "[%s]\n", key);
  else
    fprintf (file, "[%s:%s]\n", key, value);
}

static void
_save_lyric (struct OlLrcItem *item, FILE *file)
{
  char *lyric = g_ptr_array_index (item->lrc->lyrics, item->lyric_id);
  int h, m, s, ms;
  h = item->timestamp / 1000 / 60 / 60;
  m = item->timestamp / 1000 / 60 % 60;
  s = item->timestamp / 1000 % 60;
  ms = item->timestamp / 10 % 100;
  if (h != 0)
    fprintf (file, "[%02d:%02d:%02d.%02d]", h, m, s, ms);
  else
    fprintf (file, "[%02d:%02d.%02d]", m, s, ms);
  fprintf (file, "%s\n", lyric);
}
