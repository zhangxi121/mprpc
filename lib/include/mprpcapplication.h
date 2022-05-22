#ifndef MPRPC_APPLICATION_H
#define MPRPC_APPLICATION_H


#include "mprpcconfig.h"


// mprpc 框架的基础类, 单例, 负责框架的一些初始化操作,
//

class MprpcApplication
{
private:
    MprpcApplication() {}
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication(MprpcApplication &&) = delete;

public:
    ~MprpcApplication() {}
    static MprpcApplication &GetInstance();
    static MprpcConfig &GetConfig();

public:
    static void Init(int argc, char **argv);
    static MprpcConfig m_config;

private:
};

#endif