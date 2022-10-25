#include "client.h"
#include <thread>
using namespace std;
int main(){
    string name;
    cout << "请输入用户名:";
    cin >> name;
    client clt(name,"123");
    clt.connect();
    while(1){
        if(clt.login()) break;
    }
    while(1){
        int flag = 0;
        cout << "请选择操作序号:";
        cin >> flag;
        switch (flag)
        {
            case 1 : {
                clt.getonline();
                cout << clt.get_pack().Dump() <<endl;
                break;
            }
        
            case 2 : {
                cout << "请输入发送信息给谁:";
                cin >> name;
                clt.message(name,"傻逼",MSG);
                break;
            }

            case 3 : {
                clt.readmsg();
                cout << clt.get_pack().Dump() <<endl;
                if(clt.get_pack().get_type() != ACK){
                    Pack reply(clt.get_pack());
                    reply.get_to() = reply.get_from();
                    reply.get_content() = "ok";
                    reply.get_size() = sizeof(reply.get_content());
                    clt.message(reply.get_to(),reply.get_content(),ACK);
                }
                break;
            }
        default:
            break;
        }
    }
}