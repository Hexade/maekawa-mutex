#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "config.h"
#include "exception.h"
#include "tcp_socket.h"

#include <vector>

class Connection : public TcpSocket
{
    public:
        Connection(int id, int port, std::string& host)
            : TcpSocket(port, host), id(id)
        { }

        inline int getId(void) const { return id; }

    private:
        int id;
        
};

class ConnectionManager
{
    public:
        ConnectionManager() {}
        ConnectionManager(std::vector<TcpConfig>& cfgs);
        void init(std::vector<TcpConfig>& cfgs);
        void connect_all(void);
        bool connect(int id);
        void close_all(void);
        const Connection* get(int id);
        void wait_for_connections(void);
        
        inline void add(TcpConfig& cfg)
        {
            connections.emplace_back(cfg.number, cfg.port, cfg.host);
        }

        inline const std::vector<Connection>& get_all() { return connections; }

    private:
        std::vector<Connection> connections;
};

#endif
