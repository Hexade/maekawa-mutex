#ifndef SOCK_DATA_CB_H
#define SOCK_DATA_CB_H

#include "tcp_socket.h"

typedef void (*on_sock_data)(void*, TcpSocket*);

class SockDataCb
{
    public:
        SockDataCb()
        { }
        ~SockDataCb()
        { }

        void subscribe(on_sock_data cb, void* data);

        void on_data_read(TcpSocket* sock) throw (Exception);

        void* sock_data;

    private:
        on_sock_data cb_func;
};

#endif
