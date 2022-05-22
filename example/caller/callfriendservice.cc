#include "friend.pb.h"
#include "mprpcapplication.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    // 整个程序启动以后, 想使用 mprpc 框架来享受 rpc 服务调用, 一定要先调用框架的初始化函数, 只需要初始化一次,
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的 rpc 方法 Login,
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    // rpc 方法的请求参数,
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);

    // rpc 方法的响应参数,
    fixbug::GetFriendsListResponse response;

    // 发起 rpc 方法的调用, 同步的 rpc 调用过程, 底层就是 MprpcChannel::CallMethod() 的调用,
    MprpcController controller;
    stub.GetFriendsList(&controller, &request, &response, nullptr); // channel_->CallMethod(descriptor()->method(0), controller, request, response, done);
                                                                    // channel_->CallMethod() 集中来做所有 rpc 方法调用的参数序列化和网络发送,

    // stub.Login() --> MprpcChannel::CallMethod() 将 request 发送完了, 同时将收到的 recv_buf 反序列化成 response, 下面我们就可以直接使用 response,
    // 一次 rpc 调用完成, 读调用的结果,
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc GetFriendsList response success! " << std::endl;
            int size = response.friends_size();
            for (int i = 0; i < size; ++i)
            {
                std::cout << "index:" << (i + 1) << ", name:" << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendsList response error : " << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}
