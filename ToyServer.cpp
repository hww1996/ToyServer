#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <map>
#include <set>
#include <unordered_set>
#include <cstring>
#include <malloc.h>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <deque>
#include <random>
#include <signal.h>
#include <wait.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

using namespace std;

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
        threads=vector<pthread_t>(tc,0);
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
    static deque<Task> thread_queue;
    static size_t queue_size;
    static size_t threads_size;
    static bool is_running;
    vector<pthread_t> threads;
    static pthread_cond_t not_full;
    static pthread_cond_t not_empty;
    static pthread_mutex_t mlock;
};
deque<Task> ThreadPool::thread_queue;
size_t ThreadPool::queue_size;
size_t ThreadPool::threads_size;
bool ThreadPool::is_running;
pthread_cond_t ThreadPool::not_full;
pthread_cond_t ThreadPool::not_empty;
pthread_mutex_t ThreadPool::mlock;

class CharContent{
public:
    CharContent(){
        m=new char [10];
        size=0;
        m_length=10;
        memset(m,'\0',m_length);
    }
    CharContent(const CharContent &cc){
        size_t n=cc.m_length;
        m=new char [n];
        size=cc.size;
        m_length=cc.m_length;
        memset(m,'\0',m_length);
        strncpy(m,cc.m,cc.m_length);
    }
    CharContent(const char *cc,size_t n){
        if(n==0){
            m=new char [10];
            size=0;
            m_length=10;
            memset(m,'\0',m_length);
            return;
        }else{
            m_length=2*n;
            m=new char [m_length];
            size=n;
            memset(m,'\0',m_length);
            for(size_t i=0;i<n;i++){
                m[i]=cc[i];
            }
            return;
        }
    }
    CharContent(const char *cc){
        size_t n=strlen(cc);
        if(n==0){
            m=new char [10];
            size=0;
            m_length=10;
            memset(m,'\0',m_length);
            return;
        }else{
            m_length=2*n;
            m=new char [m_length];
            size=n;
            memset(m,'\0',m_length);
            for(size_t i=0;i<n;i++){
                m[i]=cc[i];
            }
            return;
        }
    }
    CharContent(size_t n){
        m=new char[n+10];
        size=n;
        m_length=n+10;
        memset(m,'\0',(n+10)*sizeof(char));
    }
    CharContent &operator=(const CharContent &cc){
        delete[] m;
        size=cc.size;
        m_length=cc.m_length;
        m=new char [m_length];
        memset(m,'\0',m_length);
        strncpy(m,cc.m,size);
        return *this;
    }

    CharContent &operator=(const char *s){
        return this->operator=(CharContent(s));
    }

    const char &operator[](size_t pos) const{
        if(pos>=size)
            throw std::out_of_range("CharContent get element out of range.");
        return *(m+pos);
    }
    char &operator[](size_t pos){
        if(pos>=size)
            throw std::out_of_range("CharContent get element out of range.");
        return *(m+pos);
    }
    const char *c_str() const{
        return this->m;
    }
    char *c_str(){
        return this->m;
    }
    size_t length() const{
        return this->size;
    }
    size_t  capacity()const{
        return this->m_length;
    }
    void clear(){
        this->size=0;
    }
    void append(const char *s,size_t n){
        if(n<m_length-size){
            size_t j=size;
            for(size_t i=0;i<n;i++){
                m[j++]=s[i];
            }
            size=size+n;
            return;
        }
        size_t new_size=(n+size)*2;
        char *new_ptr=new char [new_size];
        memset(new_ptr,'\0',new_size);
        size_t i=0;
        for(;i<size;i++){
            new_ptr[i]=m[i];
        }
        size_t j=0;
        for(;j<n;j++){
            new_ptr[i++]=s[j];
        }
        size=i;
        m_length=new_size;
        delete[] m;
        m=new_ptr;
    }
    void append(const CharContent &cc){
        return this->append(cc.m,cc.size);
    }
    void append(const char *s){
        return this->append(s,strlen(s));
    }

    CharContent subCharContent(size_t pos,size_t length){
        if(length==npos){
            if(pos>=this->size)
                return CharContent("");
            return CharContent(m+pos,this->size-pos);
        }
        if(length+pos>this->size){
            if(pos>=this->size)
                return CharContent("");
            return CharContent(m+pos,this->size-pos);
        }
        return CharContent(m+pos,length);
    }

