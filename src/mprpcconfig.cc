#include "mprpcconfig.h"
#include <iostream>
#include <string>

void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf)
    {
        std::cout << config_file << " is not exist" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (!feof(pf))
    {
        // 1. 注释
        // 2. 正确的配置项,
        // 3. 去掉开头的多余的空格,
        char buf[512] = {};
        fgets(buf, 512, pf);

        // 去掉字符串前面多余的空格,
        std::string read_buf(buf);
        Trim(read_buf);

        // 判断 # 号的注释,
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }

        // 用 = 号解析配置项
        size_t idx = read_buf.find('=');
        if (idx == -1)
        {
            // 配置项不合法,
            continue;
        }
        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        Trim(key);

        // value = "127.0.0.1\n", 多了一个换行,
        // "rpcserverip  =   127.0.0.1   \n"
        size_t endidx = read_buf.find('\n', idx);
        value = read_buf.substr(idx + 1, endidx - 1 - idx);
        Trim(value);

        m_configMap.insert({key, value});
    }
}

std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}

// 去掉字符串前后的空格,
void MprpcConfig::Trim(std::string &src_buf)
{
    size_t idx = src_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串前面有空格,
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }

    // 去掉字符串后面多余的空格,
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串后面有空格,
        src_buf = src_buf.substr(0, idx + 1);
    }
}