#ifndef __SOURCE_H__
#define __SOURCE_H__

#include"CharContent.h"
#include"HttpStruct.h"
#include<vector>



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
    int resource_find(Request request,Response *response);
	virtual ~source(){}
private:
    bool exec_cgi(Request request,Response *response,CharContent file_path);
    bool judge_file(Request request,Response *response,CharContent file_path);
    bool document_iter(CharContent uri,Response *response,CharContent file_path);
    bool file_iter(Response *response,CharContent file_path);
    int _conndfd;
    std::vector<map_item> url_maps;
};


#endif
