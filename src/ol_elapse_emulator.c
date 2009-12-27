#include <stdio.h>
#include "ol_elapse_emulator.h"

void
ol_elapse_emulator_init (OlElapseEmulator *emulator,
                         int time,
                         int accuracy)
{
  if (emulator == NULL)
    return;
  emulator->first_time = time;
  emulator->prev_time = time;
  emulator->last_time = time;
  gettimeofday (&emulator->begin_time, NULL);
  emulator->accuracy = accuracy;
}

int
ol_elapse_emulator_get_real_ms (OlElapseEmulator *emulator,
                                int time)
{
  if (emulator->first_time < 0 || emulator->prev_time - time > emulator->accuracy || time - emulator->last_time > 150)
  {
    /* reinitialize timer */
    puts ("init1");
    printf ("prev:%d, time:%d\n", emulator->prev_time, time);
    ol_elapse_emulator_init (emulator, time, emulator->accuracy);
  }
  else
  {
    struct timeval current_time;
    gettimeofday (&current_time, NULL);
    int real_time = emulator->first_time +
      (current_time.tv_sec - emulator->begin_time.tv_sec) * 1000 +
      (current_time.tv_usec - emulator->begin_time.tv_usec) / 1000;
    if (real_time - time > 2 * emulator->accuracy || time - real_time > emulator->accuracy )
    {
      puts ("init2");
      printf ("real_time: %d, time: %d\n", real_time, time);
      ol_elapse_emulator_init (emulator, time, emulator->accuracy);
    }
    else
    {
      emulator->prev_time = time;
      time = real_time;
    }
  }
  emulator->last_time = time;
  return time;
}

int
ol_elapse_emulator_get_last_ms (OlElapseEmulator *emulator,
                                int time)
{
  if (emulator->first_time < 0 || emulator->last_time - time > emulator->accuracy || time - emulator->last_time > emulator->accuracy)
  {
    /* reinitialize timer */
    printf ("prev:%d, time:%d\n", emulator->prev_time, time);
    ol_elapse_emulator_init (emulator, time, emulator->accuracy);
  }
  return emulator->last_time;
}

