# coroutine
是一个模仿go协程的c++协程库


创建一个协程：
routine* makeroutine(void (*)())

开始一个协程：
void start_routine(routine * );


注意，在协程中不能用会引起阻塞的方法，用提供的方法代替：


void co_sleep(int);


int co_accept(int , struct sockaddr* , socklen_t* );

int co_read(int , void *, int );

int co_write(int , const void *, size_t );




下面是一个开启协程的例子：

int main(){

    routine *q1=makeroutine(fun);
    
    routine *q2=makeroutine(fun2);
    
    routine *q3=makeroutine(nets_test);
    
    start_routine(q1);
    
    start_routine(q2);
    
    start_routine(q3);
    
    cout<<"main"<<endl;
    
    loop();//开启所有协程后调用这个函数
    
    
}

