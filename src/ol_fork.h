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
 * @file   ol_fork.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Tue Jan 26 22:26:08 2010
 * 
 * @brief  The IPC mechanism receiving return data from child process
 * For the usage, see test/ol_fork_test.c
 * 
 */

//@{

#ifndef _OL_FORK_H_
#define _OL_FORK_H_
#include <stdio.h>
#include <unistd.h>

extern int ret_fd;
extern FILE *fret;

/** 
 * @brief Callback function of child process
 * 
 * @param ret_data The data returned by child process
 * @param ret_size The number of bytes returned by child process
 * @param status Status information about the child process,
 *               see waitpid(2) for more information about this field
 * @param userdata The user data passed in ol_fork
 * 
 */
typedef void (*OlForkCallback) (void *ret_data,
                                size_t ret_size,
                                int status,
                                void *userdata);

/** 
 * @brief Fork a child process
 * To return data to parent process, child process just need to
 * write the return data to the file descriptor ret_fd, or to the
 * C file stream fret.
 * Once the child process exits, the callback function will be
 * called.
 * @param callback Callback function
 * @param userdata Userdata to pass to callback function
 * 
 * @return 0 if it's a child process, or the pid of the forked child process,
 * or -1 if failed withou child process created
 */
pid_t ol_fork (OlForkCallback callback, void *userdata);
//@}
#endif /* _OL_FORK_H_ */
