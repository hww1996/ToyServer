#include"ThreadPool.h"
#include"net.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<signal.h>

int main() {
    sigset_t mask_sig_set;
    sigaddset(&mask_sig_set,SIGPIPE);
    sigprocmask(SIG_BLOCK,&mask_sig_set,NULL);
    int sock_fd=make_server(1111);
    if(sock_fd<0)
        exit(-1);
    ThreadPool t(10,10);
    while(true){
        int *connfd_ptr=NULL;
        connfd_ptr=new int(-1);
        *connfd_ptr=accept(sock_fd,NULL,NULL);
        if(*connfd_ptr<0)
            break;
        t.add_to_queue(Task(handle_accept,(void *)connfd_ptr));
    }
    close(sock_fd);
    return 0;
}
