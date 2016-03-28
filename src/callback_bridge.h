#ifndef SOCK_DATA_CB_H
#define SOCK_DATA_CB_H

#include "tcp_socket.h"

typedef void (*on_sock_data)(void*, TcpSocket*);

typedef void (*on_mutex_acquired)(void);

class CallbackBridge
{
    public:
        CallbackBridge()
        { }
        ~CallbackBridge()
        { }

        void subscribe(on_sock_data cb, void* data);

        void on_sock_data_bridge(TcpSocket* sock);

        void subscribe(on_mutex_acquired cb);

        void on_mutex_acquired_bridge(void);

        void* sock_data;

    private:
        on_sock_data cb_on_data_read;
        on_mutex_acquired cb_on_mutex_acquired;
};

#endif
