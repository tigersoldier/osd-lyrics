/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009-2011  Tiger Soldier
 *
 * This file is part of OSD Lyrics.
 * OSD Lyrics is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OSD Lyrics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OSD Lyrics.  If not, see <http://www.gnu.org/licenses/>. 
 */
#include <stdio.h>
#include <glib.h>
#include "ol_elapse_emulator.h"
#include "ol_debug.h"

OlElapseEmulator *
ol_elapse_emulator_new (int initial_time, int accuracy)
{
  OlElapseEmulator *emulator = g_new (OlElapseEmulator, 1);
  ol_elapse_emulator_init (emulator, initial_time, accuracy);
  return emulator;
}

void
ol_elapse_emulator_free (OlElapseEmulator *emulator)
{
  g_free (emulator);
}

void
ol_elapse_emulator_init (OlElapseEmulator *emulator,
                         int initial_time,
                         int accuracy)
{
  ol_assert (emulator != NULL);
  emulator->first_time = initial_time;
  emulator->prev_time = initial_time;
  emulator->last_time = initial_time;
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
    ol_infof ("prev:%d, time:%d\n", emulator->prev_time, time);
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
      ol_infof ("real_time: %d, time: %d\n", real_time, time);
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
    ol_debugf ("prev:%d, time:%d\n", emulator->prev_time, time);
    ol_elapse_emulator_init (emulator, time, emulator->accuracy);
  }
  return emulator->last_time;
}
