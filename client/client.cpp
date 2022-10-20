#include "client.h"
#include "../pack.h"
using namespace std;
int main(){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(9999);
    int ret = connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
    if(ret==-1){
        perror("connect :");
        exit(-1);
    }

    Pack pack(STANDARD,"huzhida","huzhida","123",LOGIN);
    char buf[1024] = {0};
    while(1){
        cout << "123" <<endl;
        write(sockfd,pack.Dump().c_str(),pack.Dump().size());
        read(sockfd,buf,1024);
        cout << buf <<endl;
        sleep(5);
    }
    
    close(sockfd);
}