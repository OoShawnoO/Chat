#include "client.h"

using namespace std;
int main(){
    string name;
    cin >> name;
    client clt(name,"123");
    if(!clt.connect()){
        cout << "connect error" << endl;
    }
    if(!clt.login()){
        cout << "login error" << endl;
    }

    while(1){
        sleep(2);
        clt.getonline();
        cout << clt.get_pack().get_content() <<endl;
    }
}