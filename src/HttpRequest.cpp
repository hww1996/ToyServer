#include<vector>
#include<unordered_set>

#include"CharContent.h"
#include"HttpStruct.h"
#include"HttpRequest.h"

bool HttpRequest::handle_request(Request *ret){
	size_t entity_pos=0;
	if((entity_pos=request.find("\r\n\r\n"))==request.npos){
		return false;
	}
	entity_pos+=4;
	ret->Entity=request.subCharContent(entity_pos,request.length()-entity_pos);
	std::vector<CharContent> string_list=request.split("\r\n");
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

bool HttpRequest::handle_first_line(CharContent line,Request *ret){
	std::vector<CharContent> string_list=line.split(" ");
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
bool HttpRequest::handle_headers(CharContent line,Request *ret){
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