    size_t find(const char *s,size_t n,size_t pos=0){//can use KMP
        if(n==0)
            return 0;
        if(n>size-pos)
            return this->npos;
        for(size_t i=pos;i<=size-n;i++){
            bool found=true;
            for(size_t j=0;j<n;j++){
                if(m[j+i]!=s[j]){
                    found=false;
                    break;
                }
            }
            if(found)
                return i;
        }
        return this->npos;
    }

    size_t find(const CharContent &cc,size_t pos=0){
        return this->find(cc.m,cc.size,pos);
    }

    vector<CharContent> split(const CharContent &s){
        vector<CharContent> ans;
        size_t start=0,pos=0;
        while((pos=this->find(s,start))!=this->npos){
            if(pos>start)
                ans.push_back(this->subCharContent(start,pos-start));
            start=pos+s.length();
        }
        if(start<this->size)
            ans.push_back(this->subCharContent(start,this->size-start));
        return ans;
    }

    CharContent strip(const CharContent &s){
        size_t n=s.length(),m=this->size;

        if(m<n)
            return *this;
        size_t start=0,i=0;
        while(i<m){
            for(size_t j=0;j<n;j++){
                if(s[j]!=this->m[i])
                    break;
                i++;
            }
            if(i==start)
                break;
            if(i%n==0)
                start=i;
            else
                break;
        }
        size_t end=m;
        i=end-n;
        while(i>start){
            for(size_t j=0;j<n;j++){
                if(s[j]!=this->m[i])
                    break;
                i++;
            }
            if(i==end)
                end-=n;
            else
                break;
            i=end-n;
        }
        CharContent ans=this->subCharContent(start,end-start);
        return ans;
    }

    virtual ~CharContent(){
        delete[] m;
    }
    const size_t npos= static_cast<size_t>(-1);
private:
    char *m;//the pointer
    size_t size;//the size of the content.
    size_t m_length;//actually length
};



bool operator==(const CharContent &lhr,const CharContent &rhr){
    size_t left_size=lhr.length(),right_size=rhr.length();
    if(left_size!=right_size)
        return false;
    for(size_t i=0;i<left_size;i++){
        if(lhr[i]!=rhr[i]){
            return false;
        }
    }
    return true;
}

bool operator==(const CharContent &lhr,const char *rhr){
    return operator==(lhr,CharContent(rhr));
}

struct hashFunc{
    size_t operator()(const CharContent& cc) const{
        return std::_Hash_impl::hash(cc.c_str(), cc.length());
    }
};


struct EqualFunc{
    bool operator()(const CharContent &lhr,const CharContent &rhr)const{
        return lhr==rhr;
    }
};

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

struct Request{
    CharContent method;
    CharContent uri;
    CharContent protocol;
    CharContent Entity;
    unordered_map<CharContent,CharContent,hashFunc,EqualFunc> headers;
};

struct Response{
    CharContent code;
    CharContent protocol;
    CharContent Entity;
    unordered_map<CharContent,CharContent,hashFunc,EqualFunc> headers;
    vector<CharContent> cookies;
};

