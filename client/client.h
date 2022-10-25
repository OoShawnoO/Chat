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

private:
    int fd;
    string name;
    string password;
    Pack pack;
    char buffer[1024];
};

client::client(string _name,string _password):
name(_name),password(_password),pack(Pack(STANDARD,name,"Server",password,LOGIN))
{
    fd = -1;
    bzero(buffer,sizeof(buffer));
}
client::~client(){
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
    write(fd,pack.Dump().c_str(),pack.Dump().size());
    int ret = read(fd,buffer,sizeof(buffer));
    if(ret == -1){
        perror("login read");
        return false;
    }
    pack.Load(buffer);
    bzero(buffer,sizeof(buffer));
    if(pack.get_type() != LOGIN){
        cout << pack.get_content() <<endl;
        return false;
    }
    return true;
}

bool client::getonline(){
    pack.get_from() = name;
    pack.get_to() = "Server";
    pack.get_type() = GETONLINE;
    pack.get_content().clear();
    pack.get_size() = 0;
    write(fd,pack.Dump().c_str(),pack.Dump().size());
    int ret = read(fd,buffer,sizeof(buffer));
    if(ret == -1){
        perror("getonline read");
        return false;
    }
    pack.Load(buffer);
    bzero(buffer,sizeof(buffer));
    if(pack.get_type() != GETONLINE){
        cout << pack.get_content() << endl;
        return false;
    }
    return true;
}

bool client::message(string to,string content,TYPE type){
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
        readmsg();
        if(pack.get_content() == "ok" && pack.get_type() == ACK){
           cout << "发送成功" <<endl;
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
        }
        readCache += buffer;
        if(readCache.find("{") != string::npos && readCache.find("}")!=string::npos) break;
    }
    pack.Load(readCache);
    return true;
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

#endif