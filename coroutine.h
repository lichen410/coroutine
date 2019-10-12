
#ifndef UNTITLED1_COROUTINE_H
#define UNTITLED1_COROUTINE_H
#include <ucontext.h>
#include<time.h>
#include<string.h>
#include<iostream>
#include<vector>
#include<map>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/resource.h>
struct routine{
    //int id;//这个协程的标识
    //int f;//是否结束的标识
    ucontext_t *uct;
};
struct timelistnode{
    routine *t;
    time_t sec;
    timelistnode* next;
    timelistnode *pre;
};
struct timelist{
    timelistnode *head;
    timelistnode *tail;
    int n=0;
};
 routine* makeroutine(void (*)(routine *));
 void start_routine(routine * );
 void co_sleep(int);
void co_return(routine *);
int co_accept(int , struct sockaddr* , socklen_t* );
int co_read(int , void *, int );
int co_write(int , const void *, size_t );
void loop();
#endif //UNTITLED1_COROUTINE_H
