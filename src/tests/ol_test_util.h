#ifndef _OL_TEST_UTIL_H_
#define _OL_TEST_UTIL_H_

#define ol_test_expect(expr)                if (!(expr)) { printf ("ERROR: EXPECT %s failed\n", #expr); }

#endif /* _OL_TEST_UTIL_H_ */
