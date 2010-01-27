/**
 * @file   ol_fork.h
 * @author Tiger Soldier <tigersoldi@gmail.com>
 * @date   Tue Jan 26 22:26:08 2010
 * 
 * @brief  The IPC mechanism receiving return data from child process
 * 
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
 * 
 * @param callback Callback function when child process return something
 * @param data Userdata to pass to callback function
 * 
 * @return 0 if it's a child process, or the pid of the forked child process,
 * or -1 if failed withou child process created
 */
pid_t ol_fork (OlForkCallback callback, void *data);
//@}
#endif /* _OL_FORK_H_ */
