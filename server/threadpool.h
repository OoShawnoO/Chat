#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <list>
#include "locker.h"

using namespace std;

template<typename type>
class threadpool{
public:
    threadpool(int thread_number=8,int max_requests = 10000);
    ~threadpool();
    bool append(type* request);
private:
    static void* worker(void* arg);
    void run();
    int thread_number;
    pthread_t* threads;
    int max_requests;
    list<type*> workqueue;
    locker queuelocker;
    sem queuestat;
    bool stop;
};

template<typename type>
threadpool<type>::threadpool(int threadnumber,int maxrequests):
thread_number(threadnumber),
max_requests(maxrequests)
{
    if((thread_number<=0)||(max_requests<=0)){
        throw exception();
    }
    threads = new pthread_t[thread_number];
    if(!threads){
        throw exception();
    }
    for(int i=0;i<thread_number;i++){
        if(pthread_create(threads+i,NULL,worker,this)!=0){
            delete []threads;
            throw exception();
        }
        if(pthread_detach(threads[i])){
            delete []threads;
            throw exception();
        }
    }
}

template <typename type>
threadpool<type>::~threadpool(){
    delete []threads;
    stop = true;
}

template <typename type>
bool threadpool<type>::append(type* request){
    queuelocker.lock();
    if(workqueue.size()>max_requests){
        queuelocker.unlock();
        return false;
    }
    workqueue.push_back(request);
    queuelocker.unlock();
    queuestat.post();
    return true;
}

template <typename type>
void* threadpool<type>::worker(void* arg){
    threadpool* pool = (threadpool*)arg;
    pool->run();
    return pool;
}

template <typename type>
void threadpool<type>::run(){
    while(!stop){
        queuestat.wait();
        queuelocker.lock();
        if(workqueue.empty()){
            queuelocker.unlock();
            continue;
        }
        type* request = workqueue.front();
        workqueue.pop_front();
        queuelocker.unlock();

        if(!request){
            continue;
        }

        request->process();
        
    }
}

#endif