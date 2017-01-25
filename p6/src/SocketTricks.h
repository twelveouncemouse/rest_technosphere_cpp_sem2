/*
 * SocketTricks.h
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#ifndef SOCKETTRICKS_H_
#define SOCKETTRICKS_H_

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <string.h>

int set_nonblock(int fd);

ssize_t sock_fd_write(int sock, void *buf, ssize_t buflen, int fd);
ssize_t sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd);

#endif /* SOCKETTRICKS_H_ */
