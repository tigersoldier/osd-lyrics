#include <stdio.h>
#include "ol_lrc_parser.h"
#include "ol_test_util.h"

const char FILENAME[] = "lrc_gbk.lrc";

void print_token (union OlLrcToken *token)
{
  switch (ol_lrc_token_get_type (token))
  {
  case OL_LRC_TOKEN_TEXT:
    printf ("Text token: %s\n", token->text.text);
    break;
  case OL_LRC_TOKEN_ATTR:
    printf ("Attr token: attr: %s, value: %s\n",
            token->attr.attr, token->attr.value);
    break;
  case OL_LRC_TOKEN_TIME:
    printf ("Time token: %02d:%02d:%02d.%02d\n",
            token->time.time / 1000 / 60 / 60,
            token->time.time / 1000 / 60 % 60,
            token->time.time / 1000 % 60,
            token->time.time / 10 % 100);
    break;
  default:
    printf ("Invalid Token\n");
    break;
  }
}

void general_test (struct OlLrcParser *parser)
{
  union OlLrcToken *token = NULL;
  while ((token = ol_lrc_parser_next_token (parser)) != NULL)
  {
    print_token (token);
    ol_lrc_token_free (token);
  }
}

void test_file ()
{
  struct OlLrcParser *parser = ol_lrc_parser_new_from_file (FILENAME);
  general_test (parser);
  ol_lrc_parser_free (parser);
}

/* issue 80 */
void test_bom ()
{
  const char *FILENAME = "lrc_bom.lrc";
  struct OlLrcParser *parser = ol_lrc_parser_new_from_file (FILENAME);
  union OlLrcToken *token = NULL;
  ol_test_expect ((token = ol_lrc_parser_next_token (parser)) != NULL);
  ol_test_expect (ol_lrc_token_get_type (token) == OL_LRC_TOKEN_TIME);
  ol_lrc_token_free (token);
  ol_lrc_parser_free (parser);
}

int main()
{
  test_file ();
  test_bom ();
}
