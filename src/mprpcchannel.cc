#include "mprpcchannel.h"
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"
#include <arpa/inet.h>
#include <error.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* 
    header_size(4字节) + header_str + args_str 
    header_str: service_name  menthod_name  args_size,
*/

// 所有通过 stub 代理对象调用的 rpc 方法都走到这里了, 在 CallMethod() 统一做 rpc 方法调用的数据序列化和网络发送,
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                              google::protobuf::Message *response, google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();    // service_name,
    std::string method_name = method->name(); // method_name,

    // 获取参数的序列化字符串的长度 args_size,
    std::string args_str;
    uint32_t args_size = 0;
    if (request->SerializePartialToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("Serialize request error!");
        return;
    }

    // 定义  rpc 的请求header,
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    std::string rpc_header_str;
    uint32_t header_size = 0;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("Serialize rpc_header_str error!");
        return;
    }

    // 组织待发送的 rpc 请求的字符串,
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&header_size, 4)); // header_size,
    send_rpc_str += rpc_header_str;                               //rpcheader
    send_rpc_str += args_str;                                     // args

    // 打印调试信息
    std::cout << "====================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "====================================" << std::endl;

    // 使用 tcp 编程, 完成 rpc 方法的远程调用,
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char errTxt[512] = {0};
        sprintf(errTxt, "create socket error, errno:%d", errno);
        controller->SetFailed(errTxt);
        return;
    }

    // 读取配置文件 rpcserver 的信息,
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    //

    // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    ZkClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

    // 
    struct sockaddr_in serv_addr;
    socklen_t serv_len = sizeof(serv_addr);
    memset(&serv_addr, 0, serv_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip.c_str()); // inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(port);

    // 连接 rpc 服务节点,
    if (-1 == connect(clientfd, (struct sockaddr *)&serv_addr, serv_len))
    {
        close(clientfd);
        char errTxt[512] = {0};
        sprintf(errTxt, "connect error, errno::%d", errno);
        controller->SetFailed(errTxt);
        return;
    }

    // 发送 rpc 请求,
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        close(clientfd);
        char errTxt[512] = {0};
        sprintf(errTxt, "send error, errno:%d", errno);
        controller->SetFailed(errTxt);
        return;
    }

    // 接收 rpc 请求的响应值,
    char recv_buf[1024] = {0};
    int recv_size = 0;
    recv_size = recv(clientfd, recv_buf, sizeof(recv_buf), 0);
    if (-1 == recv_size)
    {
        close(clientfd);
        char errTxt[512] = {0};
        sprintf(errTxt, "recv error, errno:%d", errno);
        controller->SetFailed(errTxt);
        return;
    }
    close(clientfd);

    // 反序列化 rpc 调用的响应数据,
    // std::string response_str(recv_buf, 0, recv_size); // bug 出现问题, std::string  中遇到 "\0" ,后面的数据就存不下来了, 字符串错误, 导致反序列化失败,
    // if (!response->ParseFromString(response_str))

    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        char errTxt[512] = {0};
        sprintf(errTxt, "parse error! response_str : %s", recv_buf);
        controller->SetFailed(errTxt);
        return;
    }
}
