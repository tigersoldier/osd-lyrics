#ifndef _OL_DEBUG_H_
#define _OL_DEBUG_H_

enum OlDebugLevel {
  OL_ERROR = 0,
  OL_DEBUG = 1,
  OL_INFO = 2,
};

#define ol_logf(level, ...)        ( fprintf (stderr, __VA_ARGS__))
#define ol_log_func()              ( ol_logf (OL_DEBUG, "%s\n", __FUNCTION__))
#define ol_debugf(...)             ( ol_logf (OL_DEBUG, __VA_ARGS__))

#endif /* _OL_DEBUG_H_ */
