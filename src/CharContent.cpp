#include<stdexcept>
#include<string>
#include<cstring>
#include<cstdlib>
#include<vector>

#include"CharContent.h"
using namespace std;


CharContent &CharContent::operator=(const CharContent &cc){
	delete[] m;
	size=cc.size;
	m_length=cc.m_length;
	m=new char [m_length];
	memset(m,'\0',m_length);
	strncpy(m,cc.m,size);
	return *this;
}

CharContent &CharContent::operator=(const char *s){
	return this->operator=(CharContent(s));
}

const char &CharContent::operator[](size_t pos) const{
	if(pos>=size)
		throw std::out_of_range("CharContent get element out of range.");
	return *(m+pos);
}
char &CharContent::operator[](size_t pos){
	if(pos>=size)
		throw std::out_of_range("CharContent get element out of range.");
	return *(m+pos);
}
const char *CharContent::c_str() const{
	return this->m;
}
char *CharContent::c_str(){
	return this->m;
}
size_t CharContent::length() const{
	return this->size;
}
size_t  CharContent::capacity()const{
	return this->m_length;
}
void CharContent::clear(){
	this->size=0;
}
void CharContent::append(const char *s,size_t n){
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
void CharContent::append(const CharContent &cc){
	return this->append(cc.m,cc.size);
}
void CharContent::append(const char *s){
	return this->append(s,strlen(s));
}

CharContent CharContent::subCharContent(size_t pos,size_t length){
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

size_t CharContent::find(const char *s,size_t n,size_t pos){//can use KMP
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

size_t CharContent::find(const CharContent &cc,size_t pos){
	return this->find(cc.m,cc.size,pos);
}

vector<CharContent> CharContent::split(const CharContent &s){
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

CharContent CharContent::strip(const CharContent &s){
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

size_t hashFunc::operator()(const CharContent& cc) const{
    return std::_Hash_impl::hash(cc.c_str(), cc.length());
}


bool EqualFunc::operator()(const CharContent &lhr,const CharContent &rhr)const{
    return lhr==rhr;
}

