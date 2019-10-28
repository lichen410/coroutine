
#ifndef UNTITLED1_COROUTINE_H
#define UNTITLED1_COROUTINE_H
#include <ucontext.h>
#include<time.h>
#include<string.h>
#include<iostream>
#include<vector>
#include<map>
#include<list>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/resource.h>
using namespace std;
struct routine{
    //int id;//这个协程的标识
    //int f;//是否结束的标识
    ucontext_t *uct;
};
struct timelistnode{
    routine *t;
    time_t sec;
};


class timelist{
   list<timelistnode*> time_list;

public:
    int min_time;
    timelist(){
        min_time=3;
    }
    void add(timelistnode* p_time_node){
        if(p_time_node->sec-time(0)<min_time) min_time=p_time_node->sec-time(0);
        time_list.push_back(p_time_node);
    }
    vector<timelistnode*> loop_find_end_time(){
        vector<timelistnode*> ans;
        min_time=3;
        list<timelistnode*>::iterator iter=time_list.begin();
        for(;iter!=time_list.end();){
            if((*iter)->sec<=time(0)){
                ans.push_back(*iter);
                time_list.erase(iter++);
            }
            else{
                if((*iter)->sec-time(0)<min_time) min_time=(*iter)->sec-time(0);
                iter++;
            }
        }
        return ans;
    }
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
