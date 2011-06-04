/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2009-2011  Tiger Soldier <tigersoldier@gmail.com>
 *
 * This file is part of OSD Lyrics.
 * 
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
