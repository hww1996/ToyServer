object=CharContent.o HttpRequest.o CGI.o source.o HttpResponse.o net.o main.o

CC=g++

FLAG=-std=c++11 -pthread -I include 

vpath %.cpp src
vpath %.h include

ToyServer:$(object)
	$(CC) -o ToyServer $(object) $(FLAG)

main.o:main.cpp ThreadPool.h net.h
	$(CC) -o main.o -c main.cpp $(FLAG)

CharContent.o:CharContent.cpp CharContent.h
	$(CC) -o CharContent.o -c src/CharContent.cpp $(FLAG)

HttpRequest.o:HttpRequest.cpp CharContent.h HttpStruct.h HttpRequest.h
	$(CC) -o HttpRequest.o -c src/HttpRequest.cpp $(FLAG)

CGI.o:CGI.cpp CGI.h HttpStruct.h CharContent.h Epoll.h
	$(CC) -o CGI.o -c src/CGI.cpp $(FLAG)

source.o:source.cpp source.h CharContent.h HttpStruct.h CGI.h
	$(CC) -o source.o -c src/source.cpp $(FLAG)

HttpResponse.o:HttpResponse.cpp HttpResponse.h CharContent.h
	$(CC) -o HttpResponse.o -c src/HttpResponse.cpp $(FLAG)

net.o:net.cpp net.h CharContent.h HttpStruct.h HttpResponse.h Epoll.h HttpRequest.h source.h
	$(CC) -o net.o -c src/net.cpp $(FLAG)

.PHONY:clean cleanTarget
cleanTarget:
	rm ToyServer
clean:
	rm $(object)
