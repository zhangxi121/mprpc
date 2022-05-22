#include "test.pb.h"
#include <iostream>
#include <string>

using namespace std;
using namespace fixbug;

// $ g++  main.cc test.pb.cc   -lprotobuf -std=c++14

int main()
{
    // 封装了 LoginReq 对象的数据
    LoginRequest req;
    req.set_name("zhangsan");
    req.set_pwd("123456");

    // 对象数据的序列化,
    // req.SerializeToArray()
    std::string send_str;
    if (req.SerializeToString(&send_str))
    {
        std::cout << send_str << std::endl;
    }

    // 从 send_str 反序列化一个 LoginReq 对象,
    LoginRequest reqB;
    if (reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}
