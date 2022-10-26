#ifndef CONN_H
#define CONN_H

#include "user.h"
#include "../pack.h"

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
    int tofd;
    sockaddr_in address;
    user* m_user;
    char readBuf[BUFSIZE];
    char writeBuf[BUFSIZE];
    string readCache;
    string writeCache;
    Pack* package;
    int readIndex;
    int writeIndex;
    int bytesToSend;
public:
    static int epollfd;
    static int conn_count;
    static unordered_map<string,conn*> online;

    string get_username();
    void init(int fd,sockaddr_in* caddr);
    void init();
    bool login(string name,string password);
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
int conn::epollfd = -1;
unordered_map<string,conn*> conn::online;

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
    m_user = nullptr;
    addEvent(epollfd,_fd);
    init();
}
void conn::init(){
    tofd = -1;
    bzero(readBuf,BUFSIZE);
    bzero(writeBuf,BUFSIZE);
    readIndex = 0;
    writeIndex = 0;
    bytesToSend = 0;
    status = FREE;
    readCache.clear();
    writeCache.clear();
    if(package){
        delete package;
        package = nullptr;
    }
}
void conn::close(){
    if(m_user){
        online.erase(online.find(m_user->get_name()));
    }
    if(fd != -1){
        removeEvent(epollfd,fd);
        fd = -1;
        conn_count--;
    }
}
bool conn::login(string username,string password){
    if(user::users.find(username) != user::users.end()){
        if(user::users[username]->get_password() == password){
            m_user = user::users[username];
            online[username] = this;
            return true;
        }
    }
    return false;
}
string conn::get_username(){
    if(m_user){
        return m_user->get_name();
    }else{
        return NULL;
    }
}
bool conn::read(){
    while(true){
        int ret = ::read(fd,readBuf,sizeof(readBuf));
        if(ret == -1){
            //没有后续数据送达
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;
            return false;
        }else if(ret == 0){
            return false;
        }
        readCache += readBuf;
    }
    return true;
}
bool conn::write(){
    while(true){
        if(writeIndex + sizeof(writeBuf) > writeCache.size()){
            memcpy(writeBuf,writeCache.substr(writeIndex).c_str(),writeCache.size()-writeIndex);
            bytesToSend = writeCache.size()-writeIndex;
        }else{
            memcpy(writeBuf,writeCache.substr(writeIndex,sizeof(writeBuf)+writeIndex).c_str(),sizeof(writeBuf));
            bytesToSend = BUFSIZE;
        }
        
        int ret = ::write(tofd,writeBuf,bytesToSend);
        if(ret == -1){
            if(errno == EAGAIN){
                modEvent(epollfd,fd,EPOLLOUT);
                return true;
            }
            return false;
        }
        writeIndex += bytesToSend;
        if(writeIndex >= writeCache.size()){
            modEvent(epollfd,fd,EPOLLIN);
            init();
            return true;
        }
    }
}

void conn::process(){
    if(!process_read()) close();
    if(!process_write()) close();
}
bool conn::process_read(){
    try{
        string p = readCache.substr(readCache.find_first_of("{"),readCache.find_first_of("}")+1);
        readCache = readCache.substr(readCache.find_first_of("}")+1);
        package = new Pack(p);
        switch(package->get_type()){
            case MSG : {
                if(m_user && m_user->get_name() == package->get_from()){
                    string username = package->get_to();
                    if(online.find(username) == online.end()){
                        package->get_type() = ERR;
                        package->get_to() = package->get_from();
                        package->get_content() = "User Not OnLine or User Not Exist.";
                        package->get_from() = "Server";
                        package->get_size() = package->get_content().size();
                    }else{
                        return true;
                    }
                }
                else{
                    package->get_type() = ERR;
                    package->get_to() = "UnKnown";
                    package->get_content() = "User Not Login or Fake Username.";
                    package->get_from() = "Server";
                    package->get_size() = package->get_content().size();
                }
                break;
            }
            case LOGIN : {
                if(login(package->get_from(),package->get_content())){
                    package->get_content() = "Successfully Login. Enjoy!";
                    package->get_size() = package->get_content().size();
                    package->get_to() = package->get_from();
                    package->get_from() = "Server";
                }else{
                    package->get_type() == ERR;
                    package->get_content() = "Wrong Username or Password.";
                    package->get_size() = package->get_content().size();
                    package->get_to() = "UnKnwon";
                    package->get_from() = "Server";
                }
                break;
            }
            case GETONLINE : {
                cout << "--获取在线列表的用户:"  << m_user->get_name() << "--" << endl;
                if(m_user && m_user->get_name() == package->get_from()){
                    package->get_content().clear();
                    for(auto x : online){
                        package->get_content() += x.first;
                        package->get_content() += "\\";
                    }
                    package->get_to() = package->get_from();
                    package->get_from() = "Server";
                    package->get_size() = package->get_content().size();
                }
                else{
                    package->get_type() = ERR;
                    package->get_to() = "UnKnown";
                    package->get_content() = "User Not Login or Fake Username.";
                    package->get_from() = "Server";
                    package->get_size() = package->get_content().size();
                }
                break;
            }
        }
    }catch(...){
        cout << "wrong" << endl;
        exit(-1);
    }
    return true;
}
bool conn::process_write(){
    if(package->get_from() == "Server"){
        tofd = fd;
    }else{
        if(online.find(package->get_to()) == online.end()) return false;
        tofd = online[package->get_to()]->fd;
    }
    writeCache = package->Dump();
    modEvent(epollfd,fd,EPOLLOUT);
    return true;
}

int conn::get_fd(){return fd;}
sockaddr_in conn::get_address(){return address;}
user* conn::get_user(){return m_user;}


#endif