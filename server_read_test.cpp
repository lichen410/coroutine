//
// Created by 李晨 on 2019/10/25.
//  用协程实现。不停的接受链接，然后读取一条消息。
//
#include "coroutine.h"
#include<iostream>
using namespace std;

static int pfd;
void handler(routine *p){
    int f=pfd;
    char buf[1024];
    int n=co_read(f,buf,1024);
    buf[n]='\0';
    cout<<buf<<endl;
    char a[5]="400";
    //int pt=co_write(f,a,5);
    co_return(p);
}
void nets_test(routine* p){
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    char buf[1024];
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
    servaddr.sin_port = htons( 8000 );
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    bind(listen_fd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr));
    listen(listen_fd,3);
    socklen_t slen=sizeof(struct sockaddr_in);
    for(;;) {
        pfd = co_accept(listen_fd, (sockaddr *) (&cliaddr), &slen);
        cout << "新的客户端链接" << pfd<< endl;
        routine *ss=makeroutine(handler);
        start_routine(ss);

    }

}
int main(){

    routine *p =makeroutine(nets_test);
    start_routine(p);
    cout<<"回到main"<<endl;
    loop();
}