class HttpRequest{
public:
    HttpRequest(CharContent request):_method({"GET","POST","PUT","DELETE","HEAD","TRACE","CONNECT","OPTIONS"}),
                                     _protocol({"HTTP/1.1","HTTP/1.0"}),
                                     request(request){}
    bool handle_request(Request *ret){
        size_t entity_pos=0;
        if((entity_pos=request.find("\r\n\r\n"))==request.npos){
            return false;
        }
        entity_pos+=4;
        ret->Entity=request.subCharContent(entity_pos,request.length()-entity_pos);
        vector<CharContent> string_list=request.split("\r\n");
        if(string_list.empty())
            return false;
        if(!handle_first_line(string_list[0],ret))
            return false;
        for(size_t i=1;i<string_list.size();i++){
            if(!handle_headers(string_list[i],ret)){
                return false;
            }
        }
        return true;
    }
    virtual ~HttpRequest(){}
private:
    bool handle_first_line(CharContent line,Request *ret){
        vector<CharContent> string_list=line.split(" ");
        if(string_list.size()<3)
            return false;
        CharContent method(""),uri(""),protocol("");
        for(size_t i=0;i<3;i++){
            if(_method.find(string_list[i])!=_method.end()){
                if(method.length()>0)
                    return false;
                method=string_list[i];
            }
            else if(_protocol.find(string_list[i])!=_protocol.end()){
                if(protocol.length()>0)
                    return false;
                protocol=string_list[i];
            }
            else{
                if(uri.length()>0)
                    return false;
                uri=string_list[i];
            }
        }
        ret->method=method;
        ret->uri=uri;
        ret->protocol=protocol;
        return true;
    }
    bool handle_headers(CharContent line,Request *ret){
        size_t pos=0;
        if((pos=line.find(":"))==line.npos){
            return false;
        }
        CharContent key=line.subCharContent(0,pos);
        key=key.strip(" ");
        CharContent value=line.subCharContent(pos+1,line.length()-pos-1);
        value=value.strip(" ");
        if(ret->headers.find(key)!=ret->headers.end()){
            return false;
        }
        ret->headers[key]=value;
        return true;
    }
    unordered_set<CharContent,hashFunc,EqualFunc> _method;
    unordered_set<CharContent,hashFunc,EqualFunc> _protocol;
    CharContent request;
};

