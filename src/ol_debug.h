#ifndef _OL_DEBUG_H_
#define _OL_DEBUG_H_

#include <stdio.h>

enum OlDebugLevel {
  OL_ERROR = 0,
  OL_DEBUG = 1,
  OL_INFO = 2,
};

#define ol_logf(level, ...)           do {fprintf (stdout, "%s[%d]:", __FILE__, __LINE__); \
    fprintf (stdout, __VA_ARGS__); } while (0)
#define ol_log_func()                 do {ol_logf (OL_DEBUG, "%s\n", __FUNCTION__); } while (0)
#define ol_debugf(...)                do {ol_logf (OL_DEBUG, __VA_ARGS__); } while (0)
#define ol_debug(...)                 do {ol_logf (OL_DEBUG, "%s\n", __VA_ARGS__); } while (0)
#define ol_errorf(...)                do {ol_logf (OL_ERROR, __VA_ARGS__); } while (0)
#define ol_error(...)                 do {ol_logf (OL_ERROR, "%s\n", __VA_ARGS__); } while (0)
#define ol_assert(assertion)          do {if (!(assertion)) {      \
      ol_logf (OL_ERROR, "assert %s failed\n", #assertion);        \
      return;                                                      \
    }} while (0)
#define ol_assert_ret(assertion, ret) do {if (!(assertion)) {   \
      ol_logf (OL_ERROR, "assert %s failed\n", #assertion);     \
      return (ret);                                             \
    }} while (0)
#endif /* _OL_DEBUG_H_ */
