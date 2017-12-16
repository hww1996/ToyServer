#ifndef __EPOLL_H__
#define __EPOLL_H__

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

class Epoll{
public:
    Epoll(int n){
        this->epoll_fd=epoll_create(n);
    }
    int make_ctl(int action,int fd,uint32_t state){
        struct epoll_event ev;
        ev.data.fd=fd;
        ev.events=state;
        int ret=-1;
        ret=epoll_ctl(this->epoll_fd,action,fd,&ev);
        return ret;
    }
    int get_epoll_fd() const{
        return this->epoll_fd;
    }
    virtual ~Epoll(){
        close(epoll_fd);
    }
private:
    int epoll_fd;
};

#endif