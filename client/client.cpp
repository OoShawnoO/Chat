#include "client.h"
#include <thread>
using namespace std;

void READ(client& clt){
    
}

int main(){
    string name;
    cout << "请输入用户名:";
    cin >> name;
    client clt(name,"123");
    clt.connect();
    
    while(1){
        int flag = 0;
        cout << "请选择操作序号:";
        cin >> flag;
        switch (flag)
        {
            case 1 : {
                if(!clt.login()){
                    cout << "登录失败" <<endl;
                }
                break;
            }
        
            case 2 : {
                cout << "请输入发送信息给谁:";
                cin >> name;
                if(!clt.message(name,"傻逼",MSG)){
                    cout << "发送失败" <<endl;
                }
                break;
            }

            case 3 : {
                if(!clt.getonline()){
                    cout << "获取失败" <<endl;
                }
                break;
            }
            default:
                break;
        }
    }
}