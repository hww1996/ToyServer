#include"HttpResponse.h"
#include"CharContent.h"
#include <time.h>


CharContent HttpResponse::response_content(CharContent code){
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
CharContent HttpResponse::response_content(Response *response,Request *request){
	CharContent res_content=this->make_header(response);
	if(request!=NULL&&request->method=="HEAD"){
		return res_content;
	}
	res_content.append(response->Entity);
	return res_content;
}
CharContent HttpResponse::make_header(Response *response){
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