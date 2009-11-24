#ifndef _OL_DEBUG_H_
#define _OL_DEBUG_H_

enum OlDebugLevel {
  OL_ERROR = 0,
  OL_DEBUG = 1,
  OL_INFO = 2,
};

#define ol_logf(level, ...)        ( fprintf (stderr, __VA_ARGS__))

#endif /* _OL_DEBUG_H_ */
