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

using namespace std;
const int max_users = 10000;
class user{
private:
    int id;
    string name;
    string password;

public:
    static int user_count;
    static user* users;
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

int user::user_count = 0;
user* user::users = new user[max_users];

#endif