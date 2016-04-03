#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "exception.h"
#include "callback_bridge.h"
#include "tcp_socket.h"

#include <vector>

#define REQ_QUEUE_SZ 15

class TcpServer
{
    public:
        TcpServer(int portnum, CallbackBridge* cb, int c_data_len)
        : main_socket(portnum), client_data_cb(cb),
            client_data_len(c_data_len), max_fd(0), do_terminate(false)
        { }
        ~TcpServer() throw (Exception);
        void run(void) throw (Exception);
        inline void terminate() { do_terminate = true; }
        
    private:
        TcpSocket main_socket;
        CallbackBridge* client_data_cb;
        int client_data_len;

        std::vector<TcpSocket> client_sockets;
        int max_fd;
        fd_set active_fd_set, read_fd_set;
        bool do_terminate;
};

#endif
