#include"net.h"
#include"CharContent.h"
#include"HttpStruct.h"
#include"HttpResponse.h"
#include"Epoll.h"
#include"HttpRequest.h"
#include"source.h"
#include<fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<cstring>
using namespace std;

void *response_to_client(const CharContent &content,int connfd){
    int step=1024;
    int left_size=content.length();
    const char *content_ptr=content.c_str();
    while(left_size>0){
        if(left_size<step){
            step=left_size;
        }
        write(connfd,content_ptr,step);
        content_ptr+=step;
        left_size-=step;
    }
    close(connfd);
    return NULL;
}


void log(const char *buf){
    size_t n=strlen(buf);
    write(STDOUT_FILENO,buf,n);
    return;
}

void *handle_accept(void *arg){
    int *ptr=(int *)arg;
    int connfd=*ptr;
    delete ptr;

    Request request;
    Response response;
    HttpResponse http_response;


    Epoll e(1);
    e.make_ctl(EPOLL_CTL_ADD,connfd,EPOLLIN);
    int epoll_fd=e.get_epoll_fd();
    epoll_event event_ret;
    int ret_num=-1;
    ssize_t read_size=-1;
    CharContent ret_request;
    char buf[1024]={'\0'};
    while(true){
        ret_num=epoll_wait(epoll_fd,&event_ret,1,3000);
        if(ret_num<1){
            break;
        }
        int ret_fd=event_ret.data.fd;
        if(ret_fd==connfd&&(event_ret.events&EPOLLIN)){
            read_size=read(connfd,buf,sizeof(buf));
            if(read_size<=0){
                break;
            }
            if(read_size<sizeof(buf)){
                ret_request.append(buf,read_size);
                break;
            }
            ret_request.append(buf,read_size);
            memset(buf,'\0',sizeof(buf));
        }
    }
    write(STDOUT_FILENO,ret_request.c_str(),ret_request.length());
    e.make_ctl(EPOLL_CTL_DEL,connfd,EPOLLIN);
    if(ret_num<0){
        CharContent res_content(http_response.response_content(CharContent("500")));
        char v_log[200]={'\0'};
        snprintf(v_log,sizeof(v_log),"ret_num %d\n",ret_num);
        log(v_log);
        return response_to_client(res_content,connfd);
    }
    if(read_size<0){
        CharContent res_content(http_response.response_content(CharContent("500")));
        char v_log[200]={'\0'};
        snprintf(v_log,sizeof(v_log),"ret_num %lu\n",read_size);
        log(v_log);
        return response_to_client(res_content,connfd);
    }
    HttpRequest http_parser(ret_request);
    if(!http_parser.handle_request(&request)){
        CharContent res_content(http_response.response_content(CharContent("400")));
        char v_log[200]={'\0'};
        snprintf(v_log,sizeof(v_log),"http_parser\n");
        log(v_log);
        return response_to_client(res_content,connfd);
    }
    source server_resource(connfd);
    int resource_find_ret=server_resource.resource_find(request,&response);
    /*
     * 0 ok
     * 1 not find resource
     * 2 file type unknow(not file or document)
     * 3 exec error
     */
    if(resource_find_ret==1){
        CharContent res_content(http_response.response_content(CharContent("404")));
        char v_log[200]={'\0'};
        snprintf(v_log,sizeof(v_log),"resource_find_ret %d\n",resource_find_ret);
        log(v_log);
        return response_to_client(res_content,connfd);
    }
    if(resource_find_ret==2){
        CharContent res_content(http_response.response_content(CharContent("400")));
        char v_log[200]={'\0'};
        snprintf(v_log,sizeof(v_log),"resource_find_ret %d\n",resource_find_ret);
        log(v_log);
        return response_to_client(res_content,connfd);
    }
    if(resource_find_ret==3){
        CharContent res_content(http_response.response_content(CharContent("500")));
        char v_log[200]={'\0'};
        snprintf(v_log,sizeof(v_log),"resource_find_ret %d\n",resource_find_ret);
        log(v_log);
        return response_to_client(res_content,connfd);
    }
    CharContent res(http_response.response_content(&response,&request));
    return response_to_client(res,connfd);
}

int make_server(uint16_t port){
    int socket_fd;
    socket_fd=socket(AF_INET,SOCK_STREAM,0);
    if(socket_fd<0)
        return -1;
    sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    int ret=0;
    if((ret=bind(socket_fd,(struct sockaddr *)&addr,sizeof(addr)))<0)
        return -1;
    if((ret=listen(socket_fd,5))<0)
        return -1;
    return socket_fd;
}
