#include "mprpcapplication.h"
#include "mprpcchannel.h"
#include "user.pb.h"
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    // 整个程序启动以后, 想使用 mprpc 框架来享受 rpc 服务调用, 一定要先调用框架的初始化函数, 只需要初始化一次,
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的 rpc 方法 Login,
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    // rpc 方法的请求参数,
    fixbug::LoginRequest request;
    request.set_name("zhangsan");
    request.set_pwd("123456");

    // rpc 方法的响应参数,
    fixbug::LoginResponse response;

    // 发起 rpc 方法的调用, 同步的 rpc 调用过程, 底层就是 MprpcChannel::CallMethod() 的调用,
    stub.Login(nullptr, &request, &response, nullptr); // channel_->CallMethod(descriptor()->method(0), controller, request, response, done);
                                                       // channel_->CallMethod() 集中来做所有 rpc 方法调用的参数序列化和网络发送,

    // stub.Login() --> MprpcChannel::CallMethod() 将 request 发送完了, 同时将收到的 recv_buf 反序列化成 response, 下面我们就可以直接使用 response,
    // 一次 rpc 调用完成, 读调用的结果,
    if (0 == response.result().errcode())
    {
        std::cout << "rpc login response success : " << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
    }

    //...

    // 演示调用远程发布的 rpc 方法 Register,
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666666");
    fixbug::RegisterResponse resp;

    // 以同步的方式发起 rpc 调用请求, 等待返回结果,
    stub.Register(nullptr, &req, &resp, nullptr);

    // 一次 rpc 调用完成, 读调用的结果,
    if (0 == resp.result().errcode())
    {
        std::cout << "rpc register resp success : " << resp.success() << std::endl;
    }
    else
    {
        std::cout << "rpc register resp error : " << resp.result().errmsg() << std::endl;
    }

    return 0;
}
