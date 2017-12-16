#ifndef __HTTPSTRUCT_H__
#define __HTTPSTRUCT_H__
#include"CharContent.h"
#include<unordered_map>
#include<vector>
using namespace std;

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

#endif