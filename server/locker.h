#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>

using namespace std;

//封装互斥锁
class locker{
private:
    pthread_mutex_t mutex;
public:
    locker(){
        if(pthread_mutex_init(&mutex,NULL)!=0){
            throw exception();
        }
    }
    ~locker(){
        pthread_mutex_destroy(&mutex);
    }
    bool lock(){
        return pthread_mutex_lock(&mutex) == 0;
    }
    bool unlock(){
        return pthread_mutex_unlock(&mutex) == 0;
    }
    pthread_mutex_t* get(){
        return &mutex;
    }
};

//封装条件变量
class cond{
public:
    cond(){
        if(pthread_cond_init(&m_cond,NULL)!=0){
            throw std::exception();
        }
    }

    ~cond(){
        pthread_cond_destroy(&m_cond);
    }
    
    bool wait(pthread_mutex_t *mutex){
        return pthread_cond_wait(&m_cond,mutex) == 0;
    }

    bool timewait(pthread_mutex_t *mutex,struct timespec t){
        return pthread_cond_timedwait(&m_cond,mutex,&t) == 0;
    }


    bool signal(pthread_mutex_t *mutex){
        return pthread_cond_signal(&m_cond) == 0;
    }

    bool broadcast(){
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    pthread_cond_t m_cond;
};

//信号量
class sem{
public:
    sem(){
        if(sem_init(&m_sem,0,0)!=0){
            throw std::exception();
        }
    }
    sem(int num){
        if(sem_init(&m_sem,0,num)!=0){
            throw std::exception();
        }
    }
    ~sem(){
        sem_destroy(&m_sem);
    }
    //等待信号量
    bool wait(){
        return sem_wait(&m_sem) == 0;
    }
    //增加信号量
    bool post(){
        return sem_post(&m_sem) == 0;
    }


private:
    sem_t m_sem;
};

#endif