//
//  lpRPCserv.h
//  lpRPC
//
//  Created by zhTian on 2017/8/19.
//  Copyright © 2017年 zhTian. All rights reserved.
//

#ifndef lpRPCserv_h
#define lpRPCserv_h

#include "lpRPCproxy.h"
#include <unordered_map>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <functional>
#include <vector>

#define SERVER_PORT 9000
#define BUFFERSIZE 1024


class rpc_server;

//可扩展
class Base {
public:
    void test() {
        printf("test---\n");
    }
};

typedef std::function<int(int)> func_type;
typedef std::function<void(Base&)> func_type_1;

class rpc_server {
public:
    rpc_server(){
    }
    
    rpc_server(const rpc_server&) = delete;
    rpc_server& operator=(const rpc_server&) = delete;
    ~rpc_server(){}
    
public:
    void run() {
        _init_socket();
    }
    
    void bind_to_function(std::string key, func_type f) {
        _m_map.insert(std::make_pair(key, f));
    }
    
private:
    std::unordered_map<std::string, std::function<int(int)>> _m_map;

public:
    std::vector<std::string> explode(const std::string& str, const char& ch) {
        std::string next;
        std::vector<std::string> result;
        
        // For each character in the string
        for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
            // If we've hit the terminal character
            if (*it == ch) {
                // If we have some characters accumulated
                if (!next.empty()) {
                    // Add them to the result vector
                    result.push_back(next);
                    next.clear();
                }
            } else {
                // Accumulate the next character into the sequence
                next += *it;
            }
        }
        if (!next.empty())
            result.push_back(next);
        return result;
    }
    
private:
    void _init_socket() {
        int listensockfd;
        int new_fd;
        int max_sd;
        struct sockaddr_in addr;
        char buffer[BUFFERSIZE];
        int close_conn = 0;
        
        struct fd_set master_set;
        struct timeval timeout;
        
        listensockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(listensockfd < 0) {
            perror("socket failed.");
            exit(1);
        }
        
        int on;
        int rc = setsockopt(listensockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
        rc = ioctl(listensockfd, FIONBIO, (char *)&on);
        
        bzero(&addr, sizeof(addr));
        
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(SERVER_PORT);
        
        int c = bind(listensockfd, (struct sockaddr *)&addr, sizeof(addr));
        printf("bind = %d\n", c);
        
        listen(listensockfd, 3);
                
        int clients[FD_SETSIZE];
        for(int i=0;i<FD_SETSIZE; ++i) {
            clients[i] = -1;
        }

        max_sd = listensockfd;
        FD_ZERO(&master_set);
        FD_SET(listensockfd, &master_set);

        //timeout
        timeout.tv_sec = 3 * 60;
        timeout.tv_usec = 0;

        int i = 0;
        int maxi = 0;
        do {

            FD_ZERO(&master_set);
            FD_SET(listensockfd, &master_set);

            printf("wait select.\n");
            rc = select(max_sd+1, &master_set, NULL, NULL, &timeout);
            if(rc < 0) {
                perror("select() failed.\n");
                break;
            }
            
            if(rc == 0) {
                printf("timout.\n");
                break;
            }
            
            if(FD_ISSET(listensockfd, &master_set)) {
                new_fd = accept(listensockfd, NULL, NULL);
                if(new_fd < 0) {
                    break;
                }
                
                for(i=0;i<FD_SETSIZE;++i) {
                    if(clients[i] < 0) {
                        clients[i] = new_fd;
                        break;
                    }
                }
                
                FD_SET(new_fd, &master_set);
                
                if(new_fd > max_sd) max_sd = new_fd;
                
                if(i>maxi) maxi = i;
            }
            
            //readable.
            for(int i=0; i<=maxi; ++i) {
                int ff = clients[i];
                if(ff < 0) continue;
                
                if(FD_ISSET(ff, &master_set)) {
                    bzero(buffer, strlen(buffer));
                    
                    ssize_t length;
                    while( (length = recv(ff,buffer,sizeof(buffer),0)))
                    {
                        if(length > 0) {
                            printf("%ld bytes received, and buffer = %s\n", length, buffer);
                            
                            /*
                             
                             解析，处理，返回给client
                             
                             */                            
                            std::string s(buffer);
                            const char delimiter = ',';

                            std::vector<std::string> tokens = explode(s, delimiter);
                            std::string name = tokens[0];
                            std::string args = tokens[1];
                            
                            int retvalue = 0;
                            if(_m_map.find(name) != _m_map.end()) {
                                func_type f = _m_map[name];
                                retvalue = f(std::stoi(args));
                                printf("ret value = %d\n", retvalue);
                            }
                            std::string ss = std::to_string(retvalue);
                            char const *pchar = ss.c_str();
                            send(ff, pchar, strlen(pchar), 0);
                            
                            break;
                        }
                    }
                    
                    if(length == 0) {
                        printf("connection close.\n");
                        close_conn = true;
                        break;
                    }
                    
                    if(length < 0) {
                        printf("connection close.\n");
                        close_conn = true;
                        break;
                    }
                    
                    if(close_conn) {
                        shutdown(listensockfd, SHUT_WR);
                        FD_CLR(i, &master_set);
                        if(i == max_sd) {
                            while(FD_ISSET(max_sd, &master_set) == false)
                                max_sd -= 1;
                        }
                    }
                }
            }
            
        }while(1);
        
        for(int f=0; f<=max_sd; ++f) {
            if(FD_ISSET(f, &master_set))
                shutdown(listensockfd, SHUT_RDWR);
        }
    }
    
};

#endif /* lpRPCserv_h */
