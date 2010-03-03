#ifndef _OL_TEST_UTIL_H_
#define _OL_TEST_UTIL_H_

#include <stdio.h>

#define ol_test_expect(expr)                if (!(expr)) { printf ("ERROR[%d]: EXPECT %s failed\n", __LINE__, #expr); }

#endif /* _OL_TEST_UTIL_H_ */
