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
#include <unistd.h>

/** 
 * @brief Callback function of child process
 * 
 * @param fd The file descripter of child process's output
 * @param data The user data passed in ol_fork
 * 
 */
typedef void (*OlForkCallback) (int fd, void *data);

/** 
 * @brief Fork a child process
 * A pipe will be open for the parent and child process.
 * The parent process holds the read end of the pipe, and
 * gets the file description of the pipe in the callback function.
 * The child process holds the write end of the pipe.
 * The stdout of child process is opend as the write end of the pipe.
 * Once the child process writes something to stdout,
 * the callback will be invoked to receive the output.
 * @param callback Callback function when child process return something
 * @param data Userdata to pass to callback function
 * 
 * @return 0 if it's a child process, or the pid of the forked child process,
 * or -1 if failed withou child process created
 */
pid_t ol_fork (OlForkCallback callback, void *data);
//@}
#endif /* _OL_FORK_H_ */
