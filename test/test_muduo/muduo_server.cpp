/**
    muduo 网络库给用户提供了两个主要的类,
    TcpServer: 编写服务器程序,
    TcpClient: 编写客户端程序,

    epoll + 线程池
    好处: 能够把网络IO的代码和业务代码区分开, 
    业务代码: 用户的连接和断开, 用户的可读写事件,
    网络库: 监听这些事件的发生, 给用户,

    Ctrl + Shift + B 编译操作,
    Executing task: /usr/bin/g++ -g /home/zhangxi/MyWorkspace/chat_dir/test_muduo/*.cpp  \
    -I/home/zhangxi/boost_dir/boost_1_77_0/ -o /home/zhangxi/MyWorkspace/chat_dir/bin/muduo_server \
    -lmuduo_net -lmuduo_base -lpthread -lmysqlclient -std=c++14 <

    Ctrl + F5

*/

#include <boost/bind.hpp>
#include <functional>
#include <iostream>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <string>

using namespace std;
using namespace placeholders;
using namespace muduo;
using namespace muduo::net;

//
// g++ muduo_server.cpp -o muduo_server -g  -lmuduo_net -lmuduo_base -lpthread -std=c++11
//

// 基于muduo网络库开发服务器程序
// 1. 组合 TCPServer 对象,
// 2. 创建 EventLoop 事件循环对象的指针,
// 3. 明确 TcpServer 构造函数需要什么参数,输出 ChatServer 的构造函数
// 4. 在当前服务器类的构造函数中,注册连接的回调函数 和 处理读写事件的回调函数,
// 5. 设置合适的服务端线程数量,muduo库会自己分配 IO线程 和 worker线程,
// 6.
// 7.
//
class ChatServer
{
public:
    ChatServer(muduo::net::EventLoop *loop,               // reactor 事件循环,
               const muduo::net::InetAddress &listenAddr, // IP + port,
               const string &nameArg,                     // _servername, 给线程绑定一个名字,
               TcpServer::Option option = TcpServer::kNoReusePort)
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开回调,
        //==> std::function<void (const TcpConnectionPtr&)>
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, placeholders::_1));

        // 给服务器注册用户读写事件回调,
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));

        // 设置服务器端的线程数量, 1个IO线程 3个worker线程,
        _server.setThreadNum(4);
    }

    // 开启事件循环,
    void start()
    {
        _server.start();
    }

private:
    // 专门处理用户的连接创建和断开, epoll  listenfd  accept,
    void onConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        // LOG_INFO << "ChatServer - " << conn->peerAddress().toIpPort() << " -> "
        //          << conn->localAddress().toIpPort() << " state is "
        //          << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected())
        {
            LOG_INFO << "ChatServer - " << conn->peerAddress().toIpPort() << " -> "
                     << conn->localAddress().toIpPort() << " state is UP";
        }
        else
        {
            LOG_INFO << "ChatServer - " << conn->peerAddress().toIpPort() << " -> "
                     << conn->localAddress().toIpPort() << " state is DOWN";
            //  _loop->quit();
        }
    }

    // 专门处理用户的读写事件的,
    void onMessage(const muduo::net::TcpConnectionPtr &conn, // 连接
                   muduo::net::Buffer *buf,                  // 缓冲器
                   muduo::Timestamp time)
    {
        // 接收到所有的消息，然后回显
        muduo::string msg(buf->retrieveAllAsString());
        LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
                 << "data received at " << time.toString();
        conn->send(msg);
    }

private:
    muduo::net::TcpServer _server; // #1
    muduo::net::EventLoop *_loop;  // #2 epoll
};

int main()
{
    LOG_INFO << "pid = " << getpid();
    muduo::net::EventLoop loop; //epoll
    muduo::net::InetAddress listenAddr("0.0.0.0", 6000);
    ChatServer server(&loop, listenAddr, "ChatServer");
    server.start(); // 启动服务, listenfd  epoll_ctl()=>epoll,
    loop.loop();    // epoll_wait() 以阻塞方式等待新用户连接、已连接用户的读写事件等,
}
