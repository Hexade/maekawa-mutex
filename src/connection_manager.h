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
        ConnectionManager(std::vector<TcpConfig>& cfgs);
        void connect_all(void);
        void close_all(void);
        const Connection* get(int id);

    private:
        std::vector<TcpConfig>& configs;
        std::vector<Connection> connections;
};

#endif
