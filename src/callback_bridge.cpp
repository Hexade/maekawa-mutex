#include "exception.h"
#include "callback_bridge.h"

void CallbackBridge::subscribe(on_sock_data cb, void* data)
{
    cb_on_data_read = cb;
    sock_data = data;
}

void CallbackBridge::on_sock_data_bridge(TcpSocket* sock)
{
    if (!cb_on_data_read)
        throw Exception("callback function not defined");
    cb_on_data_read(sock_data, sock);
}

void CallbackBridge::subscribe(on_mutex_acquired cb)
{
    cb_on_mutex_acquired = cb;
}

void CallbackBridge::on_mutex_acquired_bridge()
{
    if (!cb_on_mutex_acquired)
        throw Exception("callback function not defined");
    cb_on_mutex_acquired();
}