class CGI{
public:
    CGI(const CharContent &path,const CharContent &name,int connfd):_path(path),_name(name),_connfd(connfd){}
    bool execv_cgi(Request request,Response *response){
        pid_t pid;
        int output[2];
        int input[2];
        if(pipe(output)<0)
            return false;
        if(pipe(input)<0)
            return false;
        if((pid=fork())<0)
            return false;
        else if(pid==0){
            close(input[1]);
            close(output[0]);
            dup2(output[1],STDOUT_FILENO);
            dup2(input[0],STDIN_FILENO);
            close(input[0]);
            close(output[1]);
            char *exec_name= this->_name.c_str();
            char *exec_path_name[2]={exec_name,NULL};
            unordered_set<CharContent,hashFunc,EqualFunc> cgi_key{
                    "REQUEST_METHOD",
                    "CONTENT_TYPE",
                    "CONTENT_LENGTH",
                    "QUERY_STRING",
                    "SCRIPT_NAME",
                    "PATH_INFO",
                    "PATH_TRANSLATED",
                    "REMOTE_ADDR",
                    "REMOTE_HOST",
                    "GATEWAY_INTERFACE",
                    "SERVER_NAME",
                    "SERVER_PORT",
                    "SERVER_PROTOCOL",
                    "HTTP_ACCEPT",
                    "HTTP_USER_AGENT",
                    "HTTP_REFERER",
                    "HTTP_COOKIE"
            };
            char *env[20]={NULL};
            char env_temp[20][2048]={'\0'};
            size_t env_index=0;
            for(auto iter=cgi_key.begin();iter!=cgi_key.end();++iter){
                CharContent key=*iter;
                CharContent value="";
                value=make_environment(request,key);
                key.append("=");
                key.append(value);
                env[env_index]=env_temp[env_index];
                strncpy(env[env_index],key.c_str(),key.length());
                env[env_index][key.length()]='\0';
                env_index++;
            }
            execve(this->_path.c_str(),exec_path_name,env);
            exit(-1);
        }
        close(input[0]);
        close(output[1]);
        bool get_error=false;
        char *entity_string=request.Entity.c_str();
        int entity_len=request.Entity.length(),entity_step=1024;
        while(entity_len>0){
            if(entity_step>entity_len){
                entity_step=entity_len;
            }
            if(write(input[1],entity_string,entity_step)<0)
                get_error=true;
            entity_len-=entity_step;
            entity_string+=entity_step;
        }
        char buf[1024]={'\0'};
        CharContent ans("");
        Epoll e(1);
        e.make_ctl(EPOLL_CTL_ADD,output[0],EPOLLIN);
        int epoll_fd=e.get_epoll_fd();
        while(true){
            struct epoll_event event;
            int ret=-1;
            ssize_t read_ret=-1;
            ret=epoll_wait(epoll_fd,&event,1,2000);
            if(ret<1){
                if(ret<0)
                    get_error=true;
                break;
            }
            if((read_ret=read(output[0],buf,sizeof(buf)))<0)
                get_error=true;
            if(read_ret==0)
                break;
            if(read_ret<sizeof(buf)){
                ans.append(buf,read_ret);
                break;
            }
            ans.append(buf,read_ret);
            memset(buf,'\0',sizeof(buf));
        }
        e.make_ctl(EPOLL_CTL_DEL,output[0],EPOLLIN);
        int cgi_exec_ret_num=-1;
        waitpid(pid,&cgi_exec_ret_num,0);
        close(input[1]);
        close(output[0]);
        if(get_error)
            return false;
        if(WIFEXITED(cgi_exec_ret_num)==0)
            return false;
        //to do:judge the cgi response and get the response;
        return this->CheckCGI(ans,response);
    }
    virtual ~CGI(){}
private:
    CharContent make_environment(Request request,CharContent cgi_key){
        //to do
        if(cgi_key=="REQUEST_METHOD"){
            return request.method;
        }else if(cgi_key=="CONTENT_TYPE"){
            return request.headers.find("Content-Type")!=request.headers.end()?request.headers["Content-Type"]:"";
        }else if(cgi_key=="CONTENT_LENGTH"){
            return request.headers.find("Content-Length")!=request.headers.end()?request.headers["Content-Length"]:"";
        }else if(cgi_key=="QUERY_STRING"){
            CharContent path=request.uri;
            size_t pos=0;
            if((pos=path.find("?"))==path.npos){
                return "";
            }
            return path.subCharContent(pos+1,path.length()-pos-1);
        }else if(cgi_key=="SCRIPT_NAME"){
            CharContent path=request.uri;
            size_t pos=0;
            if((pos=path.find("?"))==path.npos){
                return path;
            }
            return path.subCharContent(0,pos);
        }else if(cgi_key=="PATH_INFO"){
            return this->_name;
        }else if(cgi_key=="PATH_TRANSLATED"){
            return this->_path;
        }else if(cgi_key=="REMOTE_ADDR"){
            //发送此次请求的主机IP
            sockaddr_in remote_sock;
            socklen_t remote_sock_len=sizeof(remote_sock);
            getpeername(_connfd,(sockaddr *)&remote_sock,&remote_sock_len);
            char client_ip[21]={'\0'};
            inet_ntop(AF_INET, &remote_sock.sin_addr, client_ip, sizeof(client_ip));
            CharContent temp(client_ip);
            return temp;
        }else if(cgi_key=="REMOTE_HOST"){
            //发送此次请求的主机名
            sockaddr_in remote_sock;
            socklen_t remote_sock_len=sizeof(remote_sock);
            getpeername(_connfd,(sockaddr *)&remote_sock,&remote_sock_len);
            char client_ip[21]={'\0'};
            inet_ntop(AF_INET, &remote_sock.sin_addr, client_ip, sizeof(client_ip));
            CharContent temp(client_ip);
            return temp;
        }else if(cgi_key=="GATEWAY_INTERFACE"){
            return CharContent("CGI/1.1");
        }else if(cgi_key=="SERVER_NAME"){
            //服务器主机名、域名或IP
            sockaddr_in local_sock;
            socklen_t local_sock_len=sizeof(local_sock);
            getsockname(_connfd,(sockaddr *)&local_sock,&local_sock_len);
            char server_ip[21]={'\0'};
            inet_ntop(AF_INET, &local_sock.sin_addr, server_ip, sizeof(server_ip));
            CharContent temp(server_ip);
            return temp;
        }else if(cgi_key=="SERVER_PORT"){
            //服务器端口号
            sockaddr_in local_sock;
            socklen_t local_sock_len=sizeof(local_sock);
            getsockname(_connfd,(sockaddr *)&local_sock,&local_sock_len);
            char server_ip[21]={'\0'};
            inet_ntop(AF_INET, &local_sock.sin_addr, server_ip, sizeof(server_ip));
            char server_port[7]={'\0'};
            snprintf(server_port,sizeof(server_port),"%d",ntohs(local_sock.sin_port));
            CharContent temp(server_port);
            return temp;
        }else if(cgi_key=="SERVER_PROTOCOL"){
            return CharContent("HTTP/1.1");
        }else if(cgi_key=="HTTP_ACCEPT"){
            return request.headers.find("Content-Type")!=request.headers.end()?request.headers["Content-Type"]:"";
        }else if(cgi_key=="HTTP_USER_AGENT"){
            return request.headers.find("User-Agent")!=request.headers.end()?request.headers["User-Agent"]:"";
        }else if(cgi_key=="HTTP_REFERER"){
            return request.headers.find("Referer")!=request.headers.end()?request.headers["Referer"]:"";
        }else if(cgi_key=="HTTP_COOKIE"){
            return request.headers.find("Cookie")!=request.headers.end()?request.headers["Cookie"]:"";
        }
        return CharContent("");
    }
    bool CheckCGI(CharContent cgi_string,Response *response){
        if(cgi_string.length()==0)
            return false;
        bool one_line_space=false;
        vector<CharContent> headers_line;
        size_t pos=0,start=0;
        while((pos=cgi_string.find("\n",1,start))!=cgi_string.npos){
            CharContent temp=cgi_string.subCharContent(start,pos-start);
            temp=temp.strip("\r");
            if(temp.length()==0){
                one_line_space=true;
                break;
            }
            headers_line.push_back(temp);
            start=pos+1;
        }
        if(!one_line_space)
            return false;
        response->Entity=cgi_string.subCharContent(pos+1,cgi_string.length()-pos-1);
        return this->check_header(headers_line,response);
    }
    bool check_header(vector<CharContent> header_list,Response *response){
        size_t start_index=0;
        if(header_list.empty())
            return false;
        if(this->check_first_line(header_list[0],response))
            start_index=1;
        size_t n=header_list.size();
        size_t pos=0;
        for(size_t i=start_index;i<n;++i){
            if((pos=header_list[i].find(":"))==header_list[i].npos)
                return false;
            CharContent response_key=header_list[i].subCharContent(0,pos);
            response_key=response_key.strip(" ");
            CharContent response_value=header_list[i].subCharContent(pos+1,header_list[i].length()-pos-1);
            response_value=response_value.strip(" ");
            if(response_key=="Set-Cookie")
                response->cookies.push_back(response_value);
            if(response->headers.find(response_key)==response->headers.end()){
                response->headers[response_key]=response_value;
            }else{
                return false;
            }
        }
        if(response->headers.find("Content-Type")==response->headers.end())
            return false;
        return true;
    }
    bool check_first_line(CharContent first_line,Response *response){
        vector<CharContent> string_list;
        string_list=first_line.split(" ");
        if(string_list.size()<3)
            return false;
        if(string_list[0].find("HTTP")==string_list[0].npos)
            return false;
        response->protocol=string_list[0];
        response->code=string_list[1];
        return true;
    }
    CharContent _path;
    CharContent _name;
    int _connfd;
};

