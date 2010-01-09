#ifndef _OL_DEBUG_H_
#define _OL_DEBUG_H_

#include <stdio.h>

enum OlDebugLevel {
  OL_ERROR = 0,
  OL_DEBUG = 1,
  OL_INFO = 2,
};

#define ol_logf(level, ...)           ( fprintf (stderr, __VA_ARGS__))
#define ol_log_func()                 ( ol_logf (OL_DEBUG, "%s\n", __FUNCTION__))
#define ol_debugf(...)                ( ol_logf (OL_DEBUG, __VA_ARGS__))
#define ol_debug(...)                 ( ol_logf (OL_DEBUG, "%s\n", __VA_ARGS__))
#define ol_errorf(...)                ( ol_logf (OL_ERROR, __VA_ARGS__))
#define ol_error(...)                 ( ol_logf (OL_ERROR, "%s\n", __VA_ARGS__))
#define ol_assert(assertion)          if (!(assertion)) {      \
    ol_logf (OL_ERROR, "assert %s failed\n", #assertion);   \
    return;                                                 \
  }
#define ol_assert_ret(assertion, ret) if (!(assertion)) {   \
    ol_logf (OL_ERROR, "assert %s failed\n", #assertion);   \
    return (ret);                                           \
  }
#endif /* _OL_DEBUG_H_ */
