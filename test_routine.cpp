
#include "coroutine.h"
#include <netinet/in.h>
#include<iostream>


using namespace std;
void fun(){
    cout<<"我来到了fun1"<<endl;
    co_sleep(5);
    cout<<"我回到了fun2"<<endl;
    co_return();
}
void fun2(){
    cout<<"我来到了fun2"<<endl;
    co_sleep(10);
    cout<<"我回到了fun2"<<endl;
    co_return();
}

void nets_test(){
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
    int cfd=co_accept(listen_fd, (sockaddr *)(&cliaddr),&slen);
    cout<<"新的客户端链接"<<cfd<<endl;
    int len=co_read(cfd,buf,1024);
    cout<<"客户端口有写入请求"<<endl;
    buf[len]='\0';
    cout<<buf<<endl;
    co_return();
}
int main(){
    routine *q1=makeroutine(fun);
    routine *q2=makeroutine(fun2);
    routine *q3=makeroutine(nets_test);
    start_routine(q1);
    start_routine(q2);
    start_routine(q3);
    cout<<"main"<<endl;
    loop();
}
