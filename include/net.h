#ifndef __NET_H__
#define __NET_H__
#include"CharContent.h"
#include<fcntl.h>
#include<unistd.h>
#include<cstring>

void *response_to_client(const CharContent &content,int connfd);

void log(const char *buf);

void *handle_accept(void *arg);

int make_server(uint16_t port);

#endif
