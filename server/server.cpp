#include "server.h"

const char* address = "127.0.0.1";
const short port = 9999;


int main(){
    //监听sock的创建
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    inet_pton(AF_INET,address,&saddr.sin_addr);
    saddr.sin_port = htons(port);
    bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    //设置端口多路复用
    char opt = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    //监听
    listen(sockfd,32);
    //链接数组
    conn* conns = new conn[max_users];
    //epoll实例
    int epollfd = epoll_create(1024);
    if(epollfd == -1){
        perror("epoll_create");
        exit(-1);
    }
    conn::epollfd = epollfd;
    user* huzhida = new user("huzhida","123");
    user* renshukui = new user("renshukui","123");
    user::users[huzhida->get_name()] = huzhida;
    user::users[renshukui->get_name()] = renshukui;
    //线程池初始化
    threadpool<conn>* pool = NULL;
    try{
        pool = new threadpool<conn>;
    }catch(...){
        perror("threadpool");
        exit(-1);
    }

    //epoll事件池
    epoll_event events[max_users];

    //监听sock的事件添加至events
    addEvent(epollfd,sockfd);
    
    while(1){
        //epoll_wait返回events中对应的触发事件的Index最大值
        int ret = epoll_wait(epollfd,events,max_users,-1);
        if(ret == -1 || errno == EINTR){
            perror("epoll_wait");
            exit(-1);
        }
        //遍历发生事件的event
        for(int i=0;i<ret;i++){
            int fd = events[i].data.fd;
            if(fd == sockfd){
                //监听事件的fd 需要添加链接
                sockaddr_in caddr;
                socklen_t len = sizeof(caddr);
                int connfd = accept(sockfd,(struct sockaddr*)&caddr,&len);
                if(connfd == -1){
                    perror("accept");
                    exit(-1);
                }
                if(conn::conn_count >= max_users){
                    close(connfd);
                    continue;
                }
                //连接池初始化对应的客户端链接对象
                cout << "初始化链接" <<endl; 
                conns[connfd].init(connfd,&caddr);
            }else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                //触发的事件是 对方关闭链接 或 错误
                conns[fd].close();
            }else if(events[i].events & EPOLLIN){
                
                //触发的事件是EPOLLIN 就是有信息输入需要读取
                if(conns[fd].read()){
                    //如果读到了数据 线程池分一个线程来处理数据
                    pool->append(conns+fd);
                    cout << "线程池执行" <<endl;
                }else{
                    //如果读数据出错 关闭链接实例
                    conns[fd].close();
                    cout << "因 EPOLLIN 关闭" <<endl;
                }
            }else if(events[i].events & EPOLLOUT){
                //触发的事件是EPOLLOUT 就是有消息需要输出
                if(!conns[fd].write()){
                    //大概是对方关闭链接 本地链接实例也关闭
                    conns[fd].close();
                    cout << "因 EPOLLOUT 关闭" <<endl;
                }
            }
        }

    }
    delete []conns;
    close(sockfd);
}
