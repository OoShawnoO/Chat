#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include "../pack.h"
#include <memory.h>
#include <thread>

const char* address = "127.0.0.1";
const short port = 9999;
using namespace std;

class client{
public:
    client(string _name,string _password);
    ~client();
    bool connect();
    bool login();
    bool getonline();
    bool message(string to,string content,TYPE type);
    bool readmsg();
    string& get_name();
    string& get_password();
    Pack& get_pack();
    bool& get_done();
    bool& get_logined();
    static void READ(client&);
private:
    int fd;
    string name;
    string password;
    Pack pack;
    char buffer[1024];
    bool done;
    bool logined;
    thread* t;
};

client::client(string _name,string _password):
name(_name),password(_password),pack(Pack(STANDARD,name,"Server",password,LOGIN))
{
    logined = false;
    t = new thread(READ,ref(*this));
    fd = -1;
    bzero(buffer,sizeof(buffer));
}
client::~client(){
    if(t){
        delete t;
    }
    close(fd);
}
bool client::connect(){
    fd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET,address,&serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(port);
    int ret = ::connect(fd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
    if(ret==-1){
        perror("connect :");
        return false;
    }
    
    return true;
}
bool client::login(){
    pack.get_from() = name;
    pack.get_to() = "Server";
    pack.get_type() = LOGIN;
    pack.get_content() = password;
    pack.get_size() = password.size();
    int ret = write(fd,pack.Dump().c_str(),pack.Dump().size());
    if(ret == -1){
        perror("login write");
        return false;
    }
    sleep(1);
    if(logined != true) return false;
    return true;
}

bool client::getonline(){
    if(logined == false) return false;
    pack.get_from() = name;
    pack.get_to() = "Server";
    pack.get_type() = GETONLINE;
    pack.get_content().clear();
    pack.get_size() = 0;
    int ret = write(fd,pack.Dump().c_str(),pack.Dump().size());
    if(ret == -1){
        perror("getonline write");
        return false;
    }
    return true;
}

bool client::message(string to,string content,TYPE type){
    if(logined == false) return false;
    done = false;
    pack.get_from() = name;
    pack.get_to() = to;
    pack.get_type() = type;
    pack.get_content() = content;
    pack.get_size() = content.size();
    int writeindex = 0;
    string x = pack.Dump();
    while(writeindex < x.size()){
        int ret = write(fd,x.substr(writeindex).c_str(),x.substr(writeindex).size());
        if(ret == -1){
            perror("message write");
            exit(-1);
        }
        writeindex += ret;
    }
    
    if(type != ACK){
        while(done != true && pack.get_type() != ERR){
        }
        if(pack.get_content() == "ok" && pack.get_type() == ACK){
           cout << "发送成功" <<endl;
           return true;
        }else{
            return false;
        }
    }
    return true;
}

bool client::readmsg(){
    string readCache;
    while(true){
        bzero(buffer,sizeof(buffer));
        int ret = read(fd,buffer,sizeof(buffer));
        if(ret == -1){
            perror("message read");
            return false;
        }else if(ret == 0){
            close(fd);
            exit(-1);
        }
        readCache += buffer;
        if(readCache.find("{") != string::npos && readCache.find("}")!=string::npos) break;
    }
    pack.Load(readCache);
    return true;
}

void client::READ(client& clt){
    while(1){
        clt.readmsg();
        // cout << clt.get_pack().Dump() <<endl;
        TYPE types = clt.get_pack().get_type();
        if(types == MSG){
            cout << clt.pack.get_from() << "发送信息:" << clt.pack.get_content() << endl;
            Pack reply(clt.get_pack());
            reply.get_to() = reply.get_from();
            reply.get_content() = "ok";
            reply.get_size() = sizeof(reply.get_content());
            clt.message(reply.get_to(),reply.get_content(),ACK);
        }
        else if(types == ACK){
            clt.get_done() = true;
        }
        else if(types == GETONLINE){
            cout << "在线列表:" << clt.pack.get_content() <<endl;
        }else if(types == ERR){
            cout << clt.pack.get_content() << endl;
        }else if(types == LOGIN){
            clt.logined = true;
        }
    }
}

string& client::get_name(){
    return name;
}

string& client::get_password(){
    return password;
}

Pack& client::get_pack(){
    return pack;
}

bool& client::get_done(){
    return done;
}

bool& client::get_logined(){
    return logined;
}

#endif