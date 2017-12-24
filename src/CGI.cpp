#include <signal.h>
#include <wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <string>
#include <cstdlib>
#include <vector>

#include"CGI.h"
#include"HttpStruct.h"
#include"CharContent.h"
#include"Epoll.h"

using namespace std;

bool CGI::execv_cgi(Request request,Response *response){
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
        	close(output[1]);
        	close(input[0]);
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
	kill(pid,SIGUSR1);
	waitpid(pid,NULL,0);
	close(input[1]);
	close(output[0]);
	if(get_error)
		return false;
	//to do:judge the cgi response and get the response;
	return this->CheckCGI(ans,response);
}
CharContent CGI::make_environment(Request request,CharContent cgi_key){
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
bool CGI::CheckCGI(CharContent cgi_string,Response *response){
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
bool CGI::check_header(vector<CharContent> header_list,Response *response){
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
bool CGI::check_first_line(CharContent first_line,Response *response){
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
