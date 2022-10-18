#ifndef CONN_H
#define CONN_H

#include "user.h"
#include "pack.h"

extern conn* conns;

const int BUFSIZE = 2048;

class conn{
private:
    enum STATUS{
        FREE = 0,
        READ,
        WRITE
    };
    STATUS status;
    int fd;
    sockaddr_in address;
    user* user;
    char readBuf[BUFSIZE];
    char writeBuf[BUFSIZE];
    string buffer;
    int readIndex;
    int writeIndex;
    int bytesToSend;
public:
    static int epollfd;
    static int conn_count;

    string get_username();
    void init(int fd,sockaddr_in* caddr);
    void init();
    int login(string name,string password);
    void close();
    bool read();
    bool write();

    void process();
    bool process_read();
    bool process_write();

    int get_fd();
    sockaddr_in get_address();
    ::user* get_user();
};

int conn::conn_count = 0;

void nonblock(int fd){
    int old_flag = fcntl(fd,F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_flag);
}

void addEvent(int epollfd,int fd,bool oneshot = false){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;
    if(oneshot){
        event.events | EPOLLONESHOT;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    nonblock(fd);
}
void removeEvent(int epollfd,int fd){
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
    close(fd);
}
void modEvent(int epollfd,int fd,int ev){
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLRDHUP | EPOLLONESHOT;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
}

void conn::init(int _fd,sockaddr_in* caddr){
    fd = _fd;
    address = *caddr;
    conn_count++;
    init();
}
void conn::init(){
    bzero(readBuf,BUFSIZE);
    bzero(writeBuf,BUFSIZE);
    readIndex = 0;
    writeIndex = 0;
    bytesToSend = 0;
    status = FREE;
    buffer.clear();
}
void conn::close(){
    if(fd != -1){
        removeEvent(epollfd,fd);
        fd = -1;
        conn_count--;
    }
}
string conn::get_username(){
    if(user){
        return user->get_name();
    }else{
        return NULL;
    }
}
bool conn::read(){
    while(true){
        int ret = ::read(fd,readBuf,sizeof(readBuf));
        if(ret == -1){
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;
            return false;
        }else if(ret == 0){
            return false;
        }
        buffer += readBuf;
    }
    return true;
}
bool conn::write(){
    
}

void conn::process(){
    if(!process_read()) close();
    if(!process_write()) close();
}
bool conn::process_read(){
    Pack json(buffer);
    string to = json.get_to();
    if(json.get_type() == MSG){
        if(conns == nullptr) return false;
        int i=0;
        for(;i<max_users;i++){
            if(conns[i].get_username() == to){
                break;
            }
        }
        if( i == max_users) return false;
        else{
            buffer = json.get_content();
        }
    }
}
bool conn::process_write(){

}

int conn::get_fd(){return fd;}
sockaddr_in conn::get_address(){return address;}
user* conn::get_user(){return user;}


#endif