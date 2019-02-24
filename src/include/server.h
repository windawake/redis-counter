#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#ifndef CONNECT_SIZE
#define CONNECT_SIZE 256
#endif

#define HOST_IP "0.0.0.0"
#define PORT 9382
#define MAX_LINE 2048
#define LISTENQ 20

#define DB_NAME "/home/zhang/share/redis-counter/chat.db"


