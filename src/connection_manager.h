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

        inline int getId(void) { return id; }

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

        inline const std::vector<Connection>& get_all() { return connections; }

    private:
        std::vector<Connection> connections;
};

#endif
