#ifndef __CGI_H__
#define __CGI_H__

#include"HttpStruct.h"
#include"CharContent.h"

class CGI{
public:
    CGI(const CharContent &path,const CharContent &name,int connfd):_path(path),_name(name),_connfd(connfd){}
    bool execv_cgi(Request request,Response *response);
    virtual ~CGI(){}
private:
    CharContent make_environment(Request request,CharContent cgi_key);
    bool CheckCGI(CharContent cgi_string,Response *response);
    bool check_first_line(CharContent first_line,Response *response);
    bool check_header(std::vector<CharContent> header_list,Response *response);
    CharContent _path;
    CharContent _name;
    int _connfd;
};

#endif
