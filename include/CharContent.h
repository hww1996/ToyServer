#ifndef __CHARCONTENT_H__
#define __CHARCONTENT_H__

#include<string>
#include<cstring>
#include<cstdlib>
#include<vector>



class CharContent{
public:
    CharContent(){
        m=new char [10];
        size=0;
        m_length=10;
        memset(m,'\0',m_length);
    }
    CharContent(const CharContent &cc){
        size_t n=cc.m_length;
        m=new char [n];
        size=cc.size;
        m_length=cc.m_length;
        memset(m,'\0',m_length);
        strncpy(m,cc.m,cc.m_length);
    }
    CharContent(const char *cc,size_t n){
        if(n==0){
            m=new char [10];
            size=0;
            m_length=10;
            memset(m,'\0',m_length);
            return;
        }else{
            m_length=2*n;
            m=new char [m_length];
            size=n;
            memset(m,'\0',m_length);
            for(size_t i=0;i<n;i++){
                m[i]=cc[i];
            }
            return;
        }
    }
    CharContent(const char *cc){
        size_t n=strlen(cc);
        if(n==0){
            m=new char [10];
            size=0;
            m_length=10;
            memset(m,'\0',m_length);
            return;
        }else{
            m_length=2*n;
            m=new char [m_length];
            size=n;
            memset(m,'\0',m_length);
            for(size_t i=0;i<n;i++){
                m[i]=cc[i];
            }
            return;
        }
    }
    CharContent(size_t n){
        m=new char[n+10];
        size=n;
        m_length=n+10;
        memset(m,'\0',(n+10)*sizeof(char));
    }
    CharContent &operator=(const CharContent &cc);

    CharContent &operator=(const char *s);
    const char &operator[](size_t pos) const;
    char &operator[](size_t pos);
    const char *c_str() const;
    char *c_str();
    size_t length() const;
    size_t  capacity()const;
    void clear();
    void append(const char *s,size_t n);
    void append(const CharContent &cc);
    void append(const char *s);

    CharContent subCharContent(size_t pos,size_t length);

    size_t find(const char *s,size_t n,size_t pos=0);

    size_t find(const CharContent &cc,size_t pos=0);

    std::vector<CharContent> split(const CharContent &s);

    CharContent strip(const CharContent &s);

    virtual ~CharContent(){
        delete[] m;
    }
    const size_t npos= static_cast<size_t>(-1);
private:
    char *m;//the pointer
    size_t size;//the size of the content.
    size_t m_length;//actually length
};



bool operator==(const CharContent &lhr,const CharContent &rhr);

bool operator==(const CharContent &lhr,const char *rhr);


struct hashFunc{
    size_t operator()(const CharContent& cc) const;
};


struct EqualFunc{
    bool operator()(const CharContent &lhr,const CharContent &rhr)const;
};


#endif
