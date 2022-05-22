#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <iostream>
#include <string>
#include <vector>
#include "logger.h"

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout << "do GetFriendsList service! userid" << userid << std::endl;
        std::vector<std::string> vec;
        vec.emplace_back("gaoyang");
        vec.emplace_back("liuhong");
        vec.emplace_back("wangshuo");
        return vec;
    }

    // 重写基类方法,
    void GetFriendsList(::google::protobuf::RpcController *controller,
                        const ::fixbug::GetFriendsListRequest *request,
                        ::fixbug::GetFriendsListResponse *response,
                        ::google::protobuf::Closure *done)
    {
        uint32_t userid = request->userid();
        std::vector<std::string> friendsList = GetFriendsList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for (std::string &name : friendsList)
        {
            std::string *p = response->add_friends();
            *p = name;
        }
        done->Run();
    }
};

int main(int argc, char **argv)
{
    LOG_INFO("first log message!");
    LOG_ERR("%s:%s:%d",__FILE__,  __FUNCTION__, __LINE__);

    MprpcApplication::Init(argc, argv);

    // 把 FriendService 对象发布到 rpc 节点上, 站点上发布多个 rpc 服务,  provider.NotifyService,
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个 rpc 服务发布节点, Run 以后, 进程进入阻塞状态, 等待远程的 rpc 调用请求,
    provider.Run();
    //

    return 0;
}
