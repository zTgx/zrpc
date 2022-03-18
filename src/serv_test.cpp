//
//  serv_test.cpp
//  lpRPC
//
//  Created by zhTian on 2017/8/19.
//  Copyright © 2017年 zhTian. All rights reserved.
//

#include <iostream>
#include "lpRPCserv.h"

int do_it(int i) {
    return i*i;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    
    std::string key("powder");
    
    rpc_server serv;
    serv.bind_to_function(key, do_it);
    serv.run();
    
    return 0;
}
