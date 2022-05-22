#include "test.pb.h"
#include <iostream>
#include <string>

using namespace std;
using namespace fixbug;

// $ g++  main.cc test.pb.cc   -lprotobuf -std=c++14

int main001()
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

int main()
{
    LoginResponse resp;
    ResultCode *rc01 = resp.mutable_result();
    rc01->set_errcode(1);
    rc01->set_errmsg("login failed!");

    GetFriendListsResponse friendResp;
    ResultCode *rc = friendResp.mutable_result();
    rc->set_errcode(0);

    User *user1 = friendResp.add_fiend_list();
    user1->set_name("zhangsan");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2 = friendResp.add_fiend_list();
    user2->set_name("lisi");
    user2->set_age(22);
    user2->set_sex(User::MAN);

    std::cout << friendResp.fiend_list_size() << std::endl;

    return 0;
}