/*
 *read_type
 * 0 exec
 * 1 file
 *
 */
struct map_item{
    map_item()= default;
    map_item(CharContent uri,CharContent file_path,int read_type):
            uri(uri),file_path(file_path),read_type(read_type){}
    CharContent uri;
    CharContent file_path;
    int read_type;
    virtual ~map_item(){}
};

class source{
public:
    source(int _connfd):_conndfd(_connfd),
                        url_maps({
                                         map_item("/static","~/ServerContent",1),
                                         map_item("/","~/ServerContent/cgi",0)
                                 }){}
    /*
     * 0 ok
     * 1 not find resource
     * 2 file type unknow(not file or document)
     * 3 exec error
     */
    int resource_find(Request request,Response *response){
        size_t i=0;
        size_t pos=0;
        pos=request.uri.find("?");
        CharContent true_uri=request.uri.subCharContent(0,pos);
        size_t np=0;
        for(;i<this->url_maps.size();i++){
            np=0;
            if((np=true_uri.find(this->url_maps[i].uri))==0){
                break;
            }
        }
        if(i==this->url_maps.size())
            return 1;
        CharContent file_path=this->url_maps[i].file_path;
        true_uri=true_uri.subCharContent(this->url_maps[i].uri.length(),true_uri.length());
        file_path.append(true_uri);
        if(this->url_maps[i].read_type==0){
            if(!this->exec_cgi(request,response,file_path))
                return 3;
        }
        else if(this->url_maps[i].read_type==1){
            if(!this->judge_file(request,response,file_path))
                return 2;
        }
        else{
            return 1;
        }
        return 0;
    }
private:
    bool exec_cgi(Request request,Response *response,CharContent file_path){
        vector<CharContent> string_list=file_path.split("/");
        if(string_list.empty())
            return false;
        CGI cgi(file_path,string_list[string_list.size()-1],this->_conndfd);
        return cgi.execv_cgi(request,response);
    }
    bool judge_file(Request request,Response *response,CharContent file_path){
        struct stat file_stat;
        if(stat(file_path.c_str(),&file_stat)<0)
            return false;
        if(S_ISDIR(file_stat.st_mode)){
            return this->document_iter(request.uri,response,file_path);
        }
        else if(S_ISREG(file_stat.st_mode)){
            char etag_buf[200]={'\0'};
            snprintf(etag_buf,sizeof(etag_buf),"%lx-%lx",file_stat.st_size,file_stat.st_mtim.tv_sec);
            response->headers["ETag"]=CharContent(etag_buf);
            response->Entity=CharContent(file_stat.st_size);
            return this->file_iter(response,file_path);
        }
        return false;
    }
    bool document_iter(CharContent uri,Response *response,CharContent file_path){
        size_t pos=0;
        pos=uri.find("?");
        CharContent true_uri=uri.subCharContent(0,pos);
        DIR *dir_ptr=opendir(file_path.c_str());
        struct dirent *pStResult = NULL;
        struct dirent pStEntry;
        bool get_in=false;
        CharContent res_cont;
        res_cont.append("<html><head>file index</head><body>");
        while(readdir_r(dir_ptr,&pStEntry,&pStResult)>=0&&pStResult!=NULL){
            if(pStEntry.d_name[0]=='.')
                continue;
            get_in=true;
            res_cont.append("<p><a href=\'");
            res_cont.append(true_uri);
            res_cont.append("/");
            res_cont.append(pStEntry.d_name);
            res_cont.append("\'>");
            res_cont.append(pStEntry.d_name);
            res_cont.append("</a></p>");
        }
        if(!get_in)
            res_cont.append("<h1>no content</h1>");
        res_cont.append("</body></html>");
        response->Entity=res_cont;
        closedir(dir_ptr);
        return true;
    }
    bool file_iter(Response *response,CharContent file_path){
        int file_fd=open(file_path.c_str(),O_RDONLY,0666);
        if(file_fd<0)
            return false;
        ssize_t read_size=-1;
        read_size=read(file_fd,response->Entity.c_str(),response->Entity.length());
        if(read_size<0)
            return false;
        return true;
    }
    int _conndfd;
    vector<map_item> url_maps;
};

