#ifndef PACK_H
#define PACK_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;
class Pack{
private:
    string version;
    string from;
    string to;
    string content;
    int size;
    string type;
public:
    Pack(string _version,string _from,string _to,string _content,string _type)
    :version(_version),from(_from),to(_to),content(_content),type(_type)
    {
    }

    Pack(string package);

    string Dump();
    bool Load(string package);
    int get_size();
    string& get_version();
    string& get_from();
    string& get_to();
    string& get_content();
    string& get_type();
};

Pack::Pack(string package){
    Load(package);
}

string Pack::Dump(){
    string pack;
    pack += "{";
    pack += "version:" + version + "\r\n";
    pack += "type:" + type + "\r\n";
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
        if(attr == "version") version = x.substr(pos+1);
        else if(attr == "type") type = x.substr(pos+1);
        else if(attr == "from") from = x.substr(pos+1);
        else if(attr == "to") to = x.substr(pos+1);
        else if(attr == "size") size = atoi(x.substr(pos+1).c_str());
        else if(attr == "content") content = x.substr(pos+1);
        else{
            perror("Parse Error");
            exit(-1);
        }
    }
    return true;
}

string& Pack::get_version(){
    return version;
}
string& Pack::get_from(){
    return from;
}
string& Pack::get_to(){
    return to;
}
int Pack::get_size(){
    return size;
}
string& Pack::get_content(){
    return content;
}
string& Pack::get_type(){
    return type;
}

#endif