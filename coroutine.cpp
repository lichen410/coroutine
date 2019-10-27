#include"coroutine.h"



#define SIZE_OF_STACK 1024*128//每一个协程的栈大小

static timelist* tl=new timelist();

static int count=0;
static bool start_pro=0;
//static int id_of_routine;
static vector<routine*> reroutine(1000);
static int resize=0;
static vector<routine*> pro(1000);
static int size=0;//time链表的数量
static bool epoll_start=false;
//static char 	buf[100];
static map<int,routine*> m;
static struct epoll_event ev;
static struct epoll_event evs[1000];
static int epoll_fd;
static int num_fd;//epoll树上的数量
//创建一个协程，返回这个上下文

using namespace std;



routine* makeroutine(void (*func)(routine *)){
    if(!start_pro){
        start_pro= true;
        ucontext_t *main=(ucontext_t*)malloc(sizeof(ucontext_t));
        getcontext(main);
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
   // p->f=0;
   // p->id=id_of_routine++;
    makecontext(pp, (void(*)())func,1,p);
    ++count;
    return p;
}

//开始一个协程
 void start_routine(routine * wanna_begin){

    pro[size++]=wanna_begin;
    swapcontext(pro[size-2]->uct,pro[size-1]->uct);
}
void co_sleep(int sec){

    //1 加入time等待链表

    timelistnode* p_timenode=(timelistnode*)malloc(sizeof(timelistnode));
    p_timenode->sec=time(0)+sec;
    p_timenode->t=pro[size-1];
    tl->add(p_timenode);



    //2。切换
    size--;
    swapcontext(pro[size]->uct,pro[size-1]->uct);
}
 void co_return(routine *rep){

    //free(pro[size-1]->uct);
    //free(pro[size-1]);
    reroutine[++resize]=rep;
    size--;
    swapcontext(pro[size]->uct,pro[size-1]->uct);
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

    for(;;){
        if(!count){
            free(tl);
            break;
        }
        //cout<<"我要去睡"<<tl->min_time<<"秒"<<endl;
        int wait_num=epoll_wait(epoll_fd,evs,num_fd,tl->min_time);
        for(int i=0;i<wait_num;++i){
            start_routine(m[evs[i].data.fd]);
        }


        //cout<<"下面是处理时间队列中到期的协程"<<endl;
        vector<timelistnode*> end_timenode=tl->loop_find_end_time();

        for(int i=end_timenode.size()-1;i>=0;--i){

            routine *ppt=end_timenode[i]->t;

            timelistnode* ppn=end_timenode[i];

            end_timenode.pop_back();

            free(ppn);
            start_routine(ppt);
        }


        while(resize>0){
            routine *tt=reroutine[resize];
            free(tt->uct->uc_stack.ss_sp);
            free(tt->uct);
            free(tt);
            --resize;
            --count;
        }
    }
}
