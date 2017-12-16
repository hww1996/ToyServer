#include"source.h"
#include"CharContent.h"
#include"HttpStruct.h"
#include"CGI.h"

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<unistd.h>
#include<vector>

using namespace std;

int source::resource_find(Request request,Response *response){
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

bool source::exec_cgi(Request request,Response *response,CharContent file_path){
	vector<CharContent> string_list=file_path.split("/");
	if(string_list.empty())
		return false;
	CGI cgi(file_path,string_list[string_list.size()-1],this->_conndfd);
	return cgi.execv_cgi(request,response);
}
bool source::judge_file(Request request,Response *response,CharContent file_path){
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
bool source::document_iter(CharContent uri,Response *response,CharContent file_path){
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
bool source::file_iter(Response *response,CharContent file_path){
	int file_fd=open(file_path.c_str(),O_RDONLY,0666);
	if(file_fd<0)
		return false;
	ssize_t read_size=-1;
	read_size=read(file_fd,response->Entity.c_str(),response->Entity.length());
	if(read_size<0)
		return false;
	return true;
}
