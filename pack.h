#ifndef PACK_H
#define PACK_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum VERSION{
        STANDARD = 0,
};
enum TYPE{
    MSG = 0,
    LOGIN,
    GETONLINE,
    ACK,
    ERR,
};

class Pack{
public:
    Pack(VERSION _version,string _from,string _to,string _content,TYPE _type)
    :version(_version),from(_from),to(_to),content(_content),type(_type)
    {
        size = content.size();
    }

    Pack(string package);
    Pack(const Pack& pack);
    string Dump();
    bool Load(string package);
    void clear();
    int& get_size();
    VERSION& get_version();
    string& get_from();
    string& get_to();
    string& get_content();
    TYPE& get_type();
private:
    VERSION version;
    string from;
    string to;
    string content;
    int size;
    TYPE type;
};

Pack::Pack(string package){
    Load(package);
}

Pack::Pack(const Pack& pack){
    version = pack.version;
    from = pack.from;
    to = pack.to;
    content = pack.content;
    size = pack.size;
    type = pack.type;
}

string Pack::Dump(){
    string pack;
    pack += "{";
    pack += "version:" + to_string(version) + "\r\n";
    pack += "type:" + to_string(type) + "\r\n";
    pack += "from:" + from + "\r\n";
    pack += "to:" + to + "\r\n";
    pack += "size:" + to_string(size) + "\r\n";
    pack += "content:" + content + "\r\n";
    pack += "}";
    return pack;
}
bool Pack::Load(string package){
    int pos = 0,site = 0;
    vector<string> v;
    package = package.substr(1,package.size()-2);
    while(package.find_first_of("\r\n",pos) != string::npos && pos < package.size()){
        site = package.find_first_of("\r\n",pos);
        v.push_back(package.substr(pos,site-pos));
        pos = site+2;
    }
    if(v.size() == 0) return false;
    for(auto x : v){
        pos = x.find(":");
        string attr = x.substr(0,pos);
        if(attr == "version") version = static_cast<VERSION>(atoi(x.substr(pos+1).c_str()));
        else if(attr == "type") type = static_cast<TYPE>(atoi(x.substr(pos+1).c_str()));
        else if(attr == "from") from = x.substr(pos+1);
        else if(attr == "to") to = x.substr(pos+1);
        else if(attr == "size") size = atoi(x.substr(pos+1).c_str());
        else if(attr == "content") content = x.substr(pos+1);
        else{
            cout << attr <<endl;
            cout << "Parse Error" <<endl;
        }
    }
    return true;
}

void Pack::clear(){
    version = STANDARD;
    from = "";
    to = "";
    content = "";
    size = 0;
    type = ERR;
}

VERSION& Pack::get_version(){
    return version;
}
string& Pack::get_from(){
    return from;
}
string& Pack::get_to(){
    return to;
}
int& Pack::get_size(){
    return size;
}
string& Pack::get_content(){
    return content;
}
TYPE& Pack::get_type(){
    return type;
}

#endif