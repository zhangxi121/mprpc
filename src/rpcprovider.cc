#include "rpcprovider.h"
#include "logger.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"

/*
    service_name ==>  ServiceDescriptor 描述 
                            ==> 对应一个 Service * , 保存了对象, 记录服务对象
                            ==> 同时对应   多个 MethodDescriptor 方法,


 */

void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    service_info.m_service = service;

    // 获取服务对象的描述信息,
    static const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();

    // 获取服务的名字,
    std::string service_name = pserviceDesc->name();

    // 获取服务对象 service 的方法的数量
    int methodCnt = pserviceDesc->method_count();

    LOG_INFO("service_name: %s", service_name.c_str());

    for (int i = 0; i < methodCnt; ++i)
    {
        // 获取了服务对象执行了下标的服务方法的描述 (抽象描述),
        // Service 描述对象,   MethodDescriptor 描述方法,
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        LOG_INFO("method_name: %s", method_name.c_str());
    }
    m_serviceMap.insert({service_name, service_info});
}

void RpcProvider::Run()
{
    // 读取配置文件 rpcserver 的信息,
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建 TcpServer 对象,
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 绑定回调函数, 连接回调 和 消息读写回调, 分离了网络代码和业务代码,
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置 muduo 库的线程数量,
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点    method_name为临时性节点
    for (auto &sp : m_serviceMap)
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    LOG_INFO("RpcProvider start service at ip: %s, port: %d", ip.c_str(), port);

    // 启动网络服务,
    server.start();
    m_eventLoop.loop();
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 和 rpc_client 的连接断开了, 关闭socket,
        conn->shutdown();
    }
}

/* 
    在框架内部 RpcProvider 和 RpcConsumer 要协商好之间通信用户 protobuf 数据类型,
    service_name  menthod_name  args , 定义 proto 的 message 类型, 进行数据头(service_name  menthod_name  args_size)的序列化和反序列化,
    UserServiceLoginZhangsan123456
    16UserServiceLogin14Zhangsan123456

    header_size(4字节) + header_str + args_str 
    header_str: service_name  menthod_name  args_size,
*/
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp)
{
    // 网络上接收的远程 rpc 调用请求接收的字符流,  Login args
    std::string recv_buf = buf->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    // 根据header_size, 数取数据头的原始字符流, 反序列化数据得到 rpc_header 的详细信息,
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (true == rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功,
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 数据头反序列化失败,
        LOG_ERR("rpc_header_str: %s  parse error!", rpc_header_str.c_str());
        return;
    }

    // 获取 rpc 方法参数的字符流数据,
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    LOG_INFO("====================================");
    LOG_INFO("header_size: %d", header_size);
    LOG_INFO("rpc_header_str: %s", rpc_header_str.c_str());
    LOG_INFO("service_name: %s", service_name.c_str());
    LOG_INFO("method_name: %s", method_name.c_str());
    LOG_INFO("args_str: %s", args_str.c_str());
    LOG_INFO("====================================");

    // 获取 service 对象和 method 对象,
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        LOG_ERR("%s is not exist!", service_name.c_str());
        return;
    }

    auto methodit = it->second.m_methodMap.find(method_name);
    if (methodit == it->second.m_methodMap.end())
    {
        LOG_ERR("%s : %s is not exist!", service_name.c_str(), method_name.c_str());
        return;
    }

    google::protobuf::Service *service = it->second.m_service;           // 获取 service 对象,
    const google::protobuf::MethodDescriptor *method = methodit->second; // 获取 method 对象,

    // 生成 rpc 方法调用的 请求request 和 响应response 参数,
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        LOG_ERR("request parse error, content: %s", args_str.c_str())
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的 method 方法的调用, 绑定一个 Closure 的回调函数,
    google::protobuf::Closure *done =
        google::protobuf::NewCallback<RpcProvider,
                                      const muduo::net::TcpConnectionPtr &,
                                      google::protobuf::Message *>(this, &RpcProvider::SendRpcResponse, conn, response);
    // 在框架上根据远端 rpc 请求, 调用当前 rpc节点上发布的方法,
    // service->CallMethod() 相当于调用远程方法, Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
    return;
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializePartialToString(&response_str)) // response 进行序列化,
    {
        // 序列化成功后, 通过网络把 rpc 方法执行的结果发送回 rpc 的调用方,
        conn->send(response_str);
    }
    else
    {
        LOG_ERR("Serialize response_str error!");
    }
    conn->shutdown(); // 模拟http的短连接服务, 由 rpcprovider 服务方主动断开连接,
}
