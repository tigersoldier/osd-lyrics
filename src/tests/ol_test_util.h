#ifndef _OL_TEST_UTIL_H_
#define _OL_TEST_UTIL_H_

#include <stdio.h>
#include <string.h>

#define ol_test_expect(expr)                                      \
  if (!(expr))                                                    \
  { printf ("ERROR[%d]: EXPECT %s failed\n", __LINE__, #expr); }

#define ol_test_expect_streq(expr1, expr2)        \
  ol_test_expect (strcmp ((expr1), (expr2)) == 0)

#endif /* _OL_TEST_UTIL_H_ */
