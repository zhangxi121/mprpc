#pragma once
#include <functional>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <string>
#include <unordered_map>


// 框架提供的专门负责发布 prc 服务的网络对象类,

class RpcProvider
{
public:
    RpcProvider() {}
    ~RpcProvider() {}

public:
    // 这里是框架提供给外部使用的, 可以发布 rpc 方法的函数接口, 所以参数是 google::protobuf::Service* ,
    void NotifyService(google::protobuf::Service *service);

    // 启动 rpc 服务节点, 开始提供 rpc 远程网络调用服务,
    void Run();

private:
    // 新的 socket 连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr &conn);

    // 已建立连接用户的读写事件回调, 如果远端有一个 rpc 服务的调用请求, 那么 OnMessage 就会响应,
    void OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp);

    // google::protobuf::Closure 的回调操作, 用于序列化 rpc 的响应和网络发送,
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response);

private:
    // 组合 EventLoop
    muduo::net::EventLoop m_eventLoop;

private:
    // Service 服务类型信息,
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;                                                    // 保存服务对象,
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap; // 保存服务方法,
    };

    // 存储注册成功的服务对象和其服务方法的所有信息,
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
};
