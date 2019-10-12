#include"coroutine.h"



#define SIZE_OF_STACK 1024*128//每一个协程的栈大小
using namespace std;
static bool start_pro=0;
static vector<routine*> pro(1000);
static timelist* tl;
static int size=0;//time链表的数量
static bool epoll_start=false;
static char 	buf[100];
static socklen_t	len = sizeof( struct sockaddr_in );
static sockaddr_in clientaddr;
static map<int,routine*> m;
static struct epoll_event ev;
static struct epoll_event evs[1000];
static int epoll_fd;
static int num_fd;//epoll树上的数量
//创建一个协程，返回这个上下文
routine* makeroutine(void (*func)()){
    if(!start_pro){
        start_pro= true;
        ucontext_t *main=(ucontext_t*)malloc(sizeof(ucontext_t));
        routine *pm=(routine*)malloc(sizeof(routine));
        pm->uct=main;
        pro[size++]=(pm);
    }
    routine *p=(routine*)malloc(sizeof(routine));
    ucontext_t *pp=(ucontext_t*)malloc(sizeof(ucontext_t));
    getcontext(pp);
    if(!start_pro) pp->uc_link=0;
    else pp->uc_link=pro[size-1]->uct;
    pp->uc_stack.ss_sp=(char *)malloc(SIZE_OF_STACK);
    pp->uc_stack.ss_size=SIZE_OF_STACK;
    p->uct=pp;
    makecontext(pp, func,0);
    return p;
}

//开始一个协程
void start_routine(routine * wanna_begin){

    pro[size++]=wanna_begin;
    swapcontext(pro[size-2]->uct,pro[size-1]->uct);
}
 void co_sleep(int sec){

    //1 加入time等待链表
    if(tl==NULL){
        tl=(timelist*)malloc(sizeof(timelist));
    }
    if(tl->head==NULL){
        tl->head=(timelistnode*)malloc(sizeof(timelistnode));
        tl->tail=tl->head;
        //cout<<size<<endl;
        tl->head->t=pro[size-1];
        tl->head->sec=time(0)+sec;
    }
    else{
        timelistnode* t=(timelistnode*)malloc(sizeof(timelistnode));
        t->t=pro[size-1];
        t->next=NULL;
        t->sec=time(0)+sec;
        tl->tail->next=t;
        t->pre=tl->tail;
        tl->tail=t;
        tl->n++;
    }


    //2。切换
    size--;
    swapcontext(pro[size]->uct,pro[size-1]->uct);
}
 void co_return(){

    //free(pro[size-1]->uct);
    //free(pro[size-1]);
    size--;
}

inline void co_net_wait(int fd,bool w){
    //1步,上树
    if(!epoll_start){
        epoll_start= true;
        epoll_fd=epoll_create(1000);
    }
    if(w){
        ev.events=EPOLLOUT;
    }
    else {
        ev.events = EPOLLIN;
    }
    ev.data.fd=fd;
    epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &ev );
    num_fd+=1;
    //2步，切换
    m.insert(pair<int,routine*>(fd,pro[size-1]));
    size--;
    swapcontext(pro[size]->uct,pro[size-1]->uct);
}

inline  void co_net_end(int fd){
    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,NULL);
    --num_fd;
}
 void co_close_fd(int fd){
    --num_fd;
}
int co_accept(int fd, struct sockaddr* addr, socklen_t* len){
    co_net_wait(fd,0);

    int cfd=accept(fd, addr,len);
    co_net_end(fd);
    return cfd;
}
int co_read(int handle, void *buf, int nbyte){
    co_net_wait(handle,0);
    int n=read(handle,buf,nbyte);
    co_net_end(handle);
    return n;
}
int co_write(int fd, const void *buf, size_t nbyte){
    co_net_wait(fd,1);
    int n=write(fd,buf,nbyte);
    co_net_end(fd);
    return n;
}
void loop(){
    //cout<<"我来到了loop"<<endl;
    for(;;){
        //cout<<"s"<<endl;
        int wait_num=epoll_wait(epoll_fd,evs,num_fd,1);
        for(int i=0;i<wait_num;++i){
            start_routine(m[evs[i].data.fd]);
        }
        //下面是处理时间队列中到期的协程
        timelistnode* p=tl->head;
        if(p!=NULL){
            if(p->sec<time(0)){
                //接下来把p给移除
                routine *ppt = p->t;
                if(p->pre!=NULL&&p->next!=NULL) {
                    p->pre->next = p->next;
                    p->next->pre = p->pre;
                    //routine *ppt = p->t;
                    free(p);
                }
                else if(p->pre!=NULL){
                    p->pre->next=NULL;
                    tl->tail=p->pre;
                    free(p);
                }
                else if(p->next!=NULL){
                    p->next->pre=NULL;
                    tl->head=p->next;
                    free(p);
                }
                else {
                    free(p);
                    tl->head=NULL;
                    tl->tail=NULL;
                }
                start_routine(ppt);
                //free(p);
                //cout<<"第二次回来了！"<<endl;
            }
        }
    }
}
