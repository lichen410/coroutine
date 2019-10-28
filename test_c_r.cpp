//
// Created by 李晨 on 2019/10/12.
//
#include"coroutine.h"
#include<iostream>
using namespace std;


static int proc;



void pro(routine *p){
    while(1){
        proc++;
        cout<<"生产一个，一共有"<<proc<<"个"<<endl;
        co_sleep(1);
    }
    co_return(p);
}

void eat(routine *p){
    while(1){
        while(proc<1) co_sleep(1);
        proc--;
        cout<<"吃掉一个，还剩下："<<proc<<"个"<<endl;
    }
    co_return(p);
}
int main(){
    routine* p1=makeroutine(pro);
    routine* p2=makeroutine(eat);
    start_routine(p1);
    start_routine(p2);
    loop();
}
