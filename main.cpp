//
//  main.cpp
//  test
//
//  Created by zhTian on 2017/8/19.
//  Copyright © 2017年 zhTian. All rights reserved.
//

#include <iostream>
#include "lpRPCproxy.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    rpc_proxy rpc = rpc_proxy("127.0.0.1", 9000);
    int ret = rpc.rpc_call("/Users/zhtian/Desktop/fmt.json");
    std::cout << "ret from serv is = " << ret << "\n";
    
    return 0;
}
