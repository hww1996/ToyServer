#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include<unordered_set>
#include"CharContent.h"
#include"HttpStruct.h"


class HttpRequest{
public:
    HttpRequest(CharContent request):_method({"GET","POST","PUT","DELETE","HEAD","TRACE","CONNECT","OPTIONS"}),
                                     _protocol({"HTTP/1.1","HTTP/1.0"}),
                                     request(request){}
    bool handle_request(Request *ret);
    virtual ~HttpRequest(){}
private:
    bool handle_first_line(CharContent line,Request *ret);
    bool handle_headers(CharContent line,Request *ret);
    std::unordered_set<CharContent,hashFunc,EqualFunc> _method;
    std::unordered_set<CharContent,hashFunc,EqualFunc> _protocol;
    CharContent request;
};

#endif