#ifndef USER_H
#define USER_H

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <memory.h>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <unordered_map>
#include "redis.h"

using namespace std;
const int max_users = 10000;
class user{
private:
    int id;
    string name;
    string password;

public:
    static int user_count;
    user(){}
    user(string _username,string _password){
        init(_username,_password);
    }
    void init(string _name,string _password){
        name = _name;
        password = _password;
        id = user_count;
        user_count++;
    }
    int get_id();
    string& get_name();
    string& get_password();
};

string& user::get_name(){
    return name;
}

string& user::get_password(){
    return password;
}

int user::user_count = 0;

#endif