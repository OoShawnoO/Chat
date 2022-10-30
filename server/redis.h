#ifndef REDIS_H
#define REDIS_H

#include "hiredis/hiredis.h"
#include <iostream>
#include <string>

using namespace std;
class redis{
public:
    static redisContext* Context;
    static redisContext* Connect(const char* ip,int port);
    static void FreeContext();
    static string get(string cmd);
    static void set(string key,string value);
    static bool exist(string cmd);
private:
    redis(){}
    redis(const redis&){}
};

redisContext* redis::Context = nullptr;

redisContext* redis::Connect(const char* ip,int port){
    redisContext* context = redisConnect(ip,port);
    if(context == NULL || context->err){
        cout << "redis connect error;" <<endl;
        exit(-1);
    }
    Context = context;
    return context;
}
void redis::FreeContext(){
    redisFree(Context);
}
string redis::get(string cmd){
    redisReply* reply = (redisReply *)redisCommand(Context,"get %s",cmd.c_str());
    if(reply == NULL){
        cout << "redis get error;" <<endl;
        exit(-1);
    }
    switch(reply->type){
        case REDIS_REPLY_STRING : {
            string r = reply->str;
            freeReplyObject(reply);
            return r;
            break;
        }
        case REDIS_REPLY_INTEGER : {
            string r = to_string(reply->integer);
            freeReplyObject(reply);
            return r;
            break;
        }
        case REDIS_REPLY_ARRAY : {
            cout << "redis array return;";
            exit(-1);
            exit(-1);
            break;
        }
        case REDIS_REPLY_ERROR : {
            string r = reply->str;
            freeReplyObject(reply);
            return r;
            break;
        }
    }
    return NULL;
}
void redis::set(string key,string value){
    redisReply* reply = (redisReply *)redisCommand(Context,"set %s %s",key.c_str(),value.c_str());
    if(reply == NULL){
        cout << "redis set error;" <<endl;
        exit(-1);
    }
    freeReplyObject(reply);
}

bool redis::exist(string cmd){
    redisReply* reply = (redisReply *)redisCommand(Context,"exists %s",cmd.c_str());
    if(reply == NULL){
        cout << "redis exists error;" <<endl;
        exit(-1);
    }
    int r = reply->integer;
    freeReplyObject(reply);
    if(r == 1) return true;
    else return false;
}

#endif