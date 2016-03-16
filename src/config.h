#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

struct TcpConfig
{
    int number;
    int port;
    std::string host;    
};

class Config
{
    public:
        ~Config() {}

        Config(std::string name)
        : filename(name)
        { }

        TcpConfig& getTcpConfig(int number);
        inline std::vector<TcpConfig>& getAll(void)
        { return tcp_configs; }

        void create(void);
    
    private:
        std::string filename;
        std::vector<TcpConfig> tcp_configs;
};



#endif
