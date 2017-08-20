//
//  lpRPCproxy.h
//  lpRPC
//
//  Created by zhTian on 2017/8/19.
//  Copyright © 2017年 zhTian. All rights reserved.
//

#ifndef lpRPCproxy_h
#define lpRPCproxy_h

#define __NAMESPACE__BEGIN namespace ministl {
#define __NAMESPACE__END   };

#define BUFFSIZE 1024

#include <sys/socket.h> //sockaddr
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h> //inet_pton
#include <unistd.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include <iostream>
#include <fstream>

//__NAMESPACE__BEGIN

class rpc_proxy {
public:
    rpc_proxy(char *addr, u_short port = 9000) : _m_port(port) {
        _m_address = addr;
        _m_port = port;
        
        _init_client_socket();
    }
    
    int rpc_call(char *path) {
        int ret = 0;
        char buff[1024];
        bzero(buff, sizeof(buff));
        std::ifstream infile;
        infile.open(path);
        
        std::cout << "Reading from the file" << "\n";
        infile >> buff;
        std::cout << buff << "\n";
        
        ssize_t r = write(_sockfd, buff, strlen(buff));
        printf("r = %d\n", r);
        
        bzero(buff,BUFFSIZE);
        ssize_t length = 0;
        while((length = recv(_sockfd,buff,BUFFSIZE,0)))
        {
            if(length < 0)
            {
                printf("Recieve Data From Server %s Failed!\n", _m_address);
                break;
            }
            
            printf("Receive = %s\n", buff);
            ret = atoi(buff);
            break;
        }
        
        close(_sockfd);
        
        return ret;
    }
    
private:
    void _init_client_socket() {
        struct sockaddr_in server;
        
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        
        bzero(&server, 0);
        
        server.sin_family = AF_INET;
        server.sin_port = htons(_m_port);
        
        inet_pton(AF_INET, _m_address, &(server.sin_addr)); // IPv4
        
        connect(_sockfd, (struct sockaddr *)&server, sizeof(server));
    }
    
private:
    char *_m_address;
    u_short _m_port;
    int _sockfd;
};


//__NAMESPACE__END

#endif /* lpPRCproxy_h */
