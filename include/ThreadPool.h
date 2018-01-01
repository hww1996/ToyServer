#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__
#include<pthread.h>
#include<unistd.h>
#include<cstdlib>
#include<sys/types.h>
#include<deque>
#include<vector>



struct Task{
public:
    Task(void *(*f)(void *),void *arg):func(f),arg(arg){}
    virtual ~Task(){}
    void *run(){
        return this->func(this->arg);
    }
private:
    void *(*func)(void *);
    void *arg;
};

class ThreadPool{
public:
    ThreadPool(size_t qs=10,size_t tc=10){
        queue_size=qs;
        threads_size=tc;
        is_running=true;
        pthread_cond_init(&not_empty,NULL);
        pthread_cond_init(&not_full,NULL);
        pthread_mutex_init(&mlock,NULL);
        threads=std::vector<pthread_t>(tc,0);
        for(size_t i=0;i<tc;i++){
            pthread_create(&threads[i],NULL,thread_function,NULL);
        }
        is_running=true;
    }
    void add_to_queue(const Task &task){
        pthread_mutex_lock(&mlock);
        while(thread_queue.size()==queue_size&&is_running){
            pthread_cond_wait(&not_full,&mlock);
        }
        if(!is_running){
            pthread_mutex_unlock(&mlock);
        }
        thread_queue.push_back(task);
        pthread_cond_broadcast(&not_empty);
        pthread_mutex_unlock(&mlock);
    }
    void stop(){
        pthread_mutex_lock(&mlock);
        if(!is_running){
            pthread_mutex_unlock(&mlock);
            return;
        }
        is_running=false;
        pthread_cond_broadcast(&not_empty);
        pthread_cond_broadcast(&not_full);
        pthread_mutex_unlock(&mlock);
        for(size_t i=0;i<threads_size;i++){
            pthread_join(threads[i],NULL);
        }

        pthread_cond_destroy(&not_empty);
        pthread_cond_destroy(&not_full);
        pthread_mutex_destroy(&mlock);
    }
    virtual ~ThreadPool(){
        stop();
    }
private:
    static void *thread_function(void *arg){
        while(true){
            pthread_mutex_lock(&mlock);
            while(thread_queue.empty()&&is_running){
                pthread_cond_wait(&not_empty,&mlock);
            }
            if(!is_running){
                pthread_mutex_unlock(&mlock);
                break;
            }
            Task t=thread_queue.front();
            thread_queue.pop_front();
            pthread_cond_broadcast(&not_full);
            pthread_mutex_unlock(&mlock);
            t.run();
        }
        pthread_exit(NULL);
    }
    static std::deque<Task> thread_queue;
    static size_t queue_size;
    static size_t threads_size;
    static bool is_running;
    std::vector<pthread_t> threads;
    static pthread_cond_t not_full;
    static pthread_cond_t not_empty;
    static pthread_mutex_t mlock;
};
std::deque<Task> ThreadPool::thread_queue;
size_t ThreadPool::queue_size;
size_t ThreadPool::threads_size;
bool ThreadPool::is_running;
pthread_cond_t ThreadPool::not_full;
pthread_cond_t ThreadPool::not_empty;
pthread_mutex_t ThreadPool::mlock;


#endif
