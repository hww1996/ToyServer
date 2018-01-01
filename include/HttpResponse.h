#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__

#include"CharContent.h"
#include"HttpStruct.h"

#include<unordered_map>

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
    CharContent response_content(CharContent code);
    CharContent response_content(Response *response,Request *request);
    virtual ~HttpResponse(){}
private:
    CharContent make_header(Response *response);
    std::unordered_map<CharContent,CharContent,hashFunc,EqualFunc>
            response_code_map;
};

#endif