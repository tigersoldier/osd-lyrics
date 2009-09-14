/**
 * @file   ol_elapse_emulator.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Sat Sep 12 23:22:30 2009
 * 
 * @brief  Elapsed time emulator that emulates elapsed time for those players who doesn't support elapsed time in millisecond
 * 
 * 
 */
#ifndef _OL_ELAPSE_EMULATOR_H_
#define _OL_ELAPSE_EMULATOR_H_
#include <sys/time.h>

typedef struct _OlElapseEmulator OlElapseEmulator;

struct _OlElapseEmulator
{
  int first_time;
  int prev_time;
  int last_time;
  struct timeval begin_time;
  int accuracy;
};

void ol_elapse_emulator_init (OlElapseEmulator *emulator,
                              int time,
                              int accuracy);
int ol_elapse_emulator_get_real_ms (OlElapseEmulator *emulator,
                                    int time);
int ol_elapse_emulator_get_last_ms (OlElapseEmulator *emulator,
                                    int time);

#endif /* _OL_ELAPSE_EMULATOR_H_ */