class HttpResponse{
public:
    HttpResponse():response_code_map({//come from RFC2616
                                             //1xx
                                             {"100","Continue"},
                                             {"101","Switching Protocols"},
                                             //2xx
                                             {"200","OK"},
                                             {"201","Created"},
                                             {"202","Accepted"},
                                             {"203","Non-Authoritative Information"},
                                             {"204","No Content"},
                                             {"205","Reset Content"},
                                             {"206","Partial Content"},
                                             //3xx
                                             {"300","Multiple Choices"},
                                             {"301","Moved Permanently"},
                                             {"302","Found"},
                                             {"303","See Other"},
                                             {"304","Not Modified"},
                                             {"305","Use Proxy"},
                                             {"307","Temporary Redirect"},
                                             //4xx
                                             {"400","Bad Request"},
                                             {"401","Unauthorized"},
                                             {"402","Payment Required"},
                                             {"403","Forbidden"},
                                             {"404","Not Found"},
                                             {"405","Method Not Allowed"},
                                             {"406","Not Acceptable"},
                                             {"407","Proxy Authentication Required"},
                                             {"408","Request Timeout"},
                                             {"409","Conflict"},
                                             {"410","Gone"},
                                             {"411","Length Required"},
                                             {"412","Precondition Failed"},
                                             {"413","Request Entity Too Large"},
                                             {"414","Request-URI Too Long"},
                                             {"415","Unsupported Media Type"},
                                             {"416","Requested Range Not Satisfiable"},
                                             {"417","Expectation Failed"},
                                             //5xx
                                             {"500","Internal Server Error"},
                                             {"501","Not Implemented"},
                                             {"502","Bad Gateway"},
                                             {"503","Service Unavailable"},
                                             {"504","Gateway Timeout"},
                                             {"505","HTTP Version Not Supported"}
                                     }){}
    CharContent response_content(CharContent code){
        Response response;
        response.code=code;
        response.protocol="HTTP/1.1";
        response.Entity="<html><head><title>";
        response.Entity.append(code);
        response.Entity.append("</title></head><body><h1>");
        response.Entity.append(this->response_code_map[code]);
        response.Entity.append("</h1></body></html>");
        response.headers["Content-Type"]="text/html; charset=utf-8";
        return this->response_content(&response,NULL);
    }
    CharContent response_content(Response *response,Request *request){
        CharContent res_content=this->make_header(response);
        if(request!=NULL&&request->method=="HEAD"){
            return res_content;
        }
        res_content.append(response->Entity);
        return res_content;
    }
    virtual ~HttpResponse(){}
private:
    CharContent make_header(Response *response){
        if(response->code.length()==0){
            response->code="200";
            response->protocol="HTTP/1.1";
        }
        CharContent ret_header;
        //first line;
        ret_header.append(response->protocol);
        ret_header.append(" ");
        ret_header.append(response->code);
        ret_header.append(" ");
        ret_header.append(this->response_code_map[response->code]);
        ret_header.append("\r\n");

        if(response->headers.find("Date")==response->headers.end()){
            time_t rawtime;
            struct tm timeinfo;
            time ( &rawtime );
            localtime_r(&rawtime,&timeinfo);
            char buf[200]={'\0'};
            asctime_r(&timeinfo,buf);
            CharContent temp(buf);
            temp=temp.strip("\n");
            response->headers["Date"]=temp;
        }

        response->headers["Server"]="ToyServer/beta 0.1";
        response->headers["Connection"]="close";

        if(response->headers.find("Content-Length")==response->headers.end()){
            char buf[200]={'\0'};
            snprintf(buf,sizeof(buf),"%lu",response->Entity.length());
            response->headers["Content-Length"]=buf;
        }

        for(auto iter=response->headers.begin();iter!=response->headers.end();++iter){
            ret_header.append(iter->first);
            ret_header.append(": ");
            ret_header.append(iter->second);
            ret_header.append("\r\n");
        }
        for(size_t i=0;i<response->cookies.size();i++){
            ret_header.append("Set-Cookie: ");
            ret_header.append(response->cookies[i]);
            ret_header.append("\r\n");
        }
        ret_header.append("\r\n");
        return ret_header;
    }
    unordered_map<CharContent,CharContent,hashFunc,EqualFunc>
            response_code_map;
};

void log(const char *buf){
    size_t n=strlen(buf);
    write(STDOUT_FILENO,buf,n);
    return;
}

void *response_to_client(const CharContent &content,int connfd){
    int step=1024;
    int left_size=content.length();
    const char *content_ptr=content.c_str();
    Epoll e(1);
    e.make_ctl(EPOLL_CTL_ADD,connfd,EPOLLOUT);
    int epoll_fd=e.get_epoll_fd();
    epoll_event event_ret;
    int ret_num=-1;
    while(left_size>0){
        ret_num=epoll_wait(epoll_fd,&event_ret,1,15000);
        if(ret_num<1){
            break;
        }
        int ret_fd=event_ret.data.fd;
        if(ret_fd==connfd&&event_ret.events&EPOLLOUT){
            if(left_size<step){
                step=left_size;
            }
            ssize_t write_ret_num=-1;
            write_ret_num=write(connfd,content_ptr,step);
            if(write_ret_num<=0){
                break;
            }
            content_ptr+=step;
            left_size-=step;
        }
    }
    close(connfd);
    return NULL;
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
        ret_num=epoll_wait(epoll_fd,&event_ret,1,15000);
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
        snprintf(v_log,sizeof(v_log),"read_size %lu\n",read_size);
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

int main() {
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